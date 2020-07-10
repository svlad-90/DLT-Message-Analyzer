/**
 * @file    CSearchResultView.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSearchResultView class
 */

#include "CSearchResultView.hpp"

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

#include "../common/Definitions.hpp"
#include "CSearchResultHighlightingDelegate.hpp"
#include "CSearchResultModel.hpp"
#include "../settings/CSettingsManager.hpp"
#include "../dltWrappers/CDLTFileWrapper.hpp"


CSearchResultView::CSearchResultView(QWidget *parent):
    tParent(parent),
    mWidthUpdateRequired(eUpdateRequired::eUpdateRequired_NO),
    mbIsVerticalScrollBarVisible(false),
    mpFile(nullptr),
    mSearchRange()
{
    setItemDelegate(new CSearchResultHighlightingDelegate());

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Copy", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copySelectionToClipboard( CSettingsManager::getInstance()->getCopySearchResultAsHTML(), false );
            });
            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Copy payload", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copySelectionToClipboard( CSettingsManager::getInstance()->getCopySearchResultAsHTML(), true );
            });
            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Select all", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+A")));
            connect(pAction, &QAction::triggered, [this]()
            {
                selectAll();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        if(model()->rowCount() > 0)
        {
            QAction* pAction = new QAction("Clear search results", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+Space")));
            connect(pAction, &QAction::triggered, [this]()
            {
                clearSearchResultsRequested();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            if(nullptr != mpFile)
            {
                contextMenu.addSeparator();

                {
                    QString msg = QString("Set search range ...");

                    QAction* pAction = new QAction(msg, this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                       getUserSearchRange();
                    });
                    contextMenu.addAction(pAction);
                }

                const auto* pSelectionModel = selectionModel();
                auto selectedRows = pSelectionModel->selectedRows();
                const int selectedRowsSize = selectedRows.size();
                if(selectedRowsSize >= 2)
                {
                    auto minSelectedRow = std::min_element( selectedRows.begin(), selectedRows.end(),
                                                            [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });
                    auto maxSelectedRow = std::max_element( selectedRows.begin(), selectedRows.end(),
                                                            [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });

                    tMsgId from = minSelectedRow->data().value<int>();
                    tMsgId to = maxSelectedRow->data().value<int>();

                    tRangeProperty prop;
                    prop.isSet = true;
                    prop.from = from;
                    prop.to = ( to >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : to;
                    prop = mpFile->normalizeSearchRange( prop );

                    QString msg = QString("Lock search range from id %1 to id %2").arg(prop.from).arg(prop.to);

                    QAction* pAction = new QAction(msg, this);
                    connect(pAction, &QAction::triggered, [this, prop]()
                    {
                        mSearchRange = prop;
                        searchRangeChanged( mSearchRange, false );
                    });
                    contextMenu.addAction(pAction);
                }
                else if(selectedRowsSize == 1)
                {
                    tMsgId targetMsgId = selectedRows.begin()->data().value<int>();

                    {
                        QString msg = QString("Search \"starting from\" id %1").arg(targetMsgId);

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, [this, targetMsgId]()
                        {
                            tRangeProperty prop;
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
                        contextMenu.addAction(pAction);
                    }

                    {
                        QString msg = QString("Search \"up to\" id %1").arg(targetMsgId);

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, [this, targetMsgId]()
                        {
                            tRangeProperty prop;
                            prop.isSet = true;
                            prop.from = mSearchRange.from;
                            prop.to = ( targetMsgId >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : targetMsgId;
                            prop = mpFile->normalizeSearchRange( prop );

                            mSearchRange = prop;
                            searchRangeChanged( mSearchRange, false );
                        });
                        contextMenu.addAction(pAction);
                    }
                }

                {
                    contextMenu.addSeparator();

                    if(true == mSearchRange.isSet)
                    {
                        QString msg = QString("(*) Remove lock on search range from id %1 to id %2").arg( mSearchRange.from ).arg( mSearchRange.to );

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, [this]()
                        {
                            mSearchRange = tRangeProperty();
                            searchRangeChanged( mSearchRange, true );
                        });
                        contextMenu.addAction(pAction);
                    }
                }
            }
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Copy as HTML", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setCopySearchResultAsHTML(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getCopySearchResultAsHTML());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Visible columns", this);

            {
                const auto& searchResultColumnsVisibilityMap =
                        CSettingsManager::getInstance()->getSearchResultColumnsVisibilityMap();

                for( int i = static_cast<int>(eSearchResultColumn::Index);
                     i < static_cast<int>(eSearchResultColumn::Last);
                     ++i)
                {
                    auto foundItem = searchResultColumnsVisibilityMap.find(static_cast<eSearchResultColumn>(i));

                    if(foundItem != searchResultColumnsVisibilityMap.end())
                    {
                        QAction* pAction = new QAction(getName(static_cast<eSearchResultColumn>(i)), this);
                        connect(pAction, &QAction::triggered, [i](bool checked)
                        {
                            auto searchResultColumnsVisibilityMapInner =
                                    CSettingsManager::getInstance()->getSearchResultColumnsVisibilityMap();

                            auto foundItemInner = searchResultColumnsVisibilityMapInner.find(static_cast<eSearchResultColumn>(i));

                            if(foundItemInner != searchResultColumnsVisibilityMapInner.end()) // if item is in the map
                            {
                                foundItemInner.value() = checked; // let's update visibility value
                                CSettingsManager::getInstance()->setSearchResultColumnsVisibilityMap(searchResultColumnsVisibilityMapInner);
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
                CSettingsManager::getInstance()->resetSearchResultColumnsVisibilityMap();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Copy columns", this);

            {
                const auto& searchResultColumnsCopyPasteMap =
                        CSettingsManager::getInstance()->getSearchResultColumnsCopyPasteMap();

                for( int i = static_cast<int>(eSearchResultColumn::Index);
                     i < static_cast<int>(eSearchResultColumn::Last);
                     ++i)
                {
                    auto foundItem = searchResultColumnsCopyPasteMap.find(static_cast<eSearchResultColumn>(i));

                    if(foundItem != searchResultColumnsCopyPasteMap.end())
                    {
                        QAction* pAction = new QAction(getName(static_cast<eSearchResultColumn>(i)), this);
                        connect(pAction, &QAction::triggered, [i](bool checked)
                        {
                            auto searchResultColumnsCopyPasteMapInner =
                                    CSettingsManager::getInstance()->getSearchResultColumnsCopyPasteMap();

                            auto foundItemInner = searchResultColumnsCopyPasteMapInner.find(static_cast<eSearchResultColumn>(i));

                            if(foundItemInner != searchResultColumnsCopyPasteMapInner.end()) // if item is in the map
                            {
                                foundItemInner.value() = checked; // let's update copy paste value
                                CSettingsManager::getInstance()->setSearchResultColumnsCopyPasteMap(searchResultColumnsCopyPasteMapInner);
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
            QAction* pAction = new QAction("Reset copy columns", this);
            connect(pAction, &QAction::triggered, []()
            {
                CSettingsManager::getInstance()->resetSearchResultColumnsCopyPasteMap();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Highlight with single color", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setSearchResultMonoColorHighlighting(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getSearchResultMonoColorHighlighting());
            contextMenu.addAction(pAction);
        }

        if(true == CSettingsManager::getInstance()->getSearchResultMonoColorHighlighting())
        {
            {
                QAction* pAction = new QAction("Highlighting color ...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    QColor color = QColorDialog::getColor( CSettingsManager::getInstance()->getRegexMonoHighlightingColor(), this );
                    if( color.isValid() )
                    {
                        CSettingsManager::getInstance()->setRegexMonoHighlightingColor(color);
                    }
                });
                contextMenu.addAction(pAction);
            }

            contextMenu.addSeparator();
        }
        else
        {
            {
                QMenu* pSubMenu = new QMenu("Gradient highlighting", this);

                {
                    QAction* pAction = new QAction("Set \"from\" color ...", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        const auto& currentGradientValue = CSettingsManager::getInstance()->getSearchResultHighlightingGradient();

                        QColor color = QColorDialog::getColor( currentGradientValue.from, this );
                        if( color.isValid() )
                        {
                            CSettingsManager::getInstance()->
                            setSearchResultHighlightingGradient( tHighlightingGradient( color, currentGradientValue.to, currentGradientValue.numberOfColors ) );
                        }
                    });
                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Set \"to\" color ...", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        const auto& currentGradientValue = CSettingsManager::getInstance()->getSearchResultHighlightingGradient();

                        QColor color = QColorDialog::getColor( currentGradientValue.to, this );
                        if( color.isValid() )
                        {
                            CSettingsManager::getInstance()->
                            setSearchResultHighlightingGradient( tHighlightingGradient( currentGradientValue.from, color, currentGradientValue.numberOfColors ) );
                        }
                    });
                    pSubMenu->addAction(pAction);
                }

                {
                    const auto& currentGradientValue = CSettingsManager::getInstance()->getSearchResultHighlightingGradient();
                    QString msg = QString("Set number of colors ( cur. val. - %1 ) ...").arg(currentGradientValue.numberOfColors);

                    QAction* pAction = new QAction(msg, this);
                    connect(pAction, &QAction::triggered, []()
                    {
                        bool ok;

                        const auto& currentGradientValue = CSettingsManager::getInstance()->getSearchResultHighlightingGradient();

                        QString numberOfColorsStr = QInputDialog::getText(  nullptr, "Set number of gradient colors",
                                   "Provide number of gradient colors ( from 2 to 100 ):",
                                   QLineEdit::Normal,
                                   QString::number(currentGradientValue.numberOfColors), &ok );

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

                                CSettingsManager::getInstance()->
                                        setSearchResultHighlightingGradient(
                                            tHighlightingGradient(
                                                currentGradientValue.from, currentGradientValue.to, numberOfColors
                                                ));
                            }
                        }
                    });
                    pSubMenu->addAction(pAction);
                }

                contextMenu.addMenu(pSubMenu);
            }
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Mark timestamp with bold", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setMarkTimeStampWithBold(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getMarkTimeStampWithBold());
            contextMenu.addAction(pAction);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect( CSettingsManager::getInstance().get(),
             &CSettingsManager::searchResultColumnsVisibilityMapChanged,
             [this](const tSearchResultColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
    });
}

void CSearchResultView::getUserSearchRange()
{
    auto minOverallVal = 0;
    auto maxOverallVal = mpFile->sizeNonFiltered() - 1;
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
        tRangeProperty prop;
        prop.isSet = true;
        prop.from = from;
        prop.to = ( to >= mpFile->sizeNonFiltered() - 1 ) ? mpFile->sizeNonFiltered() - 1 : to;
        prop = mpFile->normalizeSearchRange( prop );
        mSearchRange = prop;
        searchRangeChanged( mSearchRange, false );
    }
}

void CSearchResultView::setFile( const tDLTFileWrapperPtr& pFile )
{
    mSearchRange = tRangeProperty();
    searchRangeChanged( mSearchRange, true );

    mpFile = pFile;
}

void CSearchResultView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
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

    auto updateWidth = [this]()
    {
        int widthWithoutPayload = 0;

        for(int i = 0; i < static_cast<int>(eSearchResultColumn::Last); ++i)
        {
            if(eSearchResultColumn::Payload != static_cast<eSearchResultColumn>(i))
            {
                if(false == isColumnHidden(i))
                {
                    resizeColumnToContents(i);
                    widthWithoutPayload += columnWidth(i);
                }
            }
        }

        auto payloadWidth = viewport()->size().width() - widthWithoutPayload;

        setColumnWidth( static_cast<int>(eSearchResultColumn::Payload), payloadWidth );
    };

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

void CSearchResultView::updateColumnsVisibility()
{
    const tSearchResultColumnsVisibilityMap& visibilityMap = CSettingsManager::getInstance()->getSearchResultColumnsVisibilityMap();

    for( int i = static_cast<int>(eSearchResultColumn::Index);
         i < static_cast<int>(eSearchResultColumn::Last);
         ++i)
    {
        auto foundColumn = visibilityMap.find(static_cast<eSearchResultColumn>(i));

        if( foundColumn != visibilityMap.end() )
        {
            if(true == foundColumn.value()) // if column is visible
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
                hideColumn(static_cast<int>(i));
            }
        }
        else
        {
             hideColumn(static_cast<int>(i));
        }
    }
}

void CSearchResultView::setModel(QAbstractItemModel *model)
{
    tParent::setModel(model);

    updateColumnsVisibility();

    QHeaderView *verticalHeader = tParent::verticalHeader();
    verticalHeader->resizeSections(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(17);

    QHeaderView *horizontalHeader = tParent::horizontalHeader();

    connect( horizontalHeader, &QHeaderView::sectionDoubleClicked, [this](int logicalIndex)
    {
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

void CSearchResultView::copySelectionToClipboard( bool copyAsHTML, bool copyOnlyPayload ) const
{
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
        const auto& copyPasteColumnsMap = CSettingsManager::getInstance()->getSearchResultColumnsCopyPasteMap();

        int counter = 0;

        for( const auto& copyPaste : copyPasteColumnsMap )
        {
            if(true == copyPaste)
            {
                copyPasteColumns.push_back(counter);

                if(counter == static_cast<int>(eSearchResultColumn::Payload))
                {
                    payloadColumnIdx = counter;
                }
            }

            ++counter;
        }
    }

    typedef QPair<QString /*usual*/, QString /*rich*/> tClipboardItem;
    QVector<tClipboardItem> clipboardItems;
    int finalRichStringSize = 0;
    int finalStringSize = 0;

    int copyPasteColumnsSize = copyPasteColumns.size();

    for(const auto& index : sortedSelectedRows)
    {
        auto getHighlightedColor = [](const tHighlightingRange& range)
        {
            QColor result;

            bool isMonoColorHighlighting = CSettingsManager::getInstance()->getSearchResultMonoColorHighlighting();
            bool isExplicitColor = range.explicitColor;

            if(true == isExplicitColor)
            {
                result = range.color;
            }
            else
            {
                if(true == isMonoColorHighlighting)
                {
                    result = CSettingsManager::getInstance()->getRegexMonoHighlightingColor();
                }
                else
                {
                    result = range.color;
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

            auto attachText = [&clipboardItems, &finalRichStringSize, &finalStringSize, &columnStr, &i, &copyPasteColumnsSize, &field](const tRange& range, const QColor& color, bool isHighlighted)
            {
                bool isHighlightedExtended = ( isHighlighted ||
                                               ( eSearchResultColumn::Timestamp == field
                                                 && true == CSettingsManager::getInstance()->getMarkTimeStampWithBold() ) );

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

                clipboardItems.push_back(tClipboardItem(subStr, str));

                finalRichStringSize += str.size();
                finalStringSize += columnStr.size();
            };

            const auto* pSpecificModel = qobject_cast<CSearchResultModel*>(model());

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
                                    attachText( tRange(0, range.from - 1 ), nonHighlightedColor, false );
                                }

                                attachText( tRange( range.from, range.to ), getHighlightedColor(*it), true );
                            }
                            else if(0 < counter)
                            {
                                auto itPrev = it;
                                --itPrev;
                                const auto& prevRange = *(itPrev);

                                if(prevRange.to < range.from)
                                {
                                    attachText( tRange(prevRange.to + 1, range.from - 1), nonHighlightedColor, false );
                                }

                                attachText( tRange( range.from, range.to ), getHighlightedColor(*it), true );
                            }

                            if(counter == static_cast<int>(foundHighlightingItem->size() - 1)) // last element
                            {
                                if( range.to < columnStr.size() - 1 )
                                {
                                    attachText( tRange(range.to + 1, columnStr.size() - 1), nonHighlightedColor, false );
                                }
                            }

                            ++counter;
                        }
                    }
                    else
                    {
                        attachText( tRange(0, columnStr.size() - 1), nonHighlightedColor, false );
                    }
                }
                else
                {
                    attachText( tRange(0, columnStr.size() - 1), nonHighlightedColor, false );
                }
            }
            else
            {
                attachText( tRange(0, columnStr.size() - 1), nonHighlightedColor, false );
            }
        }

        static const QString newLine("<br>");
        clipboardItems.push_back(tClipboardItem("\n",newLine));
        finalStringSize+=newLine.size();
    }

    QString finalRichText;
    finalRichText.reserve(finalRichStringSize);

    QString finalText;
    finalText.reserve(finalStringSize);

    for(const auto& clipboardItem : clipboardItems)
    {
        finalText.append(clipboardItem.first);
        finalRichText.append(clipboardItem.second);
    }

    QClipboard *pClipboard = QApplication::clipboard();
    QMimeData *rich_text = new QMimeData();

    if(true == copyAsHTML)
    {
        rich_text->setHtml(finalRichText);
    }

    rich_text->setText(finalText);
    pClipboard->setMimeData(rich_text);
}

void CSearchResultView::keyPressEvent ( QKeyEvent * event )
{
    if(event->matches(QKeySequence::Copy))
    {
        if(hasFocus())
        {
            copySelectionToClipboard( CSettingsManager::getInstance()->getCopySearchResultAsHTML(), false );
        }
    }
    else if( ((event->modifiers() & Qt::ShiftModifier) != 0)
             && ((event->modifiers() & Qt::ControlModifier) != 0)
             && (event->key() == Qt::Key::Key_C))
    {
        if(hasFocus())
        {
            copySelectionToClipboard( CSettingsManager::getInstance()->getCopySearchResultAsHTML(), true );
        }
    }
    else if(((event->modifiers() & Qt::ControlModifier) != 0
            && (event->modifiers() & Qt::ShiftModifier) != 0)
            && (event->key() == Qt::Key::Key_Space))
    {
        clearSearchResultsRequested();
    }
    else
    {
       tParent::keyPressEvent(event);
    }
}
