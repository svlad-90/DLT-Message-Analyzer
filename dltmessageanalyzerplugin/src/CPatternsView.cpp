/**
 * @file    CPatternsView.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CPatternsView class
 */

#include "CPatternsView.hpp"

#include <functional>

#include <QKeyEvent>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QDebug>
#include <QMap>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QAbstractScrollArea>
#include <QColorDialog>
#include <QTableWidget>
#include <QPalette>
#include <QModelIndex>
#include <QLineEdit>
#include <QDesktopServices>
#include <QClipboard>

#include "CSettingsManager.hpp"
#include "CPatternsModel.hpp"
#include "CConsoleCtrl.hpp"

namespace NShortcuts
{
    static bool isApplyCombinationShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->matches(QKeySequence::InsertParagraphSeparator);
    }

    static bool isSetCombineShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->key() == Qt::Key_Space;
    }

    static bool isClearSeletedPatternsShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->modifiers() & Qt::ShiftModifier && pEvent->key() == Qt::Key_R;
    }

    static bool isResetSeletedPatternsShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_R;
    }

    static bool isDeletePatternShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->key() == Qt::Key_Delete;
    }

    static bool isExpadAllShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_E;
    }

    static bool isCollapseAllShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->modifiers() & Qt::ShiftModifier && pEvent->key() == Qt::Key_E;
    }

    static bool isCopyShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_C;
    }

    static bool isPasteShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_V;
    }
}

static Qt::CheckState getOppositeCheckState(Qt::CheckState val)
{
    auto newCheckState = Qt::Unchecked;

    switch(val)
    {
        case Qt::Checked:
        newCheckState = Qt::Unchecked;
            break;
        case Qt::Unchecked:
        newCheckState = Qt::Checked;
            break;
        case Qt::PartiallyChecked:
        newCheckState = Qt::Checked;
            break;
    }

    return newCheckState;
}

class CTreeRepresentationDelegate : public QStyledItemDelegate
{
public:
    CTreeRepresentationDelegate(QTreeView* pParentTree)
        : QStyledItemDelegate(pParentTree),
          mRowAnimationDataMap(),
          mTime(),
          mUpdateTimer(),
          mpParentTree(pParentTree)
    {
        mTime.start();
    }

    void animateRows(const QVector<QModelIndex>& rows, const QColor& startColor, const QColor& intermediateColor, const QColor& endColor, const int& duration)
    {
        mRowAnimationDataMap.clear();

        const auto columnsNumber = mpParentTree->model()->columnCount();

        for(const auto& row : rows)
        {
            int startTime = mTime.elapsed();

            for(int columnId = 0; columnId < columnsNumber; ++columnId)
            {
                if(false == mpParentTree->isColumnHidden(columnId))
                {
                    auto animatedIdx = row.sibling(row.row(), columnId);
                    mRowAnimationDataMap.insert( animatedIdx, tRowAnimationData(startColor, intermediateColor, endColor, startTime, startTime + duration) );
                }
            }
        }

        auto updateAnimatedRows = [this, columnsNumber]()
        {
            for( auto it = mRowAnimationDataMap.begin(); it != mRowAnimationDataMap.end(); ++it )
            {
                for(int columnId = 0; columnId < columnsNumber; ++columnId)
                {
                    if(false == mpParentTree->isColumnHidden(columnId))
                    {
                        QModelIndex updateIdx = it.key().sibling(it.key().row(), columnId);
                        mpParentTree->update( updateIdx );
                    }
                }
            }
        };

        if(nullptr != mpParentTree)
        {
            updateAnimatedRows();
            mUpdateTimer.start(16);
        }

        connect(&mUpdateTimer, &QTimer::timeout, [this, updateAnimatedRows]()
        {
            if(false == mRowAnimationDataMap.empty())
            {
                if(nullptr != mpParentTree)
                {
                    updateAnimatedRows();
                    mUpdateTimer.start(16);
                }

                 excludeAnimatedRows();
            }
        });
    }

    // as only visible rows are rendered, we need to exclude some of them on each timer hit
    // otherwise the high CPU load is observed in case if some of animated rows are outside the visible area
    void excludeAnimatedRows()
    {
        for( auto it = mRowAnimationDataMap.begin(); it != mRowAnimationDataMap.end(); )
        {
            auto& animatedObject = *it;

            int nowToStartDiff = mTime.elapsed() - animatedObject.startTime;
            int endToStartDiff = animatedObject.endTime - animatedObject.startTime;

            int passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

            if(passedTimePercantage > 100)
            {
                it = mRowAnimationDataMap.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {   
        if(nullptr == painter)
        {
            return;
        }

        painter->save();

        QStyleOptionViewItemV4 viewItemOption(option);

        if (toPatternsColumn(index.column()) == ePatternsColumn::Default ||
            toPatternsColumn(index.column()) == ePatternsColumn::Combine)
        {
            const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            QRect newRect = QStyle::alignedRect(viewItemOption.direction, Qt::AlignCenter,
                                                QSize(viewItemOption.decorationSize.width() + 5,option.decorationSize.height()),
                                                QRect(viewItemOption.rect.x() + textMargin, option.rect.y(),
                                                      viewItemOption.rect.width() - (2 * textMargin), viewItemOption.rect.height()));
            viewItemOption.rect = newRect;
        }

        auto foundAnimatedObject = const_cast<tRowAnimationDataMap*>(&mRowAnimationDataMap)->find( index );

        if(foundAnimatedObject != mRowAnimationDataMap.end())
        {
            int nowToStartDiff = mTime.elapsed() - foundAnimatedObject->startTime;
            int endToStartDiff = foundAnimatedObject->endTime - foundAnimatedObject->startTime;

            int passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

            //qDebug() << "nowToStartDiff - " << nowToStartDiff << "; endToStartDiff - " << endToStartDiff << "; passedTimePercantage - " << passedTimePercantage;

            if( passedTimePercantage >= 100.0 ) // last frame
            {
                painter->fillRect( viewItemOption.rect, foundAnimatedObject.value().endColor );
                const_cast<tRowAnimationDataMap*>(&mRowAnimationDataMap)->erase(foundAnimatedObject);

                //qDebug() << "Last frame; Current color - red: " << QString::number(foundAnimatedObject.value().endColor.red())
                //         << "green: " << QString::number(foundAnimatedObject.value().endColor.green())
                //         << "blue: " << QString::number(foundAnimatedObject.value().endColor.blue());
            }
            else
            {
                auto calcColor = [](const QColor& fromColor, const QColor& toColor, const int& percentage)->QColor
                {
                    //qDebug() << "Next frame: "
                    //         << "fromColor.red(): " << fromColor.red() << "; "
                    //         << "fromColor.green(): " << fromColor.green() << "; "
                    //         << "fromColor.blue(): " << fromColor.blue() << "; "
                    //         << "toColor.red(): " << toColor.red() << "; "
                    //         << "toColor.green(): " << toColor.green() << "; "
                    //         << "toColor.blue(): " << toColor.blue() << ";";

                    int rDifference = ( toColor.red() - fromColor.red() );
                    int r = fromColor.red() + static_cast<int>( rDifference * percentage / 100 );
                    int gDifference = ( toColor.green() - fromColor.green() );
                    int g = fromColor.green() + static_cast<int>( gDifference * percentage / 100 );
                    int bDifference = ( toColor.blue() - fromColor.blue() );
                    int b = fromColor.blue() + static_cast<int>( bDifference * percentage / 100 );

                    //qDebug() << "Next frame: rDifference: " << rDifference << "; "
                    //         << "r: " << r << "; "
                    //         << "gDifference: " << gDifference << "; "
                    //         << "g: " << g << "; "
                    //         << "bDifference: " << bDifference << "; "
                    //         << "b: " << b << "; "
                    //         << "percentage: " << percentage << ";";

                    return QColor( r, g, b );
                };

                bool bHalfPassed = passedTimePercantage > 50.0;

                QColor color = bHalfPassed ?
                            calcColor(foundAnimatedObject->intermediateColor, foundAnimatedObject->endColor, passedTimePercantage) :
                            calcColor(foundAnimatedObject->startColor, foundAnimatedObject->intermediateColor, passedTimePercantage);


                painter->fillRect( viewItemOption.rect, color );
            }
        }

        painter->restore();

        QStyledItemDelegate::paint(painter, viewItemOption, index);
    }

    struct tRowAnimationData
    {
        tRowAnimationData(const QColor& startColor_,
                          const QColor& intermediateColor_,
                          const QColor& endColor_,
                          const int& startTime_,
                          const int& endTime_):
            startColor(startColor_),
            intermediateColor(intermediateColor_),
            endColor(endColor_),
            startTime(startTime_),
            endTime(endTime_)
        {}

        QColor startColor;
        QColor intermediateColor;
        QColor endColor;
        int startTime;
        int endTime;
    };

    typedef QMap<QModelIndex, tRowAnimationData> tRowAnimationDataMap;
    tRowAnimationDataMap mRowAnimationDataMap;
    QTime mTime;
    QTimer mUpdateTimer;
    QTreeView* mpParentTree;
};

CPatternsView::CPatternsView(QWidget *parent):
    tParent(parent),
    mpRepresentationDelegate(new CTreeRepresentationDelegate(this)),
    mpModel(nullptr),
    mpPatternsSearchInput(nullptr),
    mCopyPastePatternData()
{
    header()->setSectionsMovable(false);

    setExpandsOnDoubleClick(false); // we will have our own logic

    setSortingEnabled(true);
    sortByColumn( static_cast<int>(ePatternsColumn::AliasTreeLevel), Qt::SortOrder::AscendingOrder );

    updateColumnsVisibility();

    connect(this, &QTreeView::doubleClicked, [this](const QModelIndex &index)
    {
        if(false == index.isValid())
        {
            return;
        }

        if(toPatternsColumn(index.column()) != ePatternsColumn::Default &&
           toPatternsColumn(index.column()) != ePatternsColumn::Combine)
        {
            auto selectedRows = selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Regex));
            if(false == selectedRows.empty())
            {
                auto rowType = selectedRows[0].sibling(selectedRows[0].row(),
                                                       static_cast<int>(ePatternsColumn::RowType))
                        .data().value<ePatternsRowType>();

                if(ePatternsRowType::ePatternsRowType_Alias == rowType)
                {
                    const QString regex = selectedRows[0].data().value<QString>();

                    // we are applying selection only to those elements which have non-empty regex value
                    if(false == regex.isEmpty())
                    {
                        patternSelected( regex );
                    }
                }
            }
        }

        if(toPatternsColumn(index.column()) == ePatternsColumn::AliasTreeLevel)
        {
            isExpanded(index)? collapse(index) : expand(index);
        }
    });

    connect(this, &QTableView::clicked, [this](const QModelIndex &index)
    {
        if(false == index.isValid())
        {
            return;
        }

        if(toPatternsColumn(index.column()) == ePatternsColumn::Default)
        {
            if(nullptr != mpModel)
            {
                auto siblingIdx = index.sibling(index.row(), static_cast<int>(ePatternsColumn::Alias));
                QString alias = siblingIdx.data().value<QString>();

                auto checkState = V_2_CS( index.data() );
                auto oppositeCheckState = getOppositeCheckState(checkState);
                mpModel->setIsDefault(index, oppositeCheckState, true, true);
                mpModel->updatePatternsInPersistency();
            }
        }
        else if(toPatternsColumn(index.column()) == ePatternsColumn::Combine)
        {
            auto checkState = V_2_CS( index.data() );

            if(nullptr != mpModel)
            {
                mpModel->setIsCombine( index, getOppositeCheckState(checkState), true, true );
            }
        }
    });

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        if(false == selectionModel()->selectedRows().empty())
        {
            {
                QAction* pAction = new QAction("Edit ...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    editPatternTriggered();
                });
                contextMenu.addAction(pAction);
            }

            {
                QAction* pAction = new QAction("Delete ...", this);
                pAction->setShortcut(QKeySequence(Qt::Key_Delete));
                connect(pAction, &QAction::triggered, [this]()
                {
                    deletePatternTriggered();
                });
                contextMenu.addAction(pAction);
            }

            contextMenu.addSeparator();

            auto selectedRow = selectionModel()->selectedRows().operator[](0);

            if(ePatternsRowType::ePatternsRowType_Alias ==
               selectedRow.sibling(selectedRow.row(), static_cast<int>(ePatternsColumn::RowType)).data().value<ePatternsRowType>())
            {
                QAction* pAction = new QAction("Overwrite from input field ...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    overwriteFromInputFieldTriggered();
                });
                contextMenu.addAction(pAction);

                contextMenu.addSeparator();
            }
        }

        {
            QAction* pAction = new QAction("Copy", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copySelectedRow();
            });
            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Paste", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+V")));
            connect(pAction, &QAction::triggered, [this]()
            {
                pasteSelectedRow();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            {
                QAction* pAction = new QAction("Expand all", this);
                pAction->setShortcut(QKeySequence(tr("Ctrl+E")));
                connect(pAction, &QAction::triggered, [this]()
                {
                    expandAll();
                });
                contextMenu.addAction(pAction);
            }

            {
                QAction* pAction = new QAction("Collapse all", this);
                pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+E")));
                connect(pAction, &QAction::triggered, [this]()
                {
                    collapseAll();
                });
                contextMenu.addAction(pAction);
            }

            contextMenu.addSeparator();
        }

        {
            if(true == mpModel->areAnyCombinedPatternsAvailable())
            {
                QAction* pAction = new QAction("Run combination", this);
                pAction->setShortcut(QKeySequence(Qt::Key_Enter));
                connect(pAction, &QAction::triggered, [this]()
                {
                    QString finalRegex = createCombinedRegex();
                    patternSelected( finalRegex );
                });
                contextMenu.addAction(pAction);
            }
        }

        {
            auto selectedRows = selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Regex));
            if(false == selectedRows.empty())
            {
                QAction* pAction = new QAction("Run selected item ( double click )", this);
                connect(pAction, &QAction::triggered, [this, selectedRows]()
                {
                    patternSelected( selectedRows[0].data().value<QString>() );
                });
                contextMenu.addAction(pAction);
            }
        }

        {
            QAction* pAction = new QAction("Reset \"comb.\" patterns to default", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+R")));
            connect(pAction, &QAction::triggered, [this]()
            {
                if(nullptr != mpModel)
                {
                    mpModel->resetPatternsToDefault();
                }
            });
            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Clear \"comb.\" patterns", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+R")));
            connect(pAction, &QAction::triggered, [this]()
            {
                if(nullptr != mpModel)
                {
                    mpModel->clearSelectedPatterns();
                }
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Visible columns", this);

            {
                const auto& patternsColumnsVisibilityMap =
                        CSettingsManager::getInstance()->getPatternsColumnsVisibilityMap();

                for( int i = static_cast<int>(ePatternsColumn::AliasTreeLevel);
                     i < static_cast<int>(ePatternsColumn::AfterLastVisible);
                     ++i)
                {
                    auto foundItem = patternsColumnsVisibilityMap.find(static_cast<ePatternsColumn>(i));

                    if(foundItem != patternsColumnsVisibilityMap.end())
                    {
                        QAction* pAction = new QAction(getName(static_cast<ePatternsColumn>(i)), this);
                        connect(pAction, &QAction::triggered, [i](bool checked)
                        {
                            auto patternsColumnsVisibilityMap_ =
                                    CSettingsManager::getInstance()->getPatternsColumnsVisibilityMap();

                            auto foundItem_ = patternsColumnsVisibilityMap_.find(static_cast<ePatternsColumn>(i));

                            if(foundItem_ != patternsColumnsVisibilityMap_.end()) // if item is in the map
                            {
                                foundItem_.value() = checked; // let's update visibility value
                                CSettingsManager::getInstance()->setPatternsColumnsVisibilityMap(patternsColumnsVisibilityMap_);
                            }
                        });
                        pAction->setCheckable(true);
                        pAction->setChecked(foundItem.value());
                        pSubMenu->addAction(pAction);
                    }
                }
            }

            contextMenu.addMenu(pSubMenu);
        }

        {
            QAction* pAction = new QAction("Reset visible columns", this);
            connect(pAction, &QAction::triggered, []()
            {
                CSettingsManager::getInstance()->resetPatternsColumnsVisibilityMap();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Highlight \"combined\" patterns", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setHighlightActivePatterns(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getHighlightActivePatterns());
            contextMenu.addAction(pAction);
        }

        {
            if(true == CSettingsManager::getInstance()->getHighlightActivePatterns())
            {
                QAction* pAction = new QAction("Highlighting color ...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    QColor color = QColorDialog::getColor( CSettingsManager::getInstance()->getPatternsHighlightingColor(), this );
                    if( color.isValid() )
                    {
                        CSettingsManager::getInstance()->setPatternsHighlightingColor(color);
                    }
                });
                contextMenu.addAction(pAction);
            }
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Open regex patterns storage", this);
            connect(pAction, &QAction::triggered, []()
            {
                SEND_MSG(QString("[CPatternsView]: Attempt to open path - \"%1\"")
                         .arg(CSettingsManager::getInstance()->getRegexDirectoryFull()));

                QDesktopServices::openUrl( QUrl::fromLocalFile( CSettingsManager::getInstance()->getRegexDirectoryFull() ) );
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Minimize on selection", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setMinimizePatternsViewOnSelection(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getMinimizePatternsViewOnSelection());
            contextMenu.addAction(pAction);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect( CSettingsManager::getInstance().get(),
             &CSettingsManager::patternsColumnsVisibilityMapChanged,
             [this](const tPatternsColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
    });

    setItemDelegate(mpRepresentationDelegate);
}

CPatternsView::~CPatternsView()
{
    if(nullptr != mpRepresentationDelegate)
    {
        delete mpRepresentationDelegate;
        mpRepresentationDelegate = nullptr;
    }
}

void CPatternsView::setModel(QAbstractItemModel *model)
{
    tParent::setModel(model);
    updateColumnsVisibility();
}

void CPatternsView::setPatternsSearchInput( QLineEdit* pPatternsSearchInput )
{
    mpPatternsSearchInput = pPatternsSearchInput;

    if(nullptr != mpPatternsSearchInput)
    {
        mpPatternsSearchInput->installEventFilter(this);
    }
}

bool CPatternsView::eventFilter(QObject* pObj, QEvent* pEvent)
{
    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);

        if(NShortcuts::isApplyCombinationShortcut(pKeyEvent) ||
           NShortcuts::isClearSeletedPatternsShortcut(pKeyEvent) ||
           NShortcuts::isResetSeletedPatternsShortcut(pKeyEvent) ||
           NShortcuts::isCollapseAllShortcut(pKeyEvent) ||
           NShortcuts::isExpadAllShortcut(pKeyEvent))
        {
            keyPressEvent(pKeyEvent);
        }
    }

    return QObject::eventFilter(pObj, pEvent);
}

QString CPatternsView::createCombinedRegex()
{
    QString finalRegex;

    bool firstInjection = true;

    typedef std::function<bool(const QModelIndex& idx)> tComparator;

    QSet<QString> usedPatterns;

    auto fillInPatterns = [this, &firstInjection, &finalRegex, &usedPatterns](const tComparator comparator, bool animate, const QColor& initialColor)
    {
        QVector<QModelIndex> highlightedRows;

        auto preVisitFunc = [this,
                &firstInjection,
                &finalRegex,
                &usedPatterns,
                &comparator,
                &animate,
                &highlightedRows](const QModelIndex& idx)
        {
            auto alias = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Alias)).data().value<QString>();

            if( comparator(idx) )
            {
                auto foundUsedPattern = usedPatterns.find( alias );

                if(foundUsedPattern == usedPatterns.end())
                {
                    if( false == firstInjection )
                    {
                        finalRegex.append("|");
                    }

                    finalRegex.append( idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Regex)).data().value<QString>() );
                    firstInjection = false;

                    if(nullptr != mpRepresentationDelegate)
                    {
                        bool bHighlight = CSettingsManager::getInstance()->getHighlightActivePatterns();

                        if(true == bHighlight && true == animate)
                        {
                            highlightedRows.push_back(idx);
                        }
                    }

                    usedPatterns.insert(alias);
                }
            }

            return true;
        };

        mpModel->visit(preVisitFunc, CPatternsModel::tVisitFunction());

        if(nullptr != mpRepresentationDelegate && false == highlightedRows.empty())
        {
            QColor highlightingColor = CSettingsManager::getInstance()->getPatternsHighlightingColor();
            mpRepresentationDelegate->animateRows(highlightedRows, initialColor, highlightingColor, initialColor, 250 );
            viewport()->update();
        }
    };

    QColor initialColor = palette().color(QPalette::ColorRole::Base);

    fillInPatterns( [this](const QModelIndex& idx)
    {
        bool bResult = false;

        auto rowTypeIdx = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::RowType));
        auto rowType = rowTypeIdx.data().value<ePatternsRowType>();
        bool bIsValid = idx.isValid();
        auto combineIdx = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Combine));
        auto bIsCombine = V_2_CS( combineIdx.data() );
        bool bIsRowSelected = selectionModel()->isRowSelected(idx.row(), idx.parent());
        bResult = bIsValid && bIsCombine == Qt::Checked && !bIsRowSelected && rowType == ePatternsRowType::ePatternsRowType_Alias;

        //SEND_MSG(QString("patternName - %1 "
        //                 "bIsValid - %2, "
        //                 "bIsCombine - %3, "
        //                 "bIsRowSelected - %4, "
        //                 "rowType - %5")
        //               .arg(idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Alias)).data().value<QString>())
        //               .arg(bIsValid)
        //               .arg(bIsCombine)
        //               .arg(bIsRowSelected)
        //               .arg(static_cast<int>(rowType)));

        return bResult;
    }, true, initialColor);

    fillInPatterns( [this](const QModelIndex& idx)
    {
        bool bResult = false;

        auto rowTypeIdx = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::RowType));
        auto rowType = rowTypeIdx.data().value<ePatternsRowType>();
        bool bIsValid = idx.isValid();
        auto combineIdx = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Combine));
        auto bIsCombine = V_2_CS( combineIdx.data() );
        bool bIsRowSelected = selectionModel()->isRowSelected(idx.row(), idx.parent());
        bResult = bIsValid && bIsCombine == Qt::Checked && bIsRowSelected && rowType == ePatternsRowType::ePatternsRowType_Alias;

        //SEND_MSG(QString("patternName - %1 "
        //                 "bIsValid - %2, "
        //                 "bIsCombine - %3, "
        //                 "bIsRowSelected - %4, "
        //                 "rowType - %5")
        //               .arg(idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Alias)).data().value<QString>())
        //               .arg(bIsValid)
        //               .arg(bIsCombine)
        //               .arg(bIsRowSelected)
        //               .arg(static_cast<int>(rowType)));

        return bResult;
    }, false, QColor());

    return finalRegex;
}

void CPatternsView::applyPatternsCombination()
{
    if( true == mpModel->areAnyCombinedPatternsAvailable() )
    {
        QString finalRegex = createCombinedRegex();
        patternSelected( finalRegex );
    }
}

void CPatternsView::keyPressEvent ( QKeyEvent * pEvent )
{
    if(NShortcuts::isApplyCombinationShortcut(pEvent))
    {
        if( true == mpModel->areAnyCombinedPatternsAvailable() )
        {
            QString finalRegex = createCombinedRegex();
            patternSelected( finalRegex );
        }
    }
    else if(NShortcuts::isSetCombineShortcut(pEvent))
    {
        auto selectedRows = selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Combine));

        for(const auto& selectedRow : selectedRows)
        {
            auto checkState = V_2_CS( selectedRow.data() );

            if(nullptr != mpModel)
            {
                mpModel->setIsCombine( selectedRow, getOppositeCheckState( checkState ), true, true );
            }
        }
    }
    else if(NShortcuts::isClearSeletedPatternsShortcut(pEvent))
    {
        if(nullptr != mpModel)
        {
            mpModel->clearSelectedPatterns();
        }
    }
    else if(NShortcuts::isResetSeletedPatternsShortcut(pEvent))
    {
        if(nullptr != mpModel)
        {
            mpModel->resetPatternsToDefault();
        }
    }
    else if(NShortcuts::isDeletePatternShortcut(pEvent))
    {
        deletePatternTriggered();
    }
    else if(NShortcuts::isCollapseAllShortcut(pEvent))
    {
        collapseAll();
    }
    else if(NShortcuts::isCopyShortcut(pEvent))
    {
        copySelectedRow();
    }
    else if(NShortcuts::isPasteShortcut(pEvent))
    {
        pasteSelectedRow();
    }
    else if(NShortcuts::isExpadAllShortcut(pEvent))
    {
        expandAll();
    }
    else
    {
        tParent::keyPressEvent(pEvent);
    }
}

void CPatternsView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    //if(hint == QAbstractItemView::EnsureVisible)
    //    return;
    QTreeView::scrollTo(index, hint);
}

void CPatternsView::setSpecificModel( CPatternsModel* pModel )
{
    if(nullptr != pModel)
    {
        mpModel = pModel;

        connect( mpModel, &CPatternsModel::filteredEntriesChanged,
                 [this](const CPatternsModel::tFilteredEntryVec& filteredEntries,
                 bool expandVisible)
        {
            //QTime timer;
            //timer.start();

            //SEND_MSG(QString("~0 [CPatternsView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            if(false == expandVisible)
            {
                collapseAll();
            }

            for(const auto& filteredEntry : filteredEntries)
            {
                setRowHidden(filteredEntry.row, filteredEntry.parentIdx, filteredEntry.filtered);

                // on empty filter we leave all tree levels collapsed
                if(true == expandVisible && false == filteredEntry.filtered)
                {
                    setExpanded(filteredEntry.parentIdx, true);
                    //SEND_MSG(QString("~1 [CPatternsView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));
                }
            }

            //SEND_MSG(QString("~2 [CPatternsView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));
        });
    }

    setModel(pModel);
}

void CPatternsView::updateColumnsVisibility()
{
    const tPatternsColumnsVisibilityMap& visibilityMap = CSettingsManager::getInstance()->getPatternsColumnsVisibilityMap();

    for( int i = static_cast<int>(ePatternsColumn::AliasTreeLevel);
         i < static_cast<int>(ePatternsColumn::Last);
         ++i)
    {
        if(i >= static_cast<int>(ePatternsColumn::AfterLastVisible))
        {
            hideColumn(i);
        }
        else
        {
            auto foundColumn = visibilityMap.find(static_cast<ePatternsColumn>(i));

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

void CPatternsView::copySelectedRow()
{
    auto selectedRows = selectionModel()->selectedRows();

    const auto& selectedIdx = selectedRows.front();

    if(false == selectedRows.isEmpty())
    {
        if(nullptr != mpModel)
        {
            mCopyPastePatternData = tCopyPastePatternData();

            QString clipboardString;
            unsigned int counter = 0u;

            auto preVisitFunc = [this, &clipboardString, &counter](const QModelIndex& idx)
            {
                tCopyPastePatternItem copyPasteItem;

                ePatternsRowType patternsRowType =
                        idx.sibling(idx.row(),
                                            static_cast<int>(ePatternsColumn::RowType)).data().value<ePatternsRowType>();

                if(ePatternsRowType::ePatternsRowType_Alias == patternsRowType)
                {
                    copyPasteItem.alias =
                            idx.sibling(idx.row(),
                                                static_cast<int>(ePatternsColumn::Alias)).data().value<QString>();

                    copyPasteItem.regex =
                            idx.sibling(idx.row(),
                                                static_cast<int>(ePatternsColumn::Regex)).data().value<QString>();

                    mCopyPastePatternData.items.push_back(copyPasteItem);

                    QString rowStr(QString("[#%1]\n").arg(counter));

                    for( int i = 0; i < static_cast<int>(ePatternsColumn::AfterLastVisible); ++i )
                    {
                        if( false == isColumnHidden(i) )
                        {
                            ePatternsColumn column = static_cast<ePatternsColumn>(i);
                            rowStr.append(getName(column)).append(" - ");

                            switch( column )
                            {
                                case ePatternsColumn::AliasTreeLevel:
                                {
                                    rowStr.append(idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Alias)).data().value<QString>());
                                }
                                    break;
                                case ePatternsColumn::Regex:
                                {
                                    rowStr.append(idx.sibling(idx.row(), i).data().value<QString>());
                                }
                                    break;
                                case ePatternsColumn::Default:
                                case ePatternsColumn::Combine:
                                {
                                    rowStr.append(idx.sibling(idx.row(), i).data().value<bool>() ? "TRUE" : "FALSE");
                                }
                                    break;
                                default:
                                    break;
                            }

                            rowStr.append("\n");
                        }
                    }

                    rowStr.append("\n");
                    clipboardString.append(rowStr);
                    ++counter;
                }

                return true;
            };

            mpModel->visit(preVisitFunc, CPatternsModel::tVisitFunction(), true, selectedIdx);

            QClipboard* pClipboard = QApplication::clipboard();

            if(nullptr != pClipboard)
            {
                pClipboard->setText(clipboardString);
            }
        }

        mCopyPastePatternData.file =
                CSettingsManager::getInstance()->getSelectedRegexFile();
    }
}

void CPatternsView::pasteSelectedRow()
{
    if(false == mCopyPastePatternData.file.isEmpty())
    {
        pastePatternTriggered(mCopyPastePatternData);
    }
}
