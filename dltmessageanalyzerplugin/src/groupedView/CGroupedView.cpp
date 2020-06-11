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
#include <QStyledItemDelegate>

#include "CGroupedView.hpp"
#include "../common/CTreeItem.hpp"
#include "../log/CConsoleCtrl.hpp"

class CDoubleDelegate: public QStyledItemDelegate
{
    public:

    CDoubleDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) { }
    virtual ~CDoubleDelegate() { }

    virtual QString displayText(const QVariant &value, const QLocale &locale) const
    {
        if (value.type() == QVariant::Double) {
            return locale.toString( value.toDouble(), 'f', 3 );
        }
        return QStyledItemDelegate::displayText(value, locale);
    }
};

namespace NShortcuts
{
    static bool isCopyShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_C;
    }

    static bool isExpadOneLevelShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_E;
    }

    static bool isCollapseOneLevelShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->modifiers() & Qt::ShiftModifier && pEvent->key() == Qt::Key_E;
    }
}

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
            auto isInMyParentHierarcy = [](const tTreeItemPtr pMe, const tTreeItemPtr pVal)->bool
            {
                bool bResult_ = false;

                const auto* pCurrentParent = pMe->getParent();

                while(nullptr != pCurrentParent)
                {
                    if(pCurrentParent == pVal)
                    {
                        bResult_ = true;
                        break;
                    }

                    pCurrentParent = pCurrentParent->getParent();
                }

                return bResult_;
            };

            const auto pMyElement = static_cast<tTreeItemPtr>(mModelIndex.internalPointer());
            const auto pValElement = static_cast<tTreeItemPtr>(val.mModelIndex.internalPointer());

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

            const auto* pMyMinLevelParent = static_cast<tTreeItemPtr>(mModelIndex.internalPointer());
            const auto* pValMinLevelParent = static_cast<tTreeItemPtr>(val.mModelIndex.internalPointer());

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
    mbIsVerticalScrollBarVisible(false),
    mbRootExpandingRequired(false)
{
    connect( this, &QTreeView::expanded, [this](const QModelIndex &){ updateWidth(); } );
    connect( this, &QTreeView::collapsed, [this](const QModelIndex &){ updateWidth(); } );

    setItemDelegate(new CDoubleDelegate(this));

    header()->setDefaultAlignment(Qt::AlignCenter);

    header()->setSectionsMovable(false);

    setSortingEnabled(true);
    sortByColumn( static_cast<int>(eGroupedViewColumn::Messages), Qt::SortOrder::DescendingOrder );

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

            contextMenu.addSeparator();
        }

        {
            auto selectedRows = selectionModel()->selectedRows();

            if(1 == selectedRows.size()) // if one row is selected
            {
                auto selectedRow = selectedRows[0];
                bool bIsSelectedRowExpanded = isExpanded(selectedRow);
                bool bIsAnyChildExpanded = false;
                bool bAreAllChildrenExpanded = true;

                const int childCount = model()->rowCount( selectedRow );
                for(int i = 0; i < childCount; ++i)
                {
                    if( true == isExpanded(selectedRow.child(i, 0) ) )
                    {
                        bIsAnyChildExpanded = true;
                    }
                    else
                    {
                        bAreAllChildrenExpanded = false;
                    }

                    if(true == bIsAnyChildExpanded && false == bAreAllChildrenExpanded)
                    {
                        break;
                    }
                }

                if(false == bIsSelectedRowExpanded || false == bAreAllChildrenExpanded)
                {
                    QString name;

                    bIsSelectedRowExpanded ? name = "Expand node's children" : name = "Expand node";

                    QAction* pAction = new QAction(name, this);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+E")));
                    connect(pAction, &QAction::triggered, [this, selectedRow]()
                    {
                        changeLevelExpansion(selectedRow, true);
                    });

                    contextMenu.addAction(pAction);
                }

                if(true == bIsSelectedRowExpanded || true == bIsAnyChildExpanded)
                {
                    QString name;
                    (true == bIsAnyChildExpanded) ? name = "Collapse node's children" : name = "Collapse node";

                    QAction* pAction = new QAction(name, this);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+E")));
                    connect(pAction, &QAction::triggered, [this, selectedRow]()
                    {
                        changeLevelExpansion(selectedRow, false);
                    });

                    contextMenu.addAction(pAction);
                }
            }
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect(this, &QTreeView::doubleClicked, [this](const QModelIndex &index)
    {
        if(false == index.isValid())
        {
            return;
        }

        if( toGroupedViewColumn(index.column()) == eGroupedViewColumn::SubString )
        {
            isExpanded(index)? collapse(index) : expand(index);
        }
    });
}

void CGroupedView::setModel(QAbstractItemModel *model)
{
    tParent::setModel(model);
    updateColumnsVisibility();
}

void CGroupedView::changeLevelExpansion(const QModelIndex& expandIdx, bool bExpand)
{
    if(false == expandIdx.isValid())
    {
        return;
    }

    if(true == bExpand && false == isExpanded(expandIdx))
    {
        expand( expandIdx );
    }
    else if(true == bExpand && true == isExpanded(expandIdx))
    {
        const int childCount = model()->rowCount( expandIdx );
        for(int i = 0; i < childCount; ++i)
        {
            expand( expandIdx.child(i, 0) );
        }
    }
    else if(false == bExpand && true == isExpanded(expandIdx))
    {
        bool bAnyChildExpanded = false;

        const int childCount = model()->rowCount( expandIdx );
        for(int i = 0; i < childCount; ++i)
        {
            auto childIdx = expandIdx.child(i, 0);

            if(isExpanded(childIdx))
            {
                bAnyChildExpanded = true;
                collapse( childIdx );
            }
        }

        if(false == bAnyChildExpanded)
        {
            collapse(expandIdx);
        }
    }
    else
    {
        // do nothing
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

    if(model()->rowCount() == 0)
    {
        mbRootExpandingRequired = true;
    }
    else
    {
        if(true == mbRootExpandingRequired)
        {
            auto rootIdx = model()->index(0,0,QModelIndex());

            if(true == rootIdx.isValid())
            {
                if(false == isExpanded(rootIdx))
                {
                    expand(rootIdx);
                }
            }

            mbRootExpandingRequired = false;
        }
    }
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

        for( int i = static_cast<int>(eGroupedViewColumn::SubString);
             i < static_cast<int>(eGroupedViewColumn::AfterLastVisible);
             ++i )
        {
            auto column = index.model()->sibling(index.row(), i, index);

            switch(static_cast<eGroupedViewColumn>(i))
            {
                case eGroupedViewColumn::SubString:
                {
                    QString message = index.data().value<QString>();

                    for(int j = 0; j < item.getLevel(); ++j)
                    {
                        if(j == item.getLevel() - 1)
                        {
                            finalText.append("|-");
                        }
                        else
                        {
                            finalText.append("  ");
                        }
                    }

                    finalText.append(message);
                }
                    break;
                case eGroupedViewColumn::Messages:
                case eGroupedViewColumn::MessagesPerSecondAverage:
                case eGroupedViewColumn::Payload:
                case eGroupedViewColumn::PayloadPerSecondAverage:
                {
                    finalText.append(" | ").append(getName(static_cast<eGroupedViewColumn>(i))).append(" : ");
                    finalText.append( QString::number( column.data().value<int>() ) );
                }
                    break;
                case eGroupedViewColumn::PayloadPercantage:
                case eGroupedViewColumn::MessagesPercantage:
                {
                    finalText.append(" | ").append(getName(static_cast<eGroupedViewColumn>(i))).append(" : ");
                    finalText.append( QString::number( column.data().value<double>(), 'f', 3 ) );
                }
                    break;
                case eGroupedViewColumn::AfterLastVisible:
                case eGroupedViewColumn::Metadata:
                case eGroupedViewColumn::Last:
                    break;
            }
        }

        finalText.append("\n");
    }



    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->setText(finalText);
}

void CGroupedView::keyPressEvent ( QKeyEvent * event )
{
    if(true == NShortcuts::isCopyShortcut(event))
    {
        copyGroupedViewSelection();
    }
    else if(true == NShortcuts::isCollapseOneLevelShortcut(event))
    {
        auto selectedRows = selectionModel()->selectedRows();

        if(1 == selectedRows.size()) // if one row is selected
        {
            auto selectedRow = selectedRows[0];
            changeLevelExpansion(selectedRow, false);
        }
    }
    else if(true == NShortcuts::isExpadOneLevelShortcut(event))
    {
        auto selectedRows = selectionModel()->selectedRows();

        if(1 == selectedRows.size()) // if one row is selected
        {
            auto selectedRow = selectedRows[0];
            changeLevelExpansion(selectedRow, true);
        }
    }
    else
    {
        tParent::keyPressEvent(event);
    }
}

void CGroupedView::updateWidth()
{
    for(int i = static_cast<int>(eGroupedViewColumn::SubString);
        i < static_cast<int>(eGroupedViewColumn::AfterLastVisible);
        ++i)
    {
        resizeColumnToContents(i);
    }
}

void CGroupedView::updateColumnsVisibility()
{
    for( int i = static_cast<int>(eGroupedViewColumn::SubString);
         i < static_cast<int>(eGroupedViewColumn::Last);
         ++i)
    {
        if(i >= static_cast<int>(eGroupedViewColumn::AfterLastVisible))
        {
            hideColumn(i);
        }
        else
        {
            showColumn(i);
        }
    }
}
