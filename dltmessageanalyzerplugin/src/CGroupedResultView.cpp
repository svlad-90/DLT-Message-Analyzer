/**
 * @file    CGroupedView.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CGroupedView class
 */

#include "QHeaderView"
#include "QScrollBar"
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QModelIndex>
#include <QDebug>

#include "CGroupedView.hpp"
#include "CTreeItem.hpp"


//CIndexWrapper
class CIndexWrapper
{
public:
    CIndexWrapper( const QModelIndex& modelIndex ):
        mLevel(0), mRow(modelIndex.row()), mModelIndex(modelIndex)

    {
        auto pCurrentParent = mModelIndex.parent();

        while(pCurrentParent.row() != -1 && pCurrentParent.column() != -1)
        {
            mLevel += 1;
            pCurrentParent = pCurrentParent.parent();
        }
    }

    bool operator< ( const CIndexWrapper& val ) const
    {
        bool bResult = false;

        auto& myLevel = mLevel;
        auto& valLevel = val.mLevel;

        bool bIsInMyParentHierarcy = false;

        {
            auto isInMyParentHierarcy = [](const CGroupedViewItem* pMe, const CGroupedViewItem* pVal)->bool
            {
                bool bResult = false;

                const auto* pCurrentParent = pMe->getParent();

                while(nullptr != pCurrentParent)
                {
                    if(pCurrentParent == pVal)
                    {
                        bResult = true;
                        break;
                    }

                    pCurrentParent = pCurrentParent->getParent();
                }

                return bResult;
            };

            const auto* pMyElement = static_cast<CGroupedViewItem*>(mModelIndex.internalPointer());
            const auto* pValElement = static_cast<CGroupedViewItem*>(val.mModelIndex.internalPointer());

            if(myLevel < valLevel)
            {
                if(true == isInMyParentHierarcy(pValElement, pMyElement))
                {
                    bIsInMyParentHierarcy = true;
                    bResult = true;
                }
            }
            else if(myLevel > valLevel)
            {
                if(true == isInMyParentHierarcy(pMyElement, pValElement))
                {
                    bIsInMyParentHierarcy = true;
                    bResult = false;
                }
            }
            else
            {
                bIsInMyParentHierarcy = false;
            }
        }

        if(false == bIsInMyParentHierarcy)
        {
            auto minLevel = std::min(myLevel, valLevel);

            const auto* pMyMinLevelParent = static_cast<CGroupedViewItem*>(mModelIndex.internalPointer());
            const auto* pValMinLevelParent = static_cast<CGroupedViewItem*>(val.mModelIndex.internalPointer());

            {
                for(int i = myLevel; i > minLevel; --i)
                {
                    pMyMinLevelParent = pMyMinLevelParent->getParent();
                }
            }

            {
                for(int i = valLevel; i > minLevel; --i)
                {
                    pValMinLevelParent = pValMinLevelParent->getParent();
                }
            }

            const auto* pMyChildToCompare = pMyMinLevelParent;
            const auto* pValChildToCompare = pValMinLevelParent;

            while(pMyMinLevelParent != pValMinLevelParent)
            {
                pMyChildToCompare = pMyMinLevelParent;
                pValChildToCompare = pValMinLevelParent;
                pMyMinLevelParent = pMyMinLevelParent->getParent();
                pValMinLevelParent = pValMinLevelParent->getParent();
            }

            auto myCompareRow = pMyChildToCompare->row();
            auto valCompareRow = pValChildToCompare->row();

            bResult = myCompareRow < valCompareRow;
        }

        return bResult;
    }

    const QModelIndex& getModelIndex() const { return mModelIndex; }
    const int& getLevel() const { return mLevel; }

private:
    int mLevel;
    int mRow;
    QModelIndex mModelIndex;
};


typedef std::set<CIndexWrapper> tSelectedRowWrapperSet;

//CGroupedView
CGroupedView::CGroupedView(QWidget *parent):
    tParent(parent),
    mWidthUpdateRequired(eUpdateRequired::eUpdateRequired_NO),
    mbIsVerticalScrollBarVisible(false)

{
    setSortingEnabled(true);
    sortByColumn( static_cast<int>(eGroupedViewColumn::eGroupedViewColumn_Statistics), Qt::SortOrder::DescendingOrder );

    connect( this, &QTreeView::expanded, [this](const QModelIndex &){ updateWidth(); } );
    connect( this, &QTreeView::collapsed, [this](const QModelIndex &){ updateWidth(); } );

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Copy", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copyGroupedViewSelection();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );
}

void CGroupedView::updateWidth()
{
    int widthOfSubString = 0;

    for(int i = 0; i < 2; ++i)
    {
        switch (i)
        {
        case 0:
        {
            if(false == isColumnHidden(i))
            {
                resizeColumnToContents(i);
                widthOfSubString = columnWidth(i);
            }
        }
            break;
        case 1:
        {
            if(false == isColumnHidden(i))
            {
                auto statisticsWidth = viewport()->size().width() - widthOfSubString;
                setColumnWidth(i, statisticsWidth);
            }
        }
            break;

        }
    }
}

void CGroupedView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    tParent::dataChanged(topLeft,bottomRight,roles);

    auto pVerticalScrollBar = verticalScrollBar();

    auto isVerticalScrollBarVisible_ = pVerticalScrollBar->isVisible();

    {
        if(0 == model()->rowCount())
        {
            mWidthUpdateRequired = eUpdateRequired::eUpdateRequired_BE_READY;
        }

        if(0 < model()->rowCount())
        {
            if(eUpdateRequired::eUpdateRequired_BE_READY == mWidthUpdateRequired)
            {
                mWidthUpdateRequired = eUpdateRequired::eUpdateRequired_REQUIRED;
            }
        }
    }

    bool bUpdated = false;

    if(false == bUpdated && eUpdateRequired::eUpdateRequired_REQUIRED == mWidthUpdateRequired)
    {
        updateWidth();
        mWidthUpdateRequired = eUpdateRequired::eUpdateRequired_NO;
        bUpdated = true;
    }

    if(false == bUpdated && mbIsVerticalScrollBarVisible != isVerticalScrollBarVisible_)
    {
        updateWidth();
        mbIsVerticalScrollBarVisible = isVerticalScrollBarVisible_;
        bUpdated = true;
    }
}

bool CGroupedView::isVerticalScrollBarVisible() const
{
    if(verticalScrollBarPolicy() == Qt::ScrollBarPolicy::ScrollBarAlwaysOff)
        return false;

    bool IsVisible = false;

    int HeightOfRows = 0;
    int HeaderHeight = header()->height();
    int TableHeight = height();

    int childCount = model()->rowCount();
    QModelIndexList currentChildren;

    for(int i = 0; i < childCount; ++i)
    {
        if( false == isRowHidden(i, QModelIndex()) )
        {
            currentChildren.push_back( model()->index( i, 0 ) );
        }
    }

    while(false == currentChildren.empty())
    {
        for(const auto& child : currentChildren)
        {
            HeightOfRows += rowHeight(child);

            if( (HeightOfRows + HeaderHeight) > TableHeight)
            {
                IsVisible = true;
                break;
            }
        }

        auto currentChildrenCopy = currentChildren;
        currentChildren.clear();

        for(const auto& child : currentChildrenCopy)
        {
            childCount = model()->rowCount(child);

            for(int i = 0; i < childCount; ++i)
            {
                if( false == isRowHidden(i, child) )
                {
                    currentChildren.push_back( model()->index( i, 0, child) );
                }
            }
        }
    }

    return IsVisible;
}

void CGroupedView::copyGroupedViewSelection() const
{
    auto selectedRows = selectionModel()->selectedRows();

    tSelectedRowWrapperSet selectedRowSet;

    for(const auto& index : selectedRows)
    {
        selectedRowSet.insert( CIndexWrapper(index) );
    }

    QString finalText;

    for(const auto& item : selectedRowSet)
    {
        auto index = item.getModelIndex();

        auto pStatisticsColumn = index.model()->sibling(index.row(), index.column()+1, index); // next column with statistics

        QString message = index.data().value<QString>();
        int statistics = pStatisticsColumn.data().value<int>();

        for(int i = 0; i < item.getLevel(); ++i)
        {
            if(i == item.getLevel() - 1)
            {
                finalText.append("|-");
            }
            else
            {
                finalText.append("  ");
            }
        }

        finalText.append(message).append(" | statistics: ").append(QString::number(statistics)).append("\n");
    }

    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->setText(finalText);
}

void CGroupedView::keyPressEvent ( QKeyEvent * event )
{
    if(event->matches(QKeySequence::Copy))
    {
        if(hasFocus())
        {
            copyGroupedViewSelection();
        }
    }
    else
    {
        tParent::keyPressEvent(event);
    }
}
