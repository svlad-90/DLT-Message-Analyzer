/**
 * @file    CSearchResultView.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSearchResultView class
 */

#include "../api/CSearchResultView.hpp"

#include "QHeaderView"
#include "QScrollBar"
#include "QApplication"
#include "QClipboard"
#include "QMimeData"
#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QColorDialog>
#include "QDebug"
#include <QInputDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QTableWidget>
#include <QActionGroup>

#include "common/Definitions.hpp"
#include "common/TOptional.hpp"
#include "CSearchResultHighlightingDelegate.hpp"
#include "CSearchResultModel.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/logsWrapper/api/IFileWrapper.hpp"
#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

CSearchResultView::CSearchResultView(QWidget *parent):
    tParent(parent),
    mbIsVerticalScrollBarVisible(false),
    mbIsViewFull(false),
    mbUserManuallyAdjustedLastVisibleColumnWidth(false),
    mpFile(nullptr),
    mSearchRange(),
    mpSpecificModel(nullptr),
    mContentSizeMap(),
    mpCoverageNoteProvider(nullptr),
    mpMainTabWidget(nullptr),
    mpMainTableView(nullptr)
{
    connect(this, &QTableView::clicked, [this](const QModelIndex &index)
    {
        if(false == index.isValid())
        {
            return;
        }

        if(static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::UML_Applicability)
        {
            if(nullptr != mpSpecificModel)
            {
                auto siblingIdx = index.sibling(index.row(), static_cast<int>(eSearchResultColumn::UML_Applicability));
                mpSpecificModel->setUML_Applicability(index, !siblingIdx.data(Qt::CheckStateRole).value<bool>());
            }
        }
        else if(static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::PlotView_Applicability)
        {
            if(nullptr != mpSpecificModel)
            {
                auto siblingIdx = index.sibling(index.row(), static_cast<int>(eSearchResultColumn::PlotView_Applicability));
                mpSpecificModel->setPlotView_Applicability(index, !siblingIdx.data(Qt::CheckStateRole).value<bool>());
            }
        }
    });
}

void CSearchResultView::newSearchStarted(const QString& regex)
{
    mUsedRegex = regex;
    mContentSizeMap.clear();
    mbIsViewFull = false;
    mbUserManuallyAdjustedLastVisibleColumnWidth = false;
}

void CSearchResultView::setMainTableView(QTableView* pMainTableView)
{
    mpMainTableView = pMainTableView;
}

void CSearchResultView::getUserSearchRange()
{
    auto minOverallVal = 0;
    auto maxOverallVal = mpFile->sizeNonFiltered() != 0 ? mpFile->sizeNonFiltered() - 1 : 0;
    auto minCurrentVal = ( mSearchRange.isSet ? mSearchRange.from : minOverallVal );
    auto maxCurrentVal = ( mSearchRange.isSet ? mSearchRange.to: maxOverallVal );

    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel("Please, specify search range:"));

    QVector<QLineEdit *> fields;

    QString labelFrom = QString("From ( min - %1 ):").arg(minOverallVal);
    QLineEdit *lineEditFrom = new QLineEdit(&dialog);
    lineEditFrom->setText(QString::number(minCurrentVal));

    form.addRow(labelFrom, lineEditFrom);
    fields.push_back(lineEditFrom);

    QString labelTo = QString("To ( max - %1 ):").arg(maxOverallVal);
    QLineEdit *lineEditTo = new QLineEdit(&dialog);
    lineEditTo->setText(QString::number(maxCurrentVal));
    form.addRow(labelTo, lineEditTo);
    fields.push_back(lineEditTo);

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);

    auto acceptHandler = [&dialog,
            &lineEditFrom,
            &lineEditTo,
            &minOverallVal,
            &maxOverallVal,
            &minCurrentVal,
            &maxCurrentVal]()
    {
        bool bIsFromADigit = false;
        int from = lineEditFrom->text().toInt( &bIsFromADigit );
        bool bIsToADigit = false;
        int to = lineEditTo->text().toInt( &bIsToADigit );

        bool bIsFromInMinRange = from >= minOverallVal;
        bool bIsFromInMaxRange = from <= maxOverallVal;

        bool bIsToInMinRange = to >= minOverallVal;
        bool bIsToInMaxRange = to <= maxOverallVal;

        bool bIsFromLessOrEqualThanTo = from <= to;

        bool areAllValuesValid = bIsFromADigit &&
                                 bIsToADigit &&
                                 bIsFromInMinRange &&
                                 bIsFromInMaxRange &&
                                 bIsToInMinRange &&
                                 bIsToInMaxRange &&
                                 bIsFromLessOrEqualThanTo;

        if(true == areAllValuesValid)
        {
            dialog.accept();
        }
        else
        {
            if(!bIsFromADigit ||
            !bIsToADigit ||
            !bIsFromInMinRange ||
            !bIsFromInMaxRange ||
            !bIsToInMinRange ||
            !bIsToInMaxRange )
            {
                if(false == bIsFromADigit)
                {
                    lineEditFrom->setText( QString::number(minCurrentVal) );
                }
                else if(false == bIsFromInMinRange)
                {
                    lineEditFrom->setText( QString::number(minOverallVal) );
                }
                else if(false == bIsFromInMaxRange)
                {
                    lineEditFrom->setText( QString::number(maxCurrentVal) );
                }

                if(false == bIsToADigit)
                {
                    lineEditTo->setText( QString::number(maxCurrentVal) );
                }
                else if(false == bIsToInMinRange)
                {
                    lineEditTo->setText( QString::number(minOverallVal) );
                }
                else if(false == bIsToInMaxRange)
                {
                    lineEditTo->setText( QString::number(maxCurrentVal) );
                }
            }
            else if(false == bIsFromLessOrEqualThanTo)
            {
                lineEditFrom->setText( QString::number(to) );
            }
        }
    };

    auto rejectHandler = [&dialog]()
    {
        dialog.reject();
    };

    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, acceptHandler);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, rejectHandler);

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted)
    {
        bool isFromADigit = false;
        int from = fields[0]->text().toInt( &isFromADigit );
        bool isToADigit = false;
        int to = fields[1]->text().toInt( &isToADigit );

        // validation done in acceptHandler, so here no need to duplicate it
        tIntRangeProperty prop;
        prop.isSet = true;
        prop.from = from;
        prop.to = ( to >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : to;
        prop = mpFile->normalizeSearchRange( prop );
        mSearchRange = prop;
        searchRangeChanged( mSearchRange, false );
    }
}

void CSearchResultView::setFile( const tFileWrapperPtr& pFile )
{
    mSearchRange = tIntRangeProperty();
    searchRangeChanged( mSearchRange, true );

    mpFile = pFile;
}

void CSearchResultView::forceUpdateWidthAndResetContentMap()
{
    mContentSizeMap.clear();
    mbUserManuallyAdjustedLastVisibleColumnWidth = false;
    updateWidth(true);
}

eSearchResultColumn CSearchResultView::getLastVisibleColumn() const
{
    eSearchResultColumn result = eSearchResultColumn::Last;

    for(int i = 0; i < static_cast<int>(eSearchResultColumn::Last); ++i)
    {
        if(false == isColumnHidden(i))
        {
            result = static_cast<eSearchResultColumn>(i);
        }
    }

    return result;
}

void CSearchResultView::updateWidth(bool force, tUpdateWidthSet updateWidthSet)
{
    if(true == force)
    {
        for(int i = 0; i < static_cast<int>(eSearchResultColumn::Last); ++i)
        {
            updateWidthSet.insert(static_cast<eSearchResultColumn>(i));
        }
    }

    auto lastVisibleColumn = getLastVisibleColumn();

    auto updateAllColumnsExceptLastVisibleOne = [this, &updateWidthSet, &lastVisibleColumn]()->int
    {
        int widthWithoutLastVisible = 0;

        for(int i = 0; i < static_cast<int>(lastVisibleColumn); ++i)
        {
            if(false == isColumnHidden(i))
            {
                auto itemFound = updateWidthSet.find(static_cast<eSearchResultColumn>(i));

                if(itemFound != updateWidthSet.end())
                {
                    resizeColumnToContents(i);
                }

                widthWithoutLastVisible += columnWidth(i);
            }
        }

        return widthWithoutLastVisible;
    };

    auto setLastVisibleColumnWidth = [this, &lastVisibleColumn](const int& widthWithoutLastVisible)
    {
        auto payloadWidth = viewport()->size().width() - widthWithoutLastVisible;
        setColumnWidth( static_cast<int>(lastVisibleColumn), payloadWidth );
    };

    auto widthWithoutLastVisible = updateAllColumnsExceptLastVisibleOne();

    const auto searchViewLastColumnWidthStrategy = static_cast<eSearchViewLastColumnWidthStrategy>(getSettingsManager()->getSearchViewLastColumnWidthStrategy());

    switch(searchViewLastColumnWidthStrategy)
    {
        case eSearchViewLastColumnWidthStrategy::eReset:
        {
            if(false == mbUserManuallyAdjustedLastVisibleColumnWidth)
            {
                setLastVisibleColumnWidth(widthWithoutLastVisible);
            }
        }
            break;
        case eSearchViewLastColumnWidthStrategy::eFitToContent:
        {
            auto foundColumn = updateWidthSet.find(lastVisibleColumn);

            if(foundColumn != updateWidthSet.end())
            {
                resizeColumnToContents(static_cast<int>(lastVisibleColumn));
            }
        }
        break;
        case eSearchViewLastColumnWidthStrategy::ePreserveUserWidth:
        case eSearchViewLastColumnWidthStrategy::eLast:
            //do nothing
            break;
    }
}

void CSearchResultView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    tParent::dataChanged(topLeft, bottomRight, roles);

    auto pVerticalScrollBar = verticalScrollBar();

    bool continueCheck = true;

    if(false == mbIsViewFull)
    {
        if(false == indexAt(rect().bottomLeft()).isValid())
        {
            continueCheck = false;
        }
        else
        {
            updateWidthLogic(topLeft.row(), bottomRight.row());
            mbIsViewFull = true;
        }
    }

    if(true == continueCheck && nullptr != pVerticalScrollBar)
    {
        auto isVerticalScrollBarVisible_ = pVerticalScrollBar->isVisible();

        if(mbIsVerticalScrollBarVisible != isVerticalScrollBarVisible_)
        {
            updateWidth(true);
            mbIsVerticalScrollBarVisible = isVerticalScrollBarVisible_;
        }
    }

    if(hasAutoScroll())
    {
        scrollToBottom();
    }
}

void CSearchResultView::currentChanged(const QModelIndex &current,
                      const QModelIndex &previous)
{
    tParent::currentChanged(current, previous);

    if(true == current.isValid())
    {
        updateWidthLogic(current.row(), current.row());
    }
}

void CSearchResultView::verticalScrollbarAction(int action)
{
    tParent::verticalScrollbarAction(action);

    // we will check up to 100 elements above the current and up to 100 elements after the current

    auto indexFrom = indexAt(rect().topLeft());
    auto rangeFrom = indexFrom.row();
    auto indexTo = indexAt(rect().bottomLeft());
    auto rangeTo = indexTo.row();

    if(true == indexFrom.isValid() && true == indexTo.isValid())
    {
        updateWidthLogic(rangeFrom, rangeTo);
    }
}

void CSearchResultView::updateWidthLogic(const int& rowFrom, const int& rowTo)
{
    bool bShouldUpdateWidth = false;

    tUpdateWidthSet updateWidthSet;

    for(int iColumn = 0; iColumn < static_cast<int>(eSearchResultColumn::Last); ++iColumn)
    {
        if(false == isColumnHidden(iColumn))
        {
            for(int iRow = rowFrom; iRow <= rowTo; ++iRow)
            {
                if(nullptr != mpSpecificModel)
                {
                    auto searchColumn = static_cast<eSearchResultColumn>(iColumn);
                    auto strValue = mpSpecificModel->getStrValue(iRow, searchColumn);

                    auto dataSize = strValue.size();

                    if(mContentSizeMap[searchColumn] < dataSize)
                    {
                        updateWidthSet.insert(searchColumn);
                        mContentSizeMap[searchColumn] = dataSize;
                        bShouldUpdateWidth = true;
                    }
                }
            }
        }
    }

    if(true == bShouldUpdateWidth)
    {
        updateWidth(false, updateWidthSet);
    }
}

void CSearchResultView::updateColumnsVisibility()
{
    const tSearchResultColumnsVisibilityMap& visibilityMap = getSettingsManager()->getSearchResultColumnsVisibilityMap();

    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
         i < static_cast<int>(eSearchResultColumn::Last);
         ++i)
    {
        auto foundColumn = visibilityMap.find(static_cast<eSearchResultColumn>(i));

        if( foundColumn != visibilityMap.end() )
        {
            if(true == foundColumn.value()) // if column is visible
            {
                if( ( i != static_cast<int>(eSearchResultColumn::UML_Applicability) && i != static_cast<int>(eSearchResultColumn::PlotView_Applicability) ) ||
                  ((i == static_cast<int>(eSearchResultColumn::UML_Applicability) && (true == getSettingsManager()->getUML_FeatureActive() ) ) ) ||
                  ((i == static_cast<int>(eSearchResultColumn::PlotView_Applicability) && (true == getSettingsManager()->getPlotViewFeatureActive() ) ) ) )
                {
                    showColumn(i);

                    if(static_cast<eSearchResultColumn>(i) == eSearchResultColumn::Payload)
                    {
                        setColumnWidth(i, 1000);
                    }
                    else
                    {
                        setColumnWidth(i, 50);
                    }
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
        else
        {
             hideColumn(static_cast<int>(i));
        }
    }

    forceUpdateWidthAndResetContentMap();
}

void CSearchResultView::setModel(QAbstractItemModel *model)
{
    tParent::setModel(model);

    if(nullptr != model)
    {
       mpSpecificModel = static_cast<CSearchResultModel*>(model);
    }

    updateColumnsVisibility();

    QHeaderView *verticalHeader = tParent::verticalHeader();

    {
        //restore font from persistency
        const auto& usedFont = getSettingsManager()->getFont_SearchView();
        setFont(usedFont);
        verticalHeader->resizeSections(QHeaderView::Fixed);
        QFontMetrics fontMetrics( usedFont );
        verticalHeader->setDefaultSectionSize(fontMetrics.height());
    }

    QHeaderView *horizontalHeader = tParent::horizontalHeader();

    connect( horizontalHeader, &QHeaderView::sectionDoubleClicked, this, [this](int logicalIndex)
    {
        if(static_cast<eSearchResultColumn>(logicalIndex) == getLastVisibleColumn())
        {
            mbUserManuallyAdjustedLastVisibleColumnWidth = true;
        }

        resizeColumnToContents(logicalIndex);
    });
}

bool CSearchResultView::isVerticalScrollBarVisible() const
{   
    if(verticalScrollBarPolicy() == Qt::ScrollBarPolicy::ScrollBarAlwaysOff)
        return false;

    bool IsVisible = false;

    int HeightOfRows = 0;
    int HeaderHeight = horizontalHeader()->height();
    int TableHeight = height();

    for (int i = 0; i < model()->rowCount(); i++)
    {
        HeightOfRows += rowHeight(i);

        if( (HeightOfRows + HeaderHeight) > TableHeight)
        {
            IsVisible = true;
            break;
        }
    }

    return IsVisible;
}

QString CSearchResultView::getMainTableSelectionAsString() const
{
    QString result;

    if(mpMainTableView)
    {
        auto* pSelectionModel = mpMainTableView->selectionModel();
        auto* pModel = mpMainTableView->model();
        auto* pHeader = mpMainTableView->horizontalHeader();

        if(pSelectionModel && pModel && pHeader)
        {
            auto selectedRows = pSelectionModel->selectedRows();

            QMap<int, QModelIndex> sortedSelectedRows;

            for(const auto& index : selectedRows)
            {
                sortedSelectedRows.insert(index.row(), index);
            }

            auto columnsNumber = pModel->columnCount();

            for(const auto& index : sortedSelectedRows)
            {
                for(int columId = 0; columId < columnsNumber; ++columId )
                {
                    if(!pHeader->isSectionHidden(columId))
                    {
                        auto columnIndex = index.sibling(index.row(), columId);

                        result += columnIndex.data().toString().toHtmlEscaped();

                        if(columId < columnsNumber - 1)
                        {
                            result += " ";
                        }
                        else
                        {
                            result += "<br/>";
                        }
                    }
                }
            }
        }
    }

    return result;
}

QString CSearchResultView::getSelectionAsString( bool copyAsHTML, bool copyOnlyPayload ) const
{
    QString result;

    const QColor nonHighlightedColor(0,0,0);

    auto selectedRows = selectionModel()->selectedRows();

    QMap<int, QModelIndex> sortedSelectedRows;

    for(const auto& index : selectedRows)
    {
        sortedSelectedRows.insert(index.row(), index);
    }

    const int NOT_FOUND_COLUMN_IDX = -1;
    int payloadColumnIdx = NOT_FOUND_COLUMN_IDX;

    QVector<int> copyPasteColumns;
    copyPasteColumns.reserve((static_cast<int>(eSearchResultColumn::Last)));

    {
        const auto& copyPasteColumnsMap = getSettingsManager()->getSearchResultColumnsCopyPasteMap();

        int columnsCounter = 0;
        int copyPasteColumnsCounter = 0;

        for( const auto& copyPaste : copyPasteColumnsMap )
        {
            if(true == copyPaste)
            {
                copyPasteColumns.push_back(columnsCounter);

                if(columnsCounter == static_cast<int>(eSearchResultColumn::Payload))
                {
                    payloadColumnIdx = copyPasteColumnsCounter;
                }

                ++copyPasteColumnsCounter;
            }

            ++columnsCounter;
        }
    }

    typedef QPair<QString /*usual*/, QString /*rich*/> tCopyItem;
    QVector<tCopyItem> copyItems;
    int finalRichStringSize = 0;
    int finalStringSize = 0;

    int copyPasteColumnsSize = copyPasteColumns.size();

    for(const auto& index : sortedSelectedRows)
    {
        auto getHighlightedColor = [this](const tHighlightingRange& range)
        {
            QColor result;

            bool isMonoColorHighlighting = getSettingsManager()->getSearchResultMonoColorHighlighting();
            bool isExplicitColor = range.explicitColor;

            if(true == isExplicitColor)
            {
                result = range.color_code;
            }
            else
            {
                if(true == isMonoColorHighlighting)
                {
                    result = getSettingsManager()->getRegexMonoHighlightingColor();
                }
                else
                {
                    result = range.color_code;
                }
            }

            return result;
        };

        int i = 0;

        if( (true == copyOnlyPayload) && (NOT_FOUND_COLUMN_IDX != payloadColumnIdx) )
        {
            i = payloadColumnIdx;
            copyPasteColumnsSize = payloadColumnIdx + 1;
        }

        for( ; i < copyPasteColumnsSize; ++i )
        {
            auto columnId = copyPasteColumns[i];
            auto column = index.sibling(index.row(), columnId);
            eSearchResultColumn field = static_cast<eSearchResultColumn>(columnId);
            QString columnStr = column.data().value<QString>();

            auto attachText = [this,
                    &copyItems,
                    &finalRichStringSize,
                    &finalStringSize,
                    &columnStr,
                    &i,
                    &copyPasteColumnsSize,
                    &field]
                    (const tIntRange& range,
                    const QColor& color,
                    bool isHighlighted)
            {
                bool isHighlightedExtended = ( isHighlighted ||
                                               ( eSearchResultColumn::Timestamp == field
                                                 && true == getSettingsManager()->getMarkTimeStampWithBold() ) );

                QString subStr = columnStr.mid( range.from, range.to - range.from + 1 );

                QString str;
                str.reserve(100 + subStr.size());

                str.append("<font style=\"color:rgb(")
                        .append(QString::number(color.red()))
                        .append(",")
                        .append(QString::number(color.green()))
                        .append(",")
                        .append(QString::number(color.blue()))
                        .append(");\">");

                if(true == isHighlightedExtended)
                {
                    str.append("<b>");
                }

                str.append(subStr.toHtmlEscaped());

                if(true == isHighlightedExtended)
                {
                    str.append("</b>");
                }

                str.append("</font>");

                if(i < copyPasteColumnsSize - 1 &&
                   range.to == columnStr.size() - 1 )
                {
                    str.append(" ");
                    subStr.append(" ");
                }

                copyItems.push_back(tCopyItem(subStr, str));

                finalRichStringSize += str.size();
                finalStringSize += columnStr.size();
            };

            const auto* pSpecificModel = dynamic_cast<CSearchResultModel*>(model());

            if(nullptr != pSpecificModel)
            {
                const auto& matchesItemPack = pSpecificModel->getFoundMatchesItemPack( index );

                const auto& highlightingInfoMultiColor = matchesItemPack.getItemMetadata().highlightingInfoMultiColor;
                auto foundHighlightingItem = highlightingInfoMultiColor.find(field);

                if(highlightingInfoMultiColor.end() != foundHighlightingItem)
                {
                    const auto& fieldRanges = matchesItemPack.getItemMetadata().fieldRanges;
                    const auto& foundfieldRange = fieldRanges.find(field);

                    if(fieldRanges.end() != foundfieldRange)
                    {
                        int counter = 0;

                        for(auto it = foundHighlightingItem->begin(); it != foundHighlightingItem->end(); ++it)
                        {
                            const auto& range = *it;

                            if(0 == counter)
                            {
                                if(0 != range.from)
                                {
                                    attachText( tIntRange(0, range.from - 1 ), nonHighlightedColor, false );
                                }

                                attachText( tIntRange( range.from, range.to ), getHighlightedColor(*it), true );
                            }
                            else if(0 < counter)
                            {
                                auto itPrev = it;
                                --itPrev;
                                const auto& prevRange = *(itPrev);

                                if(prevRange.to < range.from)
                                {
                                    attachText( tIntRange(prevRange.to + 1, range.from - 1), nonHighlightedColor, false );
                                }

                                attachText( tIntRange( range.from, range.to ), getHighlightedColor(*it), true );
                            }

                            if(counter == static_cast<int>(foundHighlightingItem->size() - 1)) // last element
                            {
                                if( range.to < columnStr.size() - 1 )
                                {
                                    attachText( tIntRange(range.to + 1, columnStr.size() - 1), nonHighlightedColor, false );
                                }
                            }

                            ++counter;
                        }
                    }
                    else
                    {
                        attachText( tIntRange(0, columnStr.size() - 1), nonHighlightedColor, false );
                    }
                }
                else
                {
                    attachText( tIntRange(0, columnStr.size() - 1), nonHighlightedColor, false );
                }
            }
            else
            {
                attachText( tIntRange(0, columnStr.size() - 1), nonHighlightedColor, false );
            }
        }

        static const QString newLine("<br/>");
        copyItems.push_back(tCopyItem("\n",newLine));
        finalStringSize+=newLine.size();
    }

    QString finalRichText;
    finalRichText.reserve(finalRichStringSize);

    QString finalText;
    finalText.reserve(finalStringSize);

    for(const auto& copyItem : copyItems)
    {
        finalText.append(copyItem.first);
        finalRichText.append(copyItem.second);
    }

    if(copyAsHTML)
    {
        result = finalRichText;
    }
    else
    {
        result = finalText;
    }

    return result;
}

void CSearchResultView::copySelectionToClipboard( bool copyAsHTML, bool copyOnlyPayload ) const
{
    QClipboard *pClipboard = QApplication::clipboard();
    QMimeData *rich_text = new QMimeData();
    rich_text->setText(getSelectionAsString(copyAsHTML, copyOnlyPayload));
    pClipboard->setMimeData(rich_text);
}

void CSearchResultView::keyPressEvent ( QKeyEvent * event )
{
    if(event->matches(QKeySequence::Copy))
    {
        if(hasFocus())
        {
            copySelectionToClipboard( getSettingsManager()->getCopySearchResultAsHTML(), false );
        }
    }
    else if( ((event->modifiers() & Qt::ShiftModifier) != 0)
             && ((event->modifiers() & Qt::ControlModifier) != 0)
             && (event->key() == Qt::Key::Key_C))
    {
        if(hasFocus())
        {
            copySelectionToClipboard( getSettingsManager()->getCopySearchResultAsHTML(), true );
        }
    }
    else if(((event->modifiers() & Qt::ControlModifier) != 0
            && (event->modifiers() & Qt::ShiftModifier) != 0)
            && (event->key() == Qt::Key::Key_Space))
    {
        clearSearchResultsRequested();
    }
    else if(((event->modifiers() & Qt::ControlModifier) != 0
             && (event->modifiers() & Qt::AltModifier) != 0)
             && (event->key() == Qt::Key::Key_C))
    {
        copyMessageFiles();
    }
    else if(event->key() == Qt::Key::Key_Space)
    {
        if(nullptr != mpSpecificModel)
        {
            auto selectedRows = selectionModel()->selectedRows();

            if(false == selectedRows.empty())
            {
                TOptional<bool> bSelectedValue;
                auto tryApplyActionToColumn = [&selectedRows, &bSelectedValue, this](eSearchResultColumn targetColumn)
                {
                    const auto targetColumnInt = static_cast<int>(targetColumn);
                    auto targetColumnIdx = selectedRows[0].sibling(selectedRows[0].row(), targetColumnInt);
                    bool bTargetIndexFound = targetColumnIdx.flags() & Qt::ItemIsEditable;

                    if(false == bTargetIndexFound)
                    {
                        for(const auto& selectedRow : selectedRows)
                        {
                            targetColumnIdx = selectedRow.sibling(selectedRow.row(), targetColumnInt);

                            if(targetColumnIdx.flags() & Qt::ItemIsEditable)
                            {
                                bTargetIndexFound = true;
                                break;
                            }
                        }
                    }

                    if(true == bTargetIndexFound)
                    {
                        auto targetValue = false;

                        if(false == bSelectedValue.isSet())
                        {
                            targetValue = !(targetColumnIdx.data(Qt::CheckStateRole).value<bool>());
                            bSelectedValue.setValue(targetValue);
                        }
                        else
                        {
                            targetValue = bSelectedValue.getValue();
                        }

                        for(const auto& selectedRow : selectedRows)
                        {
                            auto siblingIdx = selectedRow.sibling(selectedRow.row(), targetColumnInt);

                            if(siblingIdx.data(Qt::CheckStateRole).value<bool>() != targetValue)
                            {
                                if(targetColumn == eSearchResultColumn::UML_Applicability)
                                {
                                    mpSpecificModel->setUML_Applicability(siblingIdx, targetValue);
                                }
                                else if(targetColumn == eSearchResultColumn::PlotView_Applicability)
                                {
                                    mpSpecificModel->setPlotView_Applicability(siblingIdx, targetValue);
                                }
                            }
                        }
                    }
                };

                if(true == getSettingsManager()->getUML_FeatureActive())
                {
                    tryApplyActionToColumn(eSearchResultColumn::UML_Applicability);
                }

                if(true == getSettingsManager()->getPlotViewFeatureActive())
                {
                    tryApplyActionToColumn(eSearchResultColumn::PlotView_Applicability);
                }
            }
        }
    }
    else if((event->modifiers() & Qt::AltModifier) != 0
            && (event->key() == Qt::Key::Key_Down))
    {
        switchToNextCheckboxItem(true, static_cast<int>(eSearchResultColumn::UML_Applicability));
    }
    else if((event->modifiers() & Qt::AltModifier) != 0
            && (event->key() == Qt::Key::Key_Up))
    {
        switchToNextCheckboxItem(false, static_cast<int>(eSearchResultColumn::UML_Applicability));
    }
    else if((event->modifiers() & Qt::ControlModifier) != 0
            && (event->key() == Qt::Key::Key_Down))
    {
        jumpToGroupedViewHighlightingMessage(eDirection::Next);
    }
    else if((event->modifiers() & Qt::ControlModifier) != 0
            && (event->key() == Qt::Key::Key_Up))
    {
        jumpToGroupedViewHighlightingMessage(eDirection::Previous);
    }
    else if(event->key() == Qt::Key::Key_Escape)
    {
        clearGroupedViewHighlighting();
    }
    else if((event->modifiers() & Qt::ControlModifier) != 0 &&
            (event->modifiers() & Qt::AltModifier) != 0 &&
            (event->key() == Qt::Key::Key_A))
    {
        if(selectionModel() &&
           !selectionModel()->selectedRows().empty())
        {
            addComment();
        }
    }
    else if((event->modifiers() & Qt::ControlModifier) != 0 &&
            (event->modifiers() & Qt::AltModifier) != 0 &&
            (event->key() == Qt::Key::Key_M))
    {
        if(mpMainTableView &&
           mpMainTableView->selectionModel() &&
           !mpMainTableView->selectionModel()->selectedRows().empty())
        {
            addCommentFromMainTable();
        }
    }
    else
    {
       tParent::keyPressEvent(event);
    }
}

void CSearchResultView::copyMessageFiles()
{
    {
        if(nullptr != mpFile && true == mpFile->getSubFilesHandlingStatus())
        {
            const auto* pSelectionModel = selectionModel();
            auto selectedRows = pSelectionModel->selectedRows();
            const int selectedRowsSize = selectedRows.size();

            auto minSelectedRow = std::min_element( selectedRows.begin(), selectedRows.end(),
                                                    [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });
            auto maxSelectedRow = std::max_element( selectedRows.begin(), selectedRows.end(),
                                                    [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });

            if(selectedRowsSize >= 2)
            {
                tMsgId from = minSelectedRow->sibling(minSelectedRow->row(), static_cast<int>(eSearchResultColumn::Index)).data().value<tMsgId>();
                tMsgId to = maxSelectedRow->sibling(maxSelectedRow->row(), static_cast<int>(eSearchResultColumn::Index)).data().value<tMsgId>();

                tIntRangeProperty prop;
                prop.isSet = true;
                prop.from = from;
                prop.to = ( to >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : to;
                prop = mpFile->normalizeSearchRange( prop );

                mpFile->copyFileNamesToClipboard(tIntRange(prop.from, prop.to));
            }
            else if(selectedRowsSize == 1)
            {
                auto selectedRowModelIdx = selectedRows.front();
                auto selectedRowIdx = selectedRowModelIdx.sibling(selectedRowModelIdx.row(), static_cast<int>(eSearchResultColumn::Index)).data().value<int>();

                mpFile->copyFileNameToClipboard(selectedRowIdx);
            }
        }
    }
}

void CSearchResultView::switchToNextCheckboxItem(bool bNext, int column)
{
    auto selectedRows = selectionModel()->selectedRows();

    if(false == selectedRows.isEmpty())
    {
        QModelIndex targetIndex;

        if(true == bNext)
        {
            targetIndex = *std::min_element( selectedRows.begin(), selectedRows.end(),
                                            [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });
        }
        else
        {
            targetIndex = *std::max_element( selectedRows.begin(), selectedRows.end(),
                                            [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });
        }

        if(true == targetIndex.isValid())
        {
            auto iSize = model()->rowCount();

            auto checkIndexFunc = [this, &column](const int& row)->bool
            {
                bool bResult = false;

                auto checkIndex = model()->index(row, column);
                if( true == checkIndex.isValid() )
                {
                    bool bUsedForUML = checkIndex.flags() & Qt::ItemIsEditable;

                    if(true == bUsedForUML)
                    {
                        scrollTo( checkIndex, QAbstractItemView::ScrollHint::PositionAtCenter );
                        setCurrentIndex(checkIndex);
                        bResult = true;
                    }
                }

                return bResult;
            };

            if(true == bNext)
            {
                if(targetIndex.row() < model()->rowCount())
                {
                    for(int i = targetIndex.row() + 1; i < iSize; ++i)
                    {
                        if(true == checkIndexFunc(i))
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                if(targetIndex.row() > 0)
                {
                    for(int i = targetIndex.row() - 1; i >= 0; --i)
                    {
                        if(true == checkIndexFunc(i))
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}

void CSearchResultView::selectAllCheckboxItems(bool select, int column)
{
    const auto iSize = model()->rowCount();
    for( int i = 0; i < iSize; ++i )
    {
        auto checkIndex = model()->index(i, column);
        bool bEditable = checkIndex.flags() & Qt::ItemIsEditable;

        if(true == bEditable)
        {
            if(checkIndex.data(Qt::CheckStateRole).value<bool>() != select)
            {
                if(column == static_cast<int>(eSearchResultColumn::UML_Applicability))
                {
                    mpSpecificModel->setUML_Applicability(checkIndex, select);
                }
                else if(column == static_cast<int>(eSearchResultColumn::PlotView_Applicability))
                {
                    mpSpecificModel->setPlotView_Applicability(checkIndex, select);
                }
            }
        }
    }
}

void CSearchResultView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    auto pHorizontalScrollBar = horizontalScrollBar();
    auto scrollPosition = 0;

    if(nullptr != pHorizontalScrollBar)
    {
        scrollPosition = horizontalScrollBar()->value();
    }

    tParent::scrollTo(index, hint);

    if(nullptr != pHorizontalScrollBar)
    {
        horizontalScrollBar()->setValue(scrollPosition);
    }
}

void CSearchResultView::setCoverageNoteProvider(const tCoverageNoteProviderPtr& pCoverageNoteProvider)
{
    mpCoverageNoteProvider = pCoverageNoteProvider;
}

void CSearchResultView::setMainTabWidget(QTabWidget* pTabWidget)
{
    mpMainTabWidget = pTabWidget;
}

void CSearchResultView::addComment()
{
    if(mpCoverageNoteProvider)
    {
        if(selectionModel() && !selectionModel()->selectedRows().empty())
        {
            auto coverageNoteId = mpCoverageNoteProvider->addCoverageNoteItem();
            mpCoverageNoteProvider->setCoverageNoteItemRegex(coverageNoteId, mUsedRegex);
            mpCoverageNoteProvider->setCoverageNoteItemMessage(coverageNoteId,
                QString("From the search result<br/><br/>") + getSelectionAsString(true, false));

            if(mpMainTabWidget)
            {
                auto bGroupedViewFeatureActive = getSettingsManager()->getGroupedViewFeatureActive();
                auto bUML_FeatureActive = getSettingsManager()->getUML_FeatureActive();
                mpMainTabWidget->setCurrentIndex(static_cast<int>(eTabIndexes::COVERAGE_NOTE_VIEW)
                                                 - !bGroupedViewFeatureActive - !bUML_FeatureActive);
            }

            mpCoverageNoteProvider->scrollToLastCoveageNoteItem();
        }
    }
}

void CSearchResultView::addCommentFromMainTable()
{
    if(mpCoverageNoteProvider && mpMainTableView)
    {
        if(mpMainTableView->selectionModel() &&
           !mpMainTableView->selectionModel()->selectedRows().empty())
        {
            auto coverageNoteId = mpCoverageNoteProvider->addCoverageNoteItem();
            mpCoverageNoteProvider->setCoverageNoteItemMessage(coverageNoteId,
                QString("From the main table<br/><br/>") + getMainTableSelectionAsString());

            if(mpMainTabWidget)
            {
                auto bGroupedViewFeatureActive = getSettingsManager()->getGroupedViewFeatureActive();
                auto bUML_FeatureActive = getSettingsManager()->getUML_FeatureActive();
                mpMainTabWidget->setCurrentIndex(static_cast<int>(eTabIndexes::COVERAGE_NOTE_VIEW)
                                                 - !bGroupedViewFeatureActive - !bUML_FeatureActive);
            }

            mpCoverageNoteProvider->scrollToLastCoveageNoteItem();
        }
    }
}

void CSearchResultView::handleSettingsManagerChange()
{
    auto pDelegate = new CSearchResultHighlightingDelegate(this);
    pDelegate->setSettingsManager(getSettingsManager());
    setItemDelegate(pDelegate);

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Add comment", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Alt+A")));
            connect(pAction, &QAction::triggered, this, [this]()
            {
                addComment();
            });

            if(true == selectionModel()->selectedRows().empty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        if(mpMainTableView && mpMainTableView->selectionModel())
        {
            QAction* pAction = new QAction("Add comment from main table", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Alt+M")));
            connect(pAction, &QAction::triggered, this, [this]()
            {
                addCommentFromMainTable();
            });

            if(true == mpMainTableView->selectionModel()->selectedRows().empty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Copy", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+C")));
            connect(pAction, &QAction::triggered, this, [this]()
            {
                copySelectionToClipboard( getSettingsManager()->getCopySearchResultAsHTML(), false );
            });

            if(true == selectionModel()->selectedRows().empty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Copy payload", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
            connect(pAction, &QAction::triggered, this, [this]()
            {
                copySelectionToClipboard( getSettingsManager()->getCopySearchResultAsHTML(), true );
            });

            if(true == selectionModel()->selectedRows().empty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Select all", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+A")));
            connect(pAction, &QAction::triggered, [this]()
            {
                selectAll();
            });

            if(0 == model()->rowCount())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        if(model()->rowCount() > 0)
        {
            QAction* pAction = new QAction("Clear search results", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+Space")));
            connect(pAction, &QAction::triggered, this, [this]()
            {
                clearSearchResultsRequested();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Copy as HTML", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setCopySearchResultAsHTML(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getCopySearchResultAsHTML());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        const auto* pSelectionModel = selectionModel();
        auto selectedRows = pSelectionModel->selectedRows();
        const int selectedRowsSize = selectedRows.size();

        auto minSelectedRow = std::min_element( selectedRows.begin(), selectedRows.end(),
                                                [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });
        auto maxSelectedRow = std::max_element( selectedRows.begin(), selectedRows.end(),
                                                [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });

        {
            QAction* pAction = new QAction("Monitor sub-files", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setSubFilesHandlingStatus(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getSubFilesHandlingStatus());
            contextMenu.addAction(pAction);
        }

        {
            if(nullptr != mpFile && true == mpFile->getSubFilesHandlingStatus())
            {
                if(selectedRowsSize >= 1)
                {
                    QString msg = QString("Copy file name(s)");

                    QAction* pAction = new QAction(msg, this);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Alt+C")));
                    connect(pAction, &QAction::triggered, this, [this, selectedRows]()
                    {
                        copyMessageFiles();
                    });
                    contextMenu.addAction(pAction);
                }
            }
        }

        contextMenu.addSeparator();

        {
            if(nullptr != mpSpecificModel &&
               false == mpSpecificModel->getHighlightedRows().empty())
            {
                contextMenu.addSeparator();

                QMenu* pSubMenu = new QMenu("'Grouped view' highlighting settings", this);

                {
                    QAction* pAction = new QAction("Jump to previous msg", this);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Up")));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        jumpToGroupedViewHighlightingMessage(eDirection::Previous);
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Jump to next msg", this);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Down")));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        jumpToGroupedViewHighlightingMessage(eDirection::Next);
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Remove 'grouped view' highlighting", this);
                    pAction->setShortcut(QKeySequence(tr("Esc")));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        clearGroupedViewHighlighting();
                    });

                    pSubMenu->addAction(pAction);
                }

                contextMenu.addMenu(pSubMenu);
            }

            contextMenu.addSeparator();
        }

        {
            if(nullptr != mpFile)
            {
                contextMenu.addSeparator();

                QMenu* pSubMenu = new QMenu("Search range settings", this);

                {
                    QString msg = QString("Set search range ...");

                    QAction* pAction = new QAction(msg, this);
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                       getUserSearchRange();
                    });
                    pSubMenu->addAction(pAction);
                }

                if(selectedRowsSize >= 2)
                {
                    tMsgId from = minSelectedRow->sibling(minSelectedRow->row(), static_cast<int>(eSearchResultColumn::Index)).data().value<tMsgId>();
                    tMsgId to = maxSelectedRow->sibling(maxSelectedRow->row(), static_cast<int>(eSearchResultColumn::Index)).data().value<tMsgId>();

                    tIntRangeProperty prop;
                    prop.isSet = true;
                    prop.from = from;
                    prop.to = ( to >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : to;
                    prop = mpFile->normalizeSearchRange( prop );

                    QString msg = QString("Lock search range from id %1 to id %2").arg(prop.from).arg(prop.to);

                    QAction* pAction = new QAction(msg, this);
                    connect(pAction, &QAction::triggered, this, [this, prop]()
                    {
                        mSearchRange = prop;
                        searchRangeChanged( mSearchRange, false );
                    });
                    pSubMenu->addAction(pAction);
                }
                else if(selectedRowsSize == 1)
                {
                    auto pSelectedRow = selectedRows.begin();
                    tMsgId targetMsgId = pSelectedRow->sibling(pSelectedRow->row(), static_cast<int>(eSearchResultColumn::Index)).data().value<int>();

                    {
                        QString msg = QString("Search \"starting from\" id %1").arg(targetMsgId);

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, this, [this, targetMsgId]()
                        {
                            tIntRangeProperty prop;
                            prop.isSet = true;
                            prop.from = targetMsgId;

                            if( mSearchRange.to >= mpFile->sizeNonFiltered() - 1 )
                            {
                                prop.to = mpFile->sizeNonFiltered() - 1;
                            }
                            else
                            {
                                if(mSearchRange.to > 0)
                                {
                                    prop.to = mSearchRange.to;
                                }
                                else
                                {
                                    prop.to = mpFile->sizeNonFiltered() - 1;
                                }
                            }

                            prop = mpFile->normalizeSearchRange( prop );

                            mSearchRange = prop;
                            searchRangeChanged( mSearchRange, false );
                        });
                        pSubMenu->addAction(pAction);
                    }

                    {
                        QString msg = QString("Search \"up to\" id %1").arg(targetMsgId);

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, this, [this, targetMsgId]()
                        {
                            tIntRangeProperty prop;
                            prop.isSet = true;
                            prop.from = mSearchRange.from;
                            prop.to = ( targetMsgId >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : targetMsgId;
                            prop = mpFile->normalizeSearchRange( prop );

                            mSearchRange = prop;
                            searchRangeChanged( mSearchRange, false );
                        });
                        pSubMenu->addAction(pAction);
                    }
                }

                {
                    pSubMenu->addSeparator();

                    if(true == mSearchRange.isSet)
                    {
                        QString msg = QString("(*) Remove lock on search range from id %1 to id %2").arg( mSearchRange.from ).arg( mSearchRange.to );

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, this, [this]()
                        {
                            mSearchRange = tIntRangeProperty();
                            searchRangeChanged( mSearchRange, true );
                        });
                        pSubMenu->addAction(pAction);
                    }
                }

                contextMenu.addMenu(pSubMenu);
            }
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Columns settings", this);

            {
                QMenu* pSubSubMenu = new QMenu("Search columns", this);

                {
                    const auto& searchResultColumnsSearchMap =
                        getSettingsManager()->getSearchResultColumnsSearchMap();

                    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
                         i < static_cast<int>(eSearchResultColumn::Last);
                         ++i)
                    {
                        auto foundItem = searchResultColumnsSearchMap.find(static_cast<eSearchResultColumn>(i));

                        if(foundItem != searchResultColumnsSearchMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<eSearchResultColumn>(i)), this);
                            connect(pAction, &QAction::triggered, this, [this, i](bool checked)
                                    {
                                        auto searchResultColumnsSearchMapInner =
                                            getSettingsManager()->getSearchResultColumnsSearchMap();

                                        auto foundItemInner = searchResultColumnsSearchMapInner.find(static_cast<eSearchResultColumn>(i));

                                        if(foundItemInner != searchResultColumnsSearchMapInner.end()) // if item is in the map
                                        {
                                            foundItemInner.value() = checked; // let's update copy paste value
                                            getSettingsManager()->setSearchResultColumnsSearchMap(searchResultColumnsSearchMapInner);
                                        }

                                        restartSearch();
                                    });
                            pAction->setCheckable(true);
                            pAction->setChecked(foundItem.value());

                            if( (i == static_cast<int>(eSearchResultColumn::UML_Applicability)
                                 && false == getSettingsManager()->getUML_FeatureActive() ) ||
                                (i == static_cast<int>(eSearchResultColumn::PlotView_Applicability)
                                 && false == getSettingsManager()->getPlotViewFeatureActive() ) )
                            {
                                pAction->setEnabled(false);
                            }

                            pSubSubMenu->addAction(pAction);
                        }
                    }
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            {
                QAction* pAction = new QAction("Reset search columns", this);
                connect(pAction, &QAction::triggered, this, [this]()
                {
                    getSettingsManager()->resetSearchResultColumnsSearchMap();
                    restartSearch();
                });
                pSubMenu->addAction(pAction);
            }

            pSubMenu->addSeparator();

            {
                QMenu* pSubSubMenu = new QMenu("Visible columns", this);

                {
                    const auto& searchResultColumnsVisibilityMap =
                            getSettingsManager()->getSearchResultColumnsVisibilityMap();

                    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
                         i < static_cast<int>(eSearchResultColumn::Last);
                         ++i)
                    {
                        auto foundItem = searchResultColumnsVisibilityMap.find(static_cast<eSearchResultColumn>(i));

                        if(foundItem != searchResultColumnsVisibilityMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<eSearchResultColumn>(i)), this);
                            connect(pAction, &QAction::triggered, this, [this, i](bool checked)
                            {
                                auto searchResultColumnsVisibilityMapInner =
                                        getSettingsManager()->getSearchResultColumnsVisibilityMap();

                                auto foundItemInner = searchResultColumnsVisibilityMapInner.find(static_cast<eSearchResultColumn>(i));

                                if(foundItemInner != searchResultColumnsVisibilityMapInner.end()) // if item is in the map
                                {
                                    foundItemInner.value() = checked; // let's update visibility value
                                    getSettingsManager()->setSearchResultColumnsVisibilityMap(searchResultColumnsVisibilityMapInner);
                                }
                            });
                            pAction->setCheckable(true);
                            pAction->setChecked(foundItem.value());

                            if( (i == static_cast<int>(eSearchResultColumn::UML_Applicability)
                                 && false == getSettingsManager()->getUML_FeatureActive() ) ||
                                (i == static_cast<int>(eSearchResultColumn::PlotView_Applicability)
                                 && false == getSettingsManager()->getPlotViewFeatureActive() ) )
                            {
                                pAction->setEnabled(false);
                            }

                            pSubSubMenu->addAction(pAction);
                        }
                    }
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            {
                QAction* pAction = new QAction("Reset visible columns", this);
                connect(pAction, &QAction::triggered, this, [this]()
                {
                    getSettingsManager()->resetSearchResultColumnsVisibilityMap();
                });
                pSubMenu->addAction(pAction);
            }

            pSubMenu->addSeparator();

            {
                QMenu* pSubSubMenu = new QMenu("Copy columns", this);

                {
                    const auto& searchResultColumnsCopyPasteMap =
                            getSettingsManager()->getSearchResultColumnsCopyPasteMap();

                    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
                         i < static_cast<int>(eSearchResultColumn::Last);
                         ++i)
                    {
                        auto foundItem = searchResultColumnsCopyPasteMap.find(static_cast<eSearchResultColumn>(i));

                        if(foundItem != searchResultColumnsCopyPasteMap.end())
                        {
                            QAction* pAction = new QAction(getName(static_cast<eSearchResultColumn>(i)), this);
                            connect(pAction, &QAction::triggered, this, [this, i](bool checked)
                            {
                                auto searchResultColumnsCopyPasteMapInner =
                                        getSettingsManager()->getSearchResultColumnsCopyPasteMap();

                                auto foundItemInner = searchResultColumnsCopyPasteMapInner.find(static_cast<eSearchResultColumn>(i));

                                if(foundItemInner != searchResultColumnsCopyPasteMapInner.end()) // if item is in the map
                                {
                                    foundItemInner.value() = checked; // let's update copy paste value
                                    getSettingsManager()->setSearchResultColumnsCopyPasteMap(searchResultColumnsCopyPasteMapInner);
                                }
                            });
                            pAction->setCheckable(true);
                            pAction->setChecked(foundItem.value());

                            if( (i == static_cast<int>(eSearchResultColumn::UML_Applicability)
                                 && false == getSettingsManager()->getUML_FeatureActive() ) ||
                                (i == static_cast<int>(eSearchResultColumn::PlotView_Applicability)
                                 && false == getSettingsManager()->getPlotViewFeatureActive() ) )
                            {
                                pAction->setEnabled(false);
                            }

                            pSubSubMenu->addAction(pAction);
                        }
                    }
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            {
                QAction* pAction = new QAction("Reset copy columns", this);
                connect(pAction, &QAction::triggered, this, [this]()
                {
                    getSettingsManager()->resetSearchResultColumnsCopyPasteMap();
                });
                pSubMenu->addAction(pAction);
            }

            pSubMenu->addSeparator();

            {
                QMenu* pSubSubMenu = new QMenu("Last column width policy", this);

                QActionGroup* pActionGroup = new QActionGroup(this);
                pActionGroup->setExclusive(true);

                for(int i = static_cast<int>(eSearchViewLastColumnWidthStrategy::eReset);
                    i < static_cast<int>(eSearchViewLastColumnWidthStrategy::eLast);
                    i++)
                {
                    QAction* pAction = new QAction(getPayloadWidthAsString(static_cast<eSearchViewLastColumnWidthStrategy>(i)), this);
                    connect(pAction, &QAction::triggered, this, [this, i]()
                    {
                        getSettingsManager()->setSearchViewLastColumnWidthStrategy(i);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(getSettingsManager()->getSearchViewLastColumnWidthStrategy() == i);

                    pSubSubMenu->addAction(pAction);
                    pActionGroup->addAction(pAction);
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            contextMenu.addMenu(pSubMenu);

            pSubMenu->addSeparator();

            {
                QAction* pAction = new QAction("Mark timestamp with bold", this);
                connect(pAction, &QAction::triggered, this, [this](bool checked)
                {
                    getSettingsManager()->setMarkTimeStampWithBold(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(getSettingsManager()->getMarkTimeStampWithBold());
                pSubMenu->addAction(pAction);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Highlighting settings", this);

            {
                QAction* pAction = new QAction("Highlight with single color", this);
                connect(pAction, &QAction::triggered, this, [this](bool checked)
                {
                    getSettingsManager()->setSearchResultMonoColorHighlighting(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(getSettingsManager()->getSearchResultMonoColorHighlighting());
                pSubMenu->addAction(pAction);
            }

            if(true == getSettingsManager()->getSearchResultMonoColorHighlighting())
            {
                {
                    QAction* pAction = new QAction("Highlighting color ...", this);
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        QColor color = QColorDialog::getColor( getSettingsManager()->getRegexMonoHighlightingColor(), this );
                        if( color.isValid() )
                        {
                            getSettingsManager()->setRegexMonoHighlightingColor(color);
                        }
                    });
                    pSubMenu->addAction(pAction);
                }

                pSubMenu->addSeparator();
            }
            else
            {
                {
                    QMenu* pSubSubMenu = new QMenu("Gradient highlighting", this);

                    {
                        QAction* pAction = new QAction("Set \"from\" color ...", this);
                        connect(pAction, &QAction::triggered, this, [this]()
                        {
                            const auto& currentGradientValue = getSettingsManager()->getSearchResultHighlightingGradient();

                            QColor color = QColorDialog::getColor( currentGradientValue.from, this );
                            if( color.isValid() )
                            {
                                getSettingsManager()->
                                setSearchResultHighlightingGradient( tHighlightingGradient( color, currentGradientValue.to, currentGradientValue.numberOfColors ) );
                            }
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Set \"to\" color ...", this);
                        connect(pAction, &QAction::triggered, this, [this]()
                        {
                            const auto& currentGradientValue = getSettingsManager()->getSearchResultHighlightingGradient();

                            QColor color = QColorDialog::getColor( currentGradientValue.to, this );
                            if( color.isValid() )
                            {
                                getSettingsManager()->
                                setSearchResultHighlightingGradient( tHighlightingGradient( currentGradientValue.from, color, currentGradientValue.numberOfColors ) );
                            }
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        const auto& currentGradientValue = getSettingsManager()->getSearchResultHighlightingGradient();
                        QString msg = QString("Set number of colors ( cur. val. - %1 ) ...").arg(currentGradientValue.numberOfColors);

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, this, [this]()
                        {
                            bool ok;

                            const auto& currentGradientValue_ = getSettingsManager()->getSearchResultHighlightingGradient();

                            QString numberOfColorsStr = QInputDialog::getText(  nullptr, "Set number of gradient colors",
                                       "Provide number of gradient colors ( from 2 to 100 ):",
                                       QLineEdit::Normal,
                                       QString::number(currentGradientValue_.numberOfColors), &ok );

                            if(true == ok)
                            {
                                auto numberOfColors = numberOfColorsStr.toInt(&ok);

                                if(true == ok)
                                {
                                    if(numberOfColors < 2)
                                    {
                                        numberOfColors = 2;
                                    }
                                    else if( numberOfColors > 100 )
                                    {
                                        numberOfColors = 100;
                                    }

                                    getSettingsManager()->
                                            setSearchResultHighlightingGradient(
                                                tHighlightingGradient(
                                                    currentGradientValue_.from, currentGradientValue_.to, numberOfColors
                                                    ));
                                }
                            }
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        if(true == getSettingsManager()->getUML_FeatureActive())
        {
            {
                QMenu* pSubMenu = new QMenu("UML settings", this);
                pSubMenu->setToolTipsVisible(true);

                {
                    QAction* pAction = new QAction("Select all UML items", this);
                    pAction->setToolTip(tr("Same as Ctrl+A, then Space"));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        selectAllCheckboxItems(true, static_cast<int>(eSearchResultColumn::UML_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Unselect all UML items", this);
                    pAction->setToolTip(tr("Same as Ctrl+A then Space"));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        selectAllCheckboxItems(false, static_cast<int>(eSearchResultColumn::UML_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                pSubMenu->addSeparator();

                {
                    QAction* pAction = new QAction("Find previous UML item", this);
                    pAction->setShortcut(QKeySequence(tr("Alt+Up")));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        switchToNextCheckboxItem(false, static_cast<int>(eSearchResultColumn::UML_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Find next UML item", this);
                    pAction->setShortcut(QKeySequence(tr("Alt+Down")));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        switchToNextCheckboxItem(true, static_cast<int>(eSearchResultColumn::UML_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                contextMenu.addMenu(pSubMenu);
            }

            contextMenu.addSeparator();
        }

        contextMenu.addSeparator();

        if(true == getSettingsManager()->getPlotViewFeatureActive())
        {
            {
                QMenu* pSubMenu = new QMenu("Plot settings", this);
                pSubMenu->setToolTipsVisible(true);

                {
                    QAction* pAction = new QAction("Select all plot items", this);
                    pAction->setToolTip(tr("Same as Ctrl+A, then Space"));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        selectAllCheckboxItems(true, static_cast<int>(eSearchResultColumn::PlotView_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Unselect all plot items", this);
                    pAction->setToolTip(tr("Same as Ctrl+A then Space"));
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        selectAllCheckboxItems(false, static_cast<int>(eSearchResultColumn::PlotView_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                pSubMenu->addSeparator();

                {
                    QAction* pAction = new QAction("Find previous plot item", this);
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        switchToNextCheckboxItem(false, static_cast<int>(eSearchResultColumn::PlotView_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Find next plot item", this);
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        switchToNextCheckboxItem(true, static_cast<int>(eSearchResultColumn::PlotView_Applicability));
                    });

                    pSubMenu->addAction(pAction);
                }

                contextMenu.addMenu(pSubMenu);
            }

            contextMenu.addSeparator();
        }

        contextMenu.addSeparator();

        if(true == getSettingsManager()->getUML_FeatureActive())
        {
            {
                QMenu* pSubMenu = new QMenu("Font settings", this);
                pSubMenu->setToolTipsVisible(true);

                {
                    QAction* pAction = new QAction("Select font ...", this);
                    connect(pAction, &QAction::triggered, this, [this]()
                    {
                        bool ok;
                        QFont selectedFont = QFontDialog::getFont(&ok, font(), this);
                        if (ok)
                        {
                            getSettingsManager()->setFont_SearchView( selectedFont );
                        }
                    });

                    pSubMenu->addAction(pAction);
                }

                contextMenu.addMenu(pSubMenu);
            }

            contextMenu.addSeparator();
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect(getSettingsManager().get(), &ISettingsManager::UML_FeatureActiveChanged, this, [this](bool)
    {
        updateColumnsVisibility();
    });

    connect(getSettingsManager().get(), &ISettingsManager::plotViewFeatureActiveChanged, this, [this](bool)
    {
        updateColumnsVisibility();
    });

    connect(getSettingsManager().get(), &ISettingsManager::font_SearchViewChanged, this, [this](const QFont& font)
    {
        setFont(font);

        QFontMetrics fontMetrics( font );
        verticalHeader()->setDefaultSectionSize(fontMetrics.height());
        forceUpdateWidthAndResetContentMap();
    });

    connect( getSettingsManager().get(), &ISettingsManager::groupedViewFeatureActiveChanged, this, [this]()
    {
        restartSearch();
    });

    connect( getSettingsManager().get(), &ISettingsManager::UML_FeatureActiveChanged, this, [this]()
    {
        restartSearch();
    });

    connect( getSettingsManager().get(), &ISettingsManager::plotViewFeatureActiveChanged, this, [this]()
    {
        restartSearch();
    });

    connect( getSettingsManager().get(),
             &ISettingsManager::searchResultColumnsVisibilityMapChanged,
             this,
             [this](const tSearchResultColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
    });

    connect( getSettingsManager().get(),
             &ISettingsManager::searchViewLastColumnWidthStrategyChanged,
             this,
             [this](const int&)
    {
        forceUpdateWidthAndResetContentMap();
    });
}

void CSearchResultView::clearGroupedViewHighlighting()
{
    if(nullptr != mpSpecificModel)
    {
        mpSpecificModel->setHighlightedRows(tMsgIdSet());
        viewport()->update();
    }
}

void CSearchResultView::jumpToGroupedViewHighlightingMessage(eDirection direction)
{
    if(nullptr != mpSpecificModel)
    {
        const auto& highlightedRows = mpSpecificModel->getHighlightedRows();

        if(false == highlightedRows.empty())
        {
            auto selectedRows = selectionModel()->selectedRows();

            auto jumpToElement = [this](const tMsgId& msgId)
            {
                if(nullptr != mpSpecificModel)
                {
                    auto jumpRow = mpSpecificModel->getRowByMsgId(msgId);

                    if(jumpRow >= 0)
                    {
                        clearSelection();
                        selectRow(jumpRow);

                        auto selectedRows = selectionModel()->selectedRows();
                        if(false == selectedRows.empty())
                        {
                            scrollTo(selectedRows[0]);
                        }
                    }
                }
            };

            auto jumpToFirstHighlightedElement = [&highlightedRows, &jumpToElement]()
            {
                jumpToElement(*highlightedRows.begin());
            };

            auto jumpToLastHighlightedElement = [&highlightedRows, &jumpToElement]()
            {
                jumpToElement(* --highlightedRows.end());
            };

            if(false == selectedRows.empty())
            {
                const auto& selectedRow = selectedRows[0];
                auto msgIdCell = selectedRow.sibling(selectedRow.row(), static_cast<int>(eSearchResultColumn::Index));
                const auto msgId = msgIdCell.data().value<tMsgId>();

                switch(direction)
                {
                    case eDirection::Next:
                    {
                        auto upper = std::upper_bound(highlightedRows.begin(), highlightedRows.end(), msgId);

                        if (upper != highlightedRows.end())
                        {
                            auto nextElementId = *upper;
                            jumpToElement(nextElementId);
                        }
                        else
                        {
                            jumpToFirstHighlightedElement();
                        }
                    }
                    break;
                    case eDirection::Previous:
                    {
                        auto lower = std::lower_bound(highlightedRows.begin(), highlightedRows.end(), msgId);

                        if (lower != highlightedRows.begin())
                        {
                            auto previousElementId = *(--lower);
                            jumpToElement(previousElementId);
                        }
                        else
                        {
                            jumpToLastHighlightedElement();
                        }
                    }
                    break;
                }
            }
            else
            {
                jumpToFirstHighlightedElement();
            }
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_SearchView_API)
    PUML_CLASS_BEGIN_CHECKED(CSearchResultView)
        PUML_INHERITANCE_CHECKED(QTableView, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CSearchResultModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IFileWrapper, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ICoverageNoteProvider, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTabWidget, 1, 1, uses main tab widget)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTableView, 1, 1, uses dlt-viewer main table view)
    PUML_CLASS_END()
PUML_PACKAGE_END()
