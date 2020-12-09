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
#include <QFontDialog>
#include <QTableWidget>

#include "../common/Definitions.hpp"
#include "CSearchResultHighlightingDelegate.hpp"
#include "CSearchResultModel.hpp"
#include "../settings/CSettingsManager.hpp"
#include "../dltWrappers/CDLTFileWrapper.hpp"
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
    mContentSizeMap()
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

            if(true == selectionModel()->selectedRows().empty())
            {
                pAction->setEnabled(false);
            }

            contextMenu.addAction(pAction);
        }

        {
            QAction* pAction = new QAction("Copy payload", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
            connect(pAction, &QAction::triggered, [this]()
            {
                copySelectionToClipboard( CSettingsManager::getInstance()->getCopySearchResultAsHTML(), true );
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
            connect(pAction, &QAction::triggered, [this]()
            {
                clearSearchResultsRequested();
            });
            contextMenu.addAction(pAction);
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

        const auto* pSelectionModel = selectionModel();
        auto selectedRows = pSelectionModel->selectedRows();
        const int selectedRowsSize = selectedRows.size();

        auto minSelectedRow = std::min_element( selectedRows.begin(), selectedRows.end(),
                                                [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });
        auto maxSelectedRow = std::max_element( selectedRows.begin(), selectedRows.end(),
                                                [](const QModelIndex& lhs, const QModelIndex& rhs){ return lhs.row() < rhs.row(); });

        {
            QAction* pAction = new QAction("Monitor sub-files", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setSubFilesHandlingStatus(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getSubFilesHandlingStatus());
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
                    connect(pAction, &QAction::triggered, [this, selectedRows]()
                    {
                        copyMessageFiles();
                    });
                    contextMenu.addAction(pAction);
                }
            }
        }

        contextMenu.addSeparator();

        {
            if(nullptr != mpFile)
            {
                contextMenu.addSeparator();

                QMenu* pSubMenu = new QMenu("Search range settings", this);

                {
                    QString msg = QString("Set search range ...");

                    QAction* pAction = new QAction(msg, this);
                    connect(pAction, &QAction::triggered, [this]()
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
                    connect(pAction, &QAction::triggered, [this, prop]()
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
                        connect(pAction, &QAction::triggered, [this, targetMsgId]()
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
                        connect(pAction, &QAction::triggered, [this, targetMsgId]()
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
                        connect(pAction, &QAction::triggered, [this]()
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
                QMenu* pSubSubMenu = new QMenu("Visible columns", this);

                {
                    const auto& searchResultColumnsVisibilityMap =
                            CSettingsManager::getInstance()->getSearchResultColumnsVisibilityMap();

                    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
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

                            if(i == static_cast<int>(eSearchResultColumn::UML_Applicability)
                            && false == CSettingsManager::getInstance()->getUML_FeatureActive())
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
                connect(pAction, &QAction::triggered, []()
                {
                    CSettingsManager::getInstance()->resetSearchResultColumnsVisibilityMap();
                });
                pSubMenu->addAction(pAction);
            }

            pSubMenu->addSeparator();

            {
                QMenu* pSubSubMenu = new QMenu("Copy columns", this);

                {
                    const auto& searchResultColumnsCopyPasteMap =
                            CSettingsManager::getInstance()->getSearchResultColumnsCopyPasteMap();

                    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
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

                            if(i == static_cast<int>(eSearchResultColumn::UML_Applicability)
                            && false == CSettingsManager::getInstance()->getUML_FeatureActive())
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
                connect(pAction, &QAction::triggered, []()
                {
                    CSettingsManager::getInstance()->resetSearchResultColumnsCopyPasteMap();
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
                    connect(pAction, &QAction::triggered, [i]()
                    {
                        CSettingsManager::getInstance()->setSearchViewLastColumnWidthStrategy(i);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(CSettingsManager::getInstance()->getSearchViewLastColumnWidthStrategy() == i);

                    pSubSubMenu->addAction(pAction);
                    pActionGroup->addAction(pAction);
                }

                pSubMenu->addMenu(pSubSubMenu);
            }

            contextMenu.addMenu(pSubMenu);

            pSubMenu->addSeparator();

            {
                QAction* pAction = new QAction("Mark timestamp with bold", this);
                connect(pAction, &QAction::triggered, [](bool checked)
                {
                    CSettingsManager::getInstance()->setMarkTimeStampWithBold(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(CSettingsManager::getInstance()->getMarkTimeStampWithBold());
                pSubMenu->addAction(pAction);
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Highlighting settings", this);

            {
                QAction* pAction = new QAction("Highlight with single color", this);
                connect(pAction, &QAction::triggered, [](bool checked)
                {
                    CSettingsManager::getInstance()->setSearchResultMonoColorHighlighting(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(CSettingsManager::getInstance()->getSearchResultMonoColorHighlighting());
                pSubMenu->addAction(pAction);
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
                        pSubSubMenu->addAction(pAction);
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
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        const auto& currentGradientValue = CSettingsManager::getInstance()->getSearchResultHighlightingGradient();
                        QString msg = QString("Set number of colors ( cur. val. - %1 ) ...").arg(currentGradientValue.numberOfColors);

                        QAction* pAction = new QAction(msg, this);
                        connect(pAction, &QAction::triggered, []()
                        {
                            bool ok;

                            const auto& currentGradientValue_ = CSettingsManager::getInstance()->getSearchResultHighlightingGradient();

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

                                    CSettingsManager::getInstance()->
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

        if(true == CSettingsManager::getInstance()->getUML_FeatureActive())
        {
            {
                QMenu* pSubMenu = new QMenu("UML settings", this);
                pSubMenu->setToolTipsVisible(true);

                {
                    QAction* pAction = new QAction("Select all UML items", this);
                    pAction->setToolTip(tr("Same as Ctrl+A, then Space"));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        selectAllUMLItems(true);
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Unselect all UML items", this);
                    pAction->setToolTip(tr("Same as Ctrl+A then Space"));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        selectAllUMLItems(false);
                    });

                    pSubMenu->addAction(pAction);
                }

                pSubMenu->addSeparator();

                {
                    QAction* pAction = new QAction("Find previous UML item", this);
                    pAction->setShortcut(QKeySequence(tr("Alt+Up")));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        switchToNextUMLItem(false);
                    });

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Find next UML item", this);
                    pAction->setShortcut(QKeySequence(tr("Alt+Down")));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        switchToNextUMLItem(true);
                    });

                    pSubMenu->addAction(pAction);
                }

                contextMenu.addMenu(pSubMenu);
            }

            contextMenu.addSeparator();
        }

        contextMenu.addSeparator();

        if(true == CSettingsManager::getInstance()->getUML_FeatureActive())
        {
            {
                QMenu* pSubMenu = new QMenu("Font settings", this);
                pSubMenu->setToolTipsVisible(true);

                {
                    QAction* pAction = new QAction("Select font ...", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        bool ok;
                        QFont selectedFont = QFontDialog::getFont(&ok, font(), this);
                        if (ok)
                        {
                            CSettingsManager::getInstance()->setFont_SearchView( selectedFont );
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

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::UML_FeatureActiveChanged, [this](bool)
    {
        updateColumnsVisibility();
    });

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::font_SearchViewChanged, [this](const QFont& font)
    {
        setFont(font);

        QFontMetrics fontMetrics( font );
        verticalHeader()->setDefaultSectionSize(fontMetrics.height());
        forceUpdateWidthAndResetContentMap();
    });

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
    });

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect( CSettingsManager::getInstance().get(), &CSettingsManager::UML_FeatureActiveChanged, [this]()
    {
        restartSearch();
    });

    connect( CSettingsManager::getInstance().get(),
             &CSettingsManager::searchResultColumnsVisibilityMapChanged,
             [this](const tSearchResultColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
    });

    connect( CSettingsManager::getInstance().get(),
             &CSettingsManager::searchViewLastColumnWidthStrategyChanged,
             [this](const int&)
    {
        forceUpdateWidthAndResetContentMap();
    });
}

void CSearchResultView::newSearchStarted()
{
    mContentSizeMap.clear();
    mbIsViewFull = false;
    mbUserManuallyAdjustedLastVisibleColumnWidth = false;
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

void CSearchResultView::setFile( const tDLTFileWrapperPtr& pFile )
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

    const auto searchViewLastColumnWidthStrategy = static_cast<eSearchViewLastColumnWidthStrategy>(CSettingsManager::getInstance()->getSearchViewLastColumnWidthStrategy());

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
            updateWidthLogic(topLeft.row(), bottomRight.row());
            continueCheck = false;
        }
        else
        {
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
    const tSearchResultColumnsVisibilityMap& visibilityMap = CSettingsManager::getInstance()->getSearchResultColumnsVisibilityMap();

    for( int i = static_cast<int>(eSearchResultColumn::UML_Applicability);
         i < static_cast<int>(eSearchResultColumn::Last);
         ++i)
    {
        auto foundColumn = visibilityMap.find(static_cast<eSearchResultColumn>(i));

        if( foundColumn != visibilityMap.end() )
        {
            if(true == foundColumn.value()) // if column is visible
            {
                if(i != static_cast<int>(eSearchResultColumn::UML_Applicability) ||
                  ((i == static_cast<int>(eSearchResultColumn::UML_Applicability) && (true == CSettingsManager::getInstance()->getUML_FeatureActive() ) ) ) )
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
        const auto& usedFont = CSettingsManager::getInstance()->getFont_SearchView();
        setFont(usedFont);
        verticalHeader->resizeSections(QHeaderView::Fixed);
        QFontMetrics fontMetrics( usedFont );
        verticalHeader->setDefaultSectionSize(fontMetrics.height());
    }

    QHeaderView *horizontalHeader = tParent::horizontalHeader();

    connect( horizontalHeader, &QHeaderView::sectionDoubleClicked, [this](int logicalIndex)
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

            auto attachText = [&clipboardItems, &finalRichStringSize, &finalStringSize, &columnStr, &i, &copyPasteColumnsSize, &field](const tIntRange& range, const QColor& color, bool isHighlighted)
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
    else if(((event->modifiers() & Qt::ControlModifier) != 0
             && (event->modifiers() & Qt::AltModifier) != 0)
             && (event->key() == Qt::Key::Key_C))
    {
        copyMessageFiles();
    }
    else if(event->key() == Qt::Key::Key_Space)
    {
        if(true == CSettingsManager::getInstance()->getUML_FeatureActive())
        {
            if(nullptr != mpSpecificModel)
            {
                auto selectedRows = selectionModel()->selectedRows();

                if(false == selectedRows.empty())
                {
                    auto targetIndex = selectedRows[0];
                    bool bTargetIndexFound = targetIndex.flags() & Qt::ItemIsEditable;

                    if(false == bTargetIndexFound)
                    {
                        for(const auto& selectedRow : selectedRows)
                        {
                            if(selectedRow.flags() & Qt::ItemIsEditable)
                            {
                                targetIndex = selectedRow;
                                bTargetIndexFound = true;
                                break;
                            }
                        }
                    }

                    if(true == bTargetIndexFound)
                    {
                        auto targetValue = !(targetIndex.sibling(targetIndex.row(), static_cast<int>(eSearchResultColumn::UML_Applicability)).data(Qt::CheckStateRole).value<bool>());

                        for(const auto& selectedRow : selectedRows)
                        {
                            auto siblingIdx = selectedRow.sibling(selectedRow.row(), static_cast<int>(eSearchResultColumn::UML_Applicability));

                            if(siblingIdx.data(Qt::CheckStateRole).value<bool>() != targetValue)
                            {
                                mpSpecificModel->setUML_Applicability(selectedRow, targetValue);
                            }
                        }
                    }
                }
            }
        }
    }
    else if((event->modifiers() & Qt::AltModifier) != 0
            && (event->key() == Qt::Key::Key_Down))
    {
        switchToNextUMLItem(true);
    }
    else if((event->modifiers() & Qt::AltModifier) != 0
            && (event->key() == Qt::Key::Key_Up))
    {
        switchToNextUMLItem(false);
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
                auto selectedRowIdx = selectedRowModelIdx.sibling(selectedRowModelIdx.row(), static_cast<int>(eSearchResultColumn::UML_Applicability)).data().value<int>();

                mpFile->copyFileNameToClipboard(selectedRowIdx);
            }
        }
    }
}

void CSearchResultView::switchToNextUMLItem(bool bNext)
{
    if(true == CSettingsManager::getInstance()->getUML_FeatureActive()) // if UML feature is active
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

                auto checkIndexFunc = [this](const int& row)->bool
                {
                    bool bResult = false;

                    auto checkIndex = model()->index(row, static_cast<int>(eSearchResultColumn::UML_Applicability));
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
}

void CSearchResultView::selectAllUMLItems(bool select)
{
    if(true == CSettingsManager::getInstance()->getUML_FeatureActive()) // if UML feature is active
    {
        const auto iSize = model()->rowCount();
        for( int i = 0; i < iSize; ++i )
        {
            auto checkIndex = model()->index(i, static_cast<int>(eSearchResultColumn::UML_Applicability));
            bool bUsedForUML = checkIndex.flags() & Qt::ItemIsEditable;

            if(true == bUsedForUML)
            {
                if(checkIndex.data(Qt::CheckStateRole).value<bool>() != select)
                {
                    mpSpecificModel->setUML_Applicability(checkIndex, select);
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

PUML_PACKAGE_BEGIN(DMA_SearchView)
    PUML_CLASS_BEGIN_CHECKED(CSearchResultView)
        PUML_INHERITANCE_CHECKED(QTableView, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CSearchResultModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CDLTFileWrapper, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
