/**
 * @file    CPatternsView.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CPatternsView class
 */

#include "../api/CPatternsView.hpp"

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
#include <QElapsedTimer>
#include <QTimer>
#include <QAbstractScrollArea>
#include <QColorDialog>
#include <QTableWidget>
#include <QPalette>
#include <QModelIndex>
#include <QLineEdit>
#include <QDesktopServices>
#include <QClipboard>

#include "components/settings/api/ISettingsManager.hpp"
#include "CPatternsModel.hpp"
#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

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

    static bool isRefreshShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::AltModifier && pEvent->key() == Qt::Key_R;
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
            auto startTime = mTime.elapsed();

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

            auto nowToStartDiff = mTime.elapsed() - animatedObject.startTime;
            auto endToStartDiff = animatedObject.endTime - animatedObject.startTime;

            auto passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

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

        QStyleOptionViewItem viewItemOption = option;

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
            auto nowToStartDiff = mTime.elapsed() - foundAnimatedObject->startTime;
            auto endToStartDiff = foundAnimatedObject->endTime - foundAnimatedObject->startTime;

            auto passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

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
                          const int64_t& startTime_,
                          const int64_t& endTime_):
            startColor(startColor_),
            intermediateColor(intermediateColor_),
            endColor(endColor_),
            startTime(startTime_),
            endTime(endTime_)
        {}

        QColor startColor;
        QColor intermediateColor;
        QColor endColor;
        int64_t startTime;
        int64_t endTime;
    };

    typedef QMap<QModelIndex, tRowAnimationData> tRowAnimationDataMap;
    tRowAnimationDataMap mRowAnimationDataMap;
    QElapsedTimer mTime;
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
    bool bResult = true;

    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);

        if(NShortcuts::isApplyCombinationShortcut(pKeyEvent) ||
           NShortcuts::isClearSeletedPatternsShortcut(pKeyEvent) ||
           NShortcuts::isResetSeletedPatternsShortcut(pKeyEvent) ||
           NShortcuts::isCollapseAllShortcut(pKeyEvent) ||
           NShortcuts::isExpadAllShortcut(pKeyEvent) ||
           NShortcuts::isRefreshShortcut(pKeyEvent) )
        {
            keyPressEvent(pKeyEvent);
            bResult = true;
        }
        else
        {
            bResult = QObject::eventFilter(pObj, pEvent);
        }
    }
    else
    {
        bResult = QObject::eventFilter(pObj, pEvent);
    }

    return bResult;
}

QString CPatternsView::createCombinedRegex(QStringList& selectedAliases)
{
    QString finalRegex;

    bool firstInjection = true;

    typedef std::function<bool(const QModelIndex& idx)> tComparator;

    QSet<QString> usedPatterns;

    auto fillInPatterns = [this,
                           &firstInjection,
                           &finalRegex,
                           &usedPatterns,
                           &selectedAliases](const tComparator comparator, bool animate, const QColor& initialColor)
    {
        QVector<QModelIndex> highlightedRows;

        auto preVisitFunc = [this,
                &firstInjection,
                &finalRegex,
                &usedPatterns,
                &comparator,
                &animate,
                &highlightedRows,
                &selectedAliases](const QModelIndex& idx)
        {
            auto alias = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Alias)).data().value<QString>();

            if( comparator(idx) )
            {
                auto foundUsedPattern = usedPatterns.find( alias );

                if(foundUsedPattern == usedPatterns.end())
                {
                    selectedAliases.append(alias);

                    if( false == firstInjection )
                    {
                        finalRegex.append("|");
                    }

                    finalRegex.append( idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Regex)).data().value<QString>() );
                    firstInjection = false;

                    if(nullptr != mpRepresentationDelegate)
                    {
                        bool bHighlight = getSettingsManager()->getHighlightActivePatterns();

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
            QColor highlightingColor = getSettingsManager()->getPatternsHighlightingColor();
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
        QStringList selectedAliases;
        QString finalRegex = createCombinedRegex(selectedAliases);
        patternSelected( finalRegex, selectedAliases );
    }
}

void CPatternsView::keyPressEvent ( QKeyEvent * pEvent )
{
    if(NShortcuts::isApplyCombinationShortcut(pEvent))
    {
        if( true == mpModel->areAnyCombinedPatternsAvailable() )
        {
            QStringList selectedAliases;
            QString finalRegex = createCombinedRegex(selectedAliases);
            patternSelected( finalRegex, selectedAliases );
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
    else if(NShortcuts::isRefreshShortcut(pEvent))
    {
        if(nullptr != getSettingsManager()
        && nullptr != mpModel )
        {
            getSettingsManager()->refreshRegexConfiguration();
            mpModel->refreshRegexPatterns();
        }
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
            //QElapsedTimer timer;
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
    const tPatternsColumnsVisibilityMap& visibilityMap = getSettingsManager()->getPatternsColumnsVisibilityMap();

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
                        idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::RowType)).data().value<ePatternsRowType>();

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

                    const auto& copyPasteMap = getSettingsManager()->getPatternsColumnsCopyPasteMap();

                    for( auto it = copyPasteMap.begin(); it != copyPasteMap.end(); ++it )
                    {
                        bool bCopyPasteColumn = it.value();

                        if( true == bCopyPasteColumn )
                        {
                            ePatternsColumn column = it.key();
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
                                    rowStr.append(idx.sibling(idx.row(), static_cast<int>(column)).data().value<QString>());
                                }
                                    break;
                                case ePatternsColumn::Default:
                                case ePatternsColumn::Combine:
                                {
                                    rowStr.append(idx.sibling(idx.row(), static_cast<int>(column)).data().value<bool>() ? "TRUE" : "FALSE");
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
                getSettingsManager()->getSelectedRegexFile();
    }
}

void CPatternsView::pasteSelectedRow()
{
    if(false == mCopyPastePatternData.file.isEmpty())
    {
        pastePatternTriggered(mCopyPastePatternData);
    }
}

void CPatternsView::handleSettingsManagerChange()
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
                    auto aliasName = selectedRows[0].sibling(selectedRows[0].row(),
                            static_cast<int>(ePatternsColumn::Alias))
                            .data().value<QString>();
                    QStringList selectedAliases;
                    selectedAliases.append(aliasName);

                    const QString regex = selectedRows[0].data().value<QString>();

                    // we are applying selection only to those elements which have non-empty regex value
                    if(false == regex.isEmpty())
                    {
                        patternSelected( regex, selectedAliases );
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
        }

        {
            QAction* pAction = new QAction("Copy", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copySelectedRow();
            });

            if(true == selectionModel()->selectedRows().isEmpty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Paste", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+V")));
            connect(pAction, &QAction::triggered, [this]()
            {
                pasteSelectedRow();
            });

            // if there is something to paste
            if(true == mCopyPastePatternData.items.empty())
            {
                pAction->setEnabled(false);
            }

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

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Refresh", this);
            pAction->setShortcut(QKeySequence(tr("Alt+R")));
            connect(pAction, &QAction::triggered, [this]()
            {
                if(nullptr != getSettingsManager()
                && nullptr != mpModel )
                {
                    getSettingsManager()->refreshRegexConfiguration();
                    mpModel->refreshRegexPatterns();
                }
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Combination settings", this);

            {
                if(true == mpModel->areAnyCombinedPatternsAvailable())
                {
                    QAction* pAction = new QAction("Run combination", this);
                    pAction->setShortcut(QKeySequence(Qt::Key_Enter));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        QStringList selectedAliases;
                        QString finalRegex = createCombinedRegex(selectedAliases);
                        patternSelected( finalRegex, selectedAliases );
                    });
                    pSubMenu->addAction(pAction);
                }
            }

            {
                auto selectedRows = selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Regex));
                if(false == selectedRows.empty())
                {
                    QAction* pAction = new QAction("Run selected item ( double click )", this);
                    connect(pAction, &QAction::triggered, [this, selectedRows]()
                    {
                        auto aliasName = selectedRows[0].sibling(selectedRows[0].row(),
                                static_cast<int>(ePatternsColumn::Alias))
                                .data().value<QString>();
                        QStringList selectedAliases;
                        selectedAliases.append(aliasName);

                        patternSelected( selectedRows[0].data().value<QString>(), selectedAliases );
                    });
                    pSubMenu->addAction(pAction);
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
                pSubMenu->addAction(pAction);
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
                pSubMenu->addAction(pAction);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Columns settings", this);

            {
                QMenu* pSubSubMenu = new QMenu("Visible columns", this);

                {
                    const auto& patternsColumnsVisibilityMap =
                            getSettingsManager()->getPatternsColumnsVisibilityMap();

                    for( int i = static_cast<int>(ePatternsColumn::AliasTreeLevel);
                         i < static_cast<int>(ePatternsColumn::AfterLastVisible);
                         ++i)
                    {
                        auto foundItem = patternsColumnsVisibilityMap.find(static_cast<ePatternsColumn>(i));

                        if(foundItem != patternsColumnsVisibilityMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<ePatternsColumn>(i)), this);
                            connect(pAction, &QAction::triggered, [this, i](bool checked)
                            {
                                auto patternsColumnsVisibilityMap_ =
                                        getSettingsManager()->getPatternsColumnsVisibilityMap();

                                auto foundItem_ = patternsColumnsVisibilityMap_.find(static_cast<ePatternsColumn>(i));

                                if(foundItem_ != patternsColumnsVisibilityMap_.end()) // if item is in the map
                                {
                                    foundItem_.value() = checked; // let's update visibility value
                                    getSettingsManager()->setPatternsColumnsVisibilityMap(patternsColumnsVisibilityMap_);
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
                    getSettingsManager()->resetPatternsColumnsVisibilityMap();
                });
                pSubMenu->addAction(pAction);
            }

            pSubMenu->addSeparator();

            {
                QMenu* pSubSubMenu = new QMenu("Copy columns", this);

                {
                    const auto& patternsColumnsCopyPasteMap =
                            getSettingsManager()->getPatternsColumnsCopyPasteMap();

                    for( int i = static_cast<int>(ePatternsColumn::AliasTreeLevel);
                         i < static_cast<int>(ePatternsColumn::AfterLastVisible);
                         ++i)
                    {
                        auto foundItem = patternsColumnsCopyPasteMap.find(static_cast<ePatternsColumn>(i));

                        if(foundItem != patternsColumnsCopyPasteMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<ePatternsColumn>(i)), this);
                            connect(pAction, &QAction::triggered, [this, i](bool checked)
                            {
                                auto patternsColumnsCopyPasteMap_ =
                                        getSettingsManager()->getPatternsColumnsCopyPasteMap();

                                auto foundItem_ = patternsColumnsCopyPasteMap_.find(static_cast<ePatternsColumn>(i));

                                if(foundItem_ != patternsColumnsCopyPasteMap_.end()) // if item is in the map
                                {
                                    foundItem_.value() = checked; // let's update copy paste value
                                    getSettingsManager()->setPatternsColumnsCopyPasteMap(patternsColumnsCopyPasteMap_);
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
                    getSettingsManager()->resetPatternsColumnsCopyPasteMap();
                });
                pSubMenu->addAction(pAction);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Highlighting settings", this);

            {
                QAction* pAction = new QAction("Highlight \"combined\" patterns", this);
                connect(pAction, &QAction::triggered, [this](bool checked)
                {
                    getSettingsManager()->setHighlightActivePatterns(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(getSettingsManager()->getHighlightActivePatterns());
                pSubMenu->addAction(pAction);
            }

            {
                if(true == getSettingsManager()->getHighlightActivePatterns())
                {
                    QAction* pAction = new QAction("Highlighting color ...", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        QColor color = QColorDialog::getColor( getSettingsManager()->getPatternsHighlightingColor(), this );
                        if( color.isValid() )
                        {
                            getSettingsManager()->setPatternsHighlightingColor(color);
                        }
                    });
                    pSubMenu->addAction(pAction);
                }
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Open regex patterns storage", this);
            connect(pAction, &QAction::triggered, [this]()
            {
                SEND_MSG(QString("[CPatternsView]: Attempt to open path - \"%1\"")
                         .arg(getSettingsManager()->getRegexDirectory()));

                QDesktopServices::openUrl( QUrl::fromLocalFile( getSettingsManager()->getRegexDirectory() ) );
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Minimize on selection", this);
            connect(pAction, &QAction::triggered, [this](bool checked)
            {
                getSettingsManager()->setMinimizePatternsViewOnSelection(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getMinimizePatternsViewOnSelection());
            contextMenu.addAction(pAction);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect( getSettingsManager().get(),
             &ISettingsManager::patternsColumnsVisibilityMapChanged,
             this, [this](const tPatternsColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
    });

    setItemDelegate(mpRepresentationDelegate);
}

PUML_PACKAGE_BEGIN(DMA_PatternsView_API)
    PUML_CLASS_BEGIN_CHECKED(CPatternsView)
        PUML_INHERITANCE_CHECKED(QTreeView, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTreeRepresentationDelegate, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CPatternsModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QLineEdit, 1, 1, patterns search input)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN_CHECKED(CTreeRepresentationDelegate)
        PUML_INHERITANCE_CHECKED(QStyledItemDelegate, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
