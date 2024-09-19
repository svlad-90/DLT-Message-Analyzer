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

#include "../api/CGroupedView.hpp"
#include "common/CTreeItem.hpp"
#include "components/log/api/CLog.hpp"
#include "components/settings/api/ISettingsManager.hpp"

#include "../api/IGroupedViewModel.hpp"

#include "DMA_Plantuml.hpp"

class CDoubleDelegate: public QStyledItemDelegate
{
    public:

    CDoubleDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) { }
    virtual ~CDoubleDelegate() { }

    virtual QString displayText(const QVariant &value, const QLocale &locale) const
    {
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (value.type() == QVariant::Double)
        #else
        if (value.userType() == QMetaType::Double)
        #endif
        {
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

    static bool isHighlightLinesShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_H;
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

            auto* const pMyElement = static_cast<tTreeItemPtr>(mModelIndex.internalPointer());
            auto* const pValElement = static_cast<tTreeItemPtr>(val.mModelIndex.internalPointer());

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

    setUniformRowHeights(true);
}

void CGroupedView::handleSettingsManagerChange()
{
    connect( getSettingsManager().get(),
             &ISettingsManager::groupedViewColumnsVisibilityMapChanged,
             this, [this](const tGroupedViewColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
        updateWidth();
    });

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

            if(true == selectionModel()->selectedRows().isEmpty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);

            contextMenu.addSeparator();
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Columns settings", this);

            {
                QMenu* pSubSubMenu = new QMenu("Visible columns", this);

                {
                    const auto& gorupedViewColumnsVisibilityMap =
                            getSettingsManager()->getGroupedViewColumnsVisibilityMap();

                    for( int i = static_cast<int>(eGroupedViewColumn::SubString);
                         i < static_cast<int>(eGroupedViewColumn::AfterLastVisible);
                         ++i)
                    {
                        auto foundItem = gorupedViewColumnsVisibilityMap.find(static_cast<eGroupedViewColumn>(i));

                        if(foundItem != gorupedViewColumnsVisibilityMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<eGroupedViewColumn>(i)), this);
                            connect(pAction, &QAction::triggered, [this, i](bool checked)
                            {
                                auto groupedViewColumnsVisibilityMap_ =
                                        getSettingsManager()->getGroupedViewColumnsVisibilityMap();

                                auto foundItem_ = groupedViewColumnsVisibilityMap_.find(static_cast<eGroupedViewColumn>(i));

                                if(foundItem_ != groupedViewColumnsVisibilityMap_.end()) // if item is in the map
                                {
                                    foundItem_.value() = checked; // let's update visibility value
                                    getSettingsManager()->setGroupedViewColumnsVisibilityMap(groupedViewColumnsVisibilityMap_);
                                }
                            });
                            pAction->setCheckable(true);
                            pAction->setChecked(foundItem.value());
                            pSubSubMenu->addAction(pAction);
                        }
                    }
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            {
                QAction* pAction = new QAction("Reset visible columns", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    getSettingsManager()->resetGroupedViewColumnsVisibilityMap();
                });
                pSubMenu->addAction(pAction);
            }

            pSubMenu->addSeparator();

            {
                QMenu* pSubSubMenu = new QMenu("Copy columns", this);

                {
                    const auto& gorupedViewColumnsCopyPasteMap =
                            getSettingsManager()->getGroupedViewColumnsCopyPasteMap();

                    for( int i = static_cast<int>(eGroupedViewColumn::SubString);
                         i < static_cast<int>(eGroupedViewColumn::AfterLastVisible);
                         ++i)
                    {
                        auto foundItem = gorupedViewColumnsCopyPasteMap.find(static_cast<eGroupedViewColumn>(i));

                        if(foundItem != gorupedViewColumnsCopyPasteMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<eGroupedViewColumn>(i)), this);
                            connect(pAction, &QAction::triggered, [this, i](bool checked)
                            {
                                auto groupedViewColumnsCopyPasteMap_ =
                                        getSettingsManager()->getGroupedViewColumnsCopyPasteMap();

                                auto foundItem_ = groupedViewColumnsCopyPasteMap_.find(static_cast<eGroupedViewColumn>(i));

                                if(foundItem_ != groupedViewColumnsCopyPasteMap_.end()) // if item is in the map
                                {
                                    foundItem_.value() = checked; // let's update copy paste value
                                    getSettingsManager()->setGroupedViewColumnsCopyPasteMap(groupedViewColumnsCopyPasteMap_);
                                }
                            });
                            pAction->setCheckable(true);
                            pAction->setChecked(foundItem.value());
                            pSubSubMenu->addAction(pAction);
                        }
                    }
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            {
                QAction* pAction = new QAction("Reset copy columns", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    getSettingsManager()->resetGroupedViewColumnsCopyPasteMap();
                });
                pSubMenu->addAction(pAction);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                    if( true == isExpanded(selectedRow.child(i, 0) ) )
#else
                    if( true == isExpanded(model()->index(i, 0, selectedRow) ) )
#endif
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

        contextMenu.addSeparator();

        {
            auto selectedRows = selectionModel()->selectedRows();

            if(1 == selectedRows.size()) // if one row is selected
            {
                {
                    QAction* pAction = new QAction("Highlight in search view", this);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+H")));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        highlightLines();
                    });

                    contextMenu.addAction(pAction);
                }
            }
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );
}

void CGroupedView::highlightLines()
{
    auto selectedRows = selectionModel()->selectedRows();

    if(1 == selectedRows.size()) // if one row is selected
    {
        auto& selectedRow = selectedRows[0];
        auto* pModel = model();

        if(nullptr != pModel)
        {
            auto* pSpecificModel = static_cast<IGroupedViewModel*>(pModel);
            searchViewHighlightingRequested(pSpecificModel->getAllMessageIds(selectedRow));
        }
    }
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    expand( expandIdx.child(i, 0) );
#else
    expand( model()->index(i,0,expandIdx) );
#endif
        }
    }
    else if(false == bExpand && true == isExpanded(expandIdx))
    {
        bool bAnyChildExpanded = false;

        const int childCount = model()->rowCount( expandIdx );
        for(int i = 0; i < childCount; ++i)
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    auto childIdx = expandIdx.child(i, 0);
#else
    auto childIdx = model()->index(i,0,expandIdx);
#endif

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

    const auto& copyColumns = getSettingsManager()->getGroupedViewColumnsCopyPasteMap();

    bool bAnythingToCopy = false;

    for(const auto& copyColumn : copyColumns)
    {
        if(true == copyColumn)
        {
            bAnythingToCopy = true;
            break;
        }
    }

    if(true == bAnythingToCopy)
    {
        for(const auto& item : selectedRowSet)
        {
            auto index = item.getModelIndex();

            for( int i = static_cast<int>(eGroupedViewColumn::SubString);
                 i < static_cast<int>(eGroupedViewColumn::AfterLastVisible);
                 ++i )
            {
                auto foundVisibleColumn = copyColumns.find(static_cast<eGroupedViewColumn>(i));

                if(foundVisibleColumn != copyColumns.end() && true == foundVisibleColumn.value())
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
            }

            finalText.append("\n");
        }
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
    else if(true == NShortcuts::isHighlightLinesShortcut(event))
    {
        highlightLines();
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
    const tGroupedViewColumnsVisibilityMap& visibilityMap = getSettingsManager()->getGroupedViewColumnsVisibilityMap();

    for( int i = static_cast<int>(eGroupedViewColumn::SubString);
         i < static_cast<int>(eGroupedViewColumn::Last);
         ++i)
    {
        auto foundColumn = visibilityMap.find(static_cast<eGroupedViewColumn>(i));

        if( foundColumn != visibilityMap.end() )
        {
            if(true == foundColumn.value()) // if column is visible
            {
                showColumn(i);
            }
            else
            {
                hideColumn(static_cast<int>(i));
            }
        }
        else
        {
             hideColumn(static_cast<int>(i));
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_GroupedView_API)
    PUML_CLASS_BEGIN_CHECKED(CGroupedView)
        PUML_INHERITANCE_CHECKED(QTreeView, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
