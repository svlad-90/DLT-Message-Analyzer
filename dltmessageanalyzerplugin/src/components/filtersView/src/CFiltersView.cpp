#include "QElapsedTimer"
#include "QTimer"
#include "QPainter"
#include "QApplication"

#include "QHeaderView"
#include "QScrollBar"
#include "QMenu"
#include "QStyledItemDelegate"
#include "QClipboard"
#include "QLineEdit"
#include "QKeyEvent"
#include "QActionGroup"
#include "QTextEdit"

#include "CFiltersModel.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/log/api/CLog.hpp"
#include "common/CQtHelper.hpp"
#include "components/regexHistory/api/CRegexHistoryTextEdit.hpp"

#include "CFilterItemDelegate.hpp"
#include "../api/CFiltersView.hpp"

#include "DMA_Plantuml.hpp"

namespace NShortcuts
{
    static bool isEnter( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->key() == Qt::Key_Enter;
    }

    static bool isCopyShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_C;
    }
}

CFiltersView::CFiltersView(QWidget *parent):
    QTreeView(parent),
    mpModel(nullptr),
    mWidthUpdateRequired(eUpdateRequired::eUpdateRequired_NO),
    mbIsVerticalScrollBarVisible(false),
    mbResizeOnExpandCollapse(true),
    mbSkipFirstUpdateWidth(true),
    mpFilterItemDelegate(nullptr)
{
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setSectionsMovable(false);

    setSortingEnabled(true);
    sortByColumn( static_cast<int>(eRegexFiltersColumn::Index), Qt::SortOrder::AscendingOrder );

    connect(this, &QTreeView::doubleClicked, [this](const QModelIndex &index)
    {
        if(false == index.isValid())
        {
            return;
        }

        if( static_cast<eRegexFiltersColumn>(index.column()) == eRegexFiltersColumn::Value )
        {
            isExpanded(index) ? collapse(index) : expand(index);
        }
    });
}

void CFiltersView::setRegexInputField(CRegexHistoryTextEdit* pRegexTextEdit)
{
    mpRegexTextEdit = pRegexTextEdit;
}

void CFiltersView::copySelectedRowToClipboard()
{
    QClipboard* pClipboard = QApplication::clipboard();

    if(nullptr != pClipboard && nullptr != mpRegexTextEdit)
    {
        pClipboard->setText(mpRegexTextEdit->textCursor().selectedText());
    }
}

void CFiltersView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if(true == current.isValid())
    {
        auto pTreeItem = static_cast<CTreeItem*>(current.internalPointer());

        if(nullptr != pTreeItem &&
           nullptr != mpRegexTextEdit)
        {
            auto range = pTreeItem->data(static_cast<int>(eRegexFiltersColumn::Range)).get<tIntRange>();
            mpRegexTextEdit->setSelection(range.from, range.to - range.from + 1);
        }
    }

    tParent::currentChanged(current, previous);
}

CFiltersView::~CFiltersView()
{
    if(nullptr != mpFilterItemDelegate)
    {
        delete mpFilterItemDelegate;
        mpFilterItemDelegate = nullptr;
    }
}

void CFiltersView::setModel(QAbstractItemModel *model)
{
    tParent::setModel(model);
    updateColumnsVisibility();
}

void CFiltersView::updateColumnsVisibility()
{
    const tRegexFiltersColumnsVisibilityMap& visibilityMap = getSettingsManager()->getRegexFiltersColumnsVisibilityMap();

    for( int i = static_cast<int>(eRegexFiltersColumn::Value);
         i < static_cast<int>(eRegexFiltersColumn::Last);
         ++i)
    {
        if(i >= static_cast<int>(eRegexFiltersColumn::AfterLastVisible))
        {
            hideColumn(i);
        }
        else
        {
            auto foundColumn = visibilityMap.find(static_cast<eRegexFiltersColumn>(i));

            if( foundColumn != visibilityMap.end() )
            {
                if(true == foundColumn.value()) // if column is visible
                {
                    showColumn(i);
                }
                else
                {
                    hideColumn(i);
                }
            }
            else
            {
                 hideColumn(static_cast<int>(i));
            }
        }
    }
}

void CFiltersView::highlightInvalidRegex(const QModelIndex &index)
{
    if(true == index.isValid() && nullptr != mpFilterItemDelegate)
    {
        QVector<QModelIndex> animatedRows;
        animatedRows.push_back(index);
        mpFilterItemDelegate->animateRows(animatedRows, QColor(255,0,0), 300);
    }
}

void CFiltersView::setSpecificModel( CFiltersModel* pModel )
{
    if(nullptr != pModel)
    {
        mpModel = pModel;

        if(nullptr != mpFilterItemDelegate)
        {
            mpFilterItemDelegate->setSpecificModel(mpModel);
        }

        connect( mpModel, &CFiltersModel::filteredEntriesChanged, this,
                 [this](const CFiltersModel::tFilteredEntryVec& filteredEntries,
                 bool expandVisible)
        {
            mbResizeOnExpandCollapse = false;

            //QElapsedTimer timer;
            //timer.start();

            //SEND_MSG(QString("~0 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            if(false == expandVisible)
            {
                collapseAll();
            }

            //SEND_MSG(QString("~1 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            for(const auto& filteredEntry : filteredEntries)
            {
                setRowHidden(filteredEntry.row, filteredEntry.parentIdx, filteredEntry.filtered);

                if(true == expandVisible && false == filteredEntry.filtered)
                {
                    setExpanded(filteredEntry.parentIdx, true);
                    //SEND_MSG(QString("~2 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));
                }
            }

            //SEND_MSG(QString("~3 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            mbResizeOnExpandCollapse = true;

            if(false == mbSkipFirstUpdateWidth)
            {
                updateWidth();
            }
            else
            {
                mbSkipFirstUpdateWidth = false;
            }
        }, Qt::AutoConnection);
    }

    setModel(pModel);
}

void CFiltersView::updateWidth()
{
    for(int i = static_cast<int>(eRegexFiltersColumn::Value);
        i < static_cast<int>(eRegexFiltersColumn::AfterLastVisible);
        ++i)
    {
        resizeColumnToContents(i);
    }
}

void CFiltersView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
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
}

void CFiltersView::keyPressEvent ( QKeyEvent * event )
{
    if(true == NShortcuts::isEnter(event))
    {
        if(nullptr != mpRegexTextEdit)
        {
            mpRegexTextEdit->setFocus();
        }
    }
    else if(true == NShortcuts::isCopyShortcut(event))
    {
        copySelectedRowToClipboard();
    }
    else
    {
        tParent::keyPressEvent(event);
    }
}

void CFiltersView::handleSettingsManagerChange()
{
    if(nullptr == mpFilterItemDelegate)
    {
        mpFilterItemDelegate = new CFilterItemDelegate(this);
        mpFilterItemDelegate->setSettingsManager(getSettingsManager());
        setItemDelegate(mpFilterItemDelegate);
    }

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Copy", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copySelectedRowToClipboard();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Filter variables", this);
            connect(pAction, &QAction::triggered, [this](bool checked)
            {
                getSettingsManager()->setFilterVariables(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getFilterVariables());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Switch to regex edit", this);
            pAction->setShortcut(QKeySequence(tr("Enter")));
            connect(pAction, &QAction::triggered, [this]()
            {
                if(nullptr != mpRegexTextEdit)
                {
                    mpRegexTextEdit->setFocus();
                }
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Columns settings", this);

            {
                QMenu* pSubSubMenu = new QMenu("Visible columns", this);

                {
                    const auto& regexFiltersColumnsVisibilityMap =
                            getSettingsManager()->getRegexFiltersColumnsVisibilityMap();

                    for( int i = static_cast<int>(eRegexFiltersColumn::Value);
                         i < static_cast<int>(eRegexFiltersColumn::AfterLastVisible);
                         ++i)
                    {
                        auto foundItem = regexFiltersColumnsVisibilityMap.find(static_cast<eRegexFiltersColumn>(i));

                        if(foundItem != regexFiltersColumnsVisibilityMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<eRegexFiltersColumn>(i)), this);
                            connect(pAction, &QAction::triggered, [this, i](bool checked)
                            {
                                auto regexFiltersColumnsVisibilityMap_ =
                                        getSettingsManager()->getRegexFiltersColumnsVisibilityMap();

                                auto foundItem_ = regexFiltersColumnsVisibilityMap_.find(static_cast<eRegexFiltersColumn>(i));

                                if(foundItem_ != regexFiltersColumnsVisibilityMap_.end()) // if item is in the map
                                {
                                    foundItem_.value() = checked; // let's update visibility value
                                    getSettingsManager()->setRegexFiltersColumnsVisibilityMap(regexFiltersColumnsVisibilityMap_);
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
                    getSettingsManager()->resetRegexFiltersColumnsVisibilityMap();
                });
                pSubMenu->addAction(pAction);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Completion settings", this);

            {
                QAction* pAction = new QAction("Case sensitive", this);
                connect(pAction, &QAction::triggered, [this](bool checked)
                {
                    getSettingsManager()->setFiltersCompletion_CaseSensitive(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(getSettingsManager()->getFiltersCompletion_CaseSensitive());
                pSubMenu->addAction(pAction);
            }

            {
                QAction* pAction = new QAction("Max number of suggestions...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    int inputVal;

                    const auto& allowedRange = getSettingsManager()->getFiltersCompletion_MaxNumberOfSuggestions_AllowedRange();

                    if(true == allowedRange.isSet())
                    {
                        bool bInputSuccess = getRangedArithmeticValue<int>(inputVal,
                                                                           allowedRange.getValue().from,
                                                                           allowedRange.getValue().to,
                                                                           getSettingsManager()->getFiltersCompletion_MaxNumberOfSuggestions(),
                                                                           this,
                                                                           "max number of suggestions",
                                                                           "Max number of suggestions",
                                                                           [](bool* ok, const QString& str)->int
                                                                           {
                                                                               return str.toInt(ok);
                                                                           });

                        if(true == bInputSuccess)
                        {
                            getSettingsManager()->setFiltersCompletion_MaxNumberOfSuggestions(inputVal);
                        }
                    }
                });
                pSubMenu->addAction(pAction);
            }

            {
                QAction* pAction = new QAction("Max characters in suggestion...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    int inputVal;

                    const auto& allowedRange = getSettingsManager()->getFiltersCompletion_MaxCharactersInSuggestion_AllowedRange();

                    if(true == allowedRange.isSet())
                    {
                        bool bInputSuccess = getRangedArithmeticValue<int>(inputVal,
                                                                           allowedRange.getValue().from,
                                                                           allowedRange.getValue().to,
                                                                           getSettingsManager()->getFiltersCompletion_MaxCharactersInSuggestion(),
                                                                           this,
                                                                           "max characters in suggestion",
                                                                           "Max characters in suggestion",
                                                                           [](bool* ok, const QString& str)->int
                                                                           {
                                                                               return str.toInt(ok);
                                                                           });

                        if(true == bInputSuccess)
                        {
                            getSettingsManager()->setFiltersCompletion_MaxCharactersInSuggestion(inputVal);
                        }
                    }
                });
                pSubMenu->addAction(pAction);
            }

            {
                QAction* pAction = new QAction("Pop-up width...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    int inputVal;

                    const auto& allowedRange = getSettingsManager()->getFiltersCompletion_CompletionPopUpWidth_AllowedRange();

                    if(true == allowedRange.isSet())
                    {
                        bool bInputSuccess = getRangedArithmeticValue<int>(inputVal,
                                                                           allowedRange.getValue().from,
                                                                           allowedRange.getValue().to,
                                                                           getSettingsManager()->getFiltersCompletion_CompletionPopUpWidth(),
                                                                           this,
                                                                           "pop up width",
                                                                           "Pop up width",
                                                                           [](bool* ok, const QString& str)->int
                                                                           {
                                                                               return str.toInt(ok);
                                                                           });

                        if(true == bInputSuccess)
                        {
                            getSettingsManager()->setFiltersCompletion_CompletionPopUpWidth(inputVal);
                        }
                    }
                });
                pSubMenu->addAction(pAction);
            }

            {
                QMenu* pSubSubMenu = new QMenu("Search policy", this);

                QActionGroup* pActionGroup = new QActionGroup(this);
                pActionGroup->setExclusive(true);

                {
                    QAction* pAction = new QAction("\"Starts with\"", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        getSettingsManager()->setFiltersCompletion_SearchPolicy(false);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(!getSettingsManager()->getFiltersCompletion_SearchPolicy());

                    pSubSubMenu->addAction(pAction);
                    pActionGroup->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("\"Contains\"", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        getSettingsManager()->setFiltersCompletion_SearchPolicy(true);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(getSettingsManager()->getFiltersCompletion_SearchPolicy());

                    pSubSubMenu->addAction(pAction);
                    pActionGroup->addAction(pAction);
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect( getSettingsManager().get(),
             &ISettingsManager::regexFiltersColumnsVisibilityMapChanged,
             this, [this](const tRegexFiltersColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
        updateWidth();
    });
}

PUML_PACKAGE_BEGIN(DMA_FiltersView_API)
    PUML_CLASS_BEGIN_CHECKED(CFiltersView)
        PUML_INHERITANCE_CHECKED(QTreeView, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CFilterItemDelegate, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CFiltersModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CRegexHistoryTextEdit, 1, 1, regex input field)
    PUML_CLASS_END()
PUML_PACKAGE_END()
