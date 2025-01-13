/**
 * @file    CDLTMessageAnalyzer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTMessageAnalyzer class
 */

#include "plugin/api/CDLTMessageAnalyzer.hpp"

#include <QtDebug>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QInputDialog>
#include <QHeaderView>
#include "QFileDialog"
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include "QHeaderView"
#include "QScrollBar"
#include "QMainWindow"
#include "QTimer"
#include "QCheckBox"
#include "QPushButton"
#include "QAbstractItemView"
#include "QMenu"
#include "QApplication"
#include "QColorDialog"
#include "QKeyEvent"

#include "common/CTreeItem.hpp"
#include "components/patternsView/api/IPatternsModel.hpp"
#include "components/groupedView/api/CGroupedView.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/logsWrapper/api/IFileWrapper.hpp"
#include "common/CRegexDirectoryMonitor.hpp"

#include "components/groupedView/api/IGroupedViewModel.hpp"
#include "components/searchView/api/CSearchResultView.hpp"
#include "components/searchView/api/ISearchResultModel.hpp"
#include "components/analyzer/api/IDLTMessageAnalyzerController.hpp"
#include "common/CBGColorAnimation.hpp"
#include "components/patternsView/api/CPatternsView.hpp"
#include "components/filtersView/api/CFiltersView.hpp"
#include "components/filtersView/api/IFiltersModel.hpp"
#include "components/log/api/CLog.hpp"
#include "components/plant_uml/api/CUMLView.hpp"
#include "common/CTableMemoryJumper.hpp"
#include "common/CQtHelper.hpp"
#include "components/plotView/api/CCustomPlotExtended.hpp"

#include "components/logsWrapper/api/IDLTLogsWrapperCreator.hpp"

#include "components/regexHistory/api/CRegexHistoryTextEdit.hpp"

#include "DMA_Plantuml.hpp"

namespace NShortcuts
{
    static bool isSaveRegexPatternShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->key() == Qt::Key_S;
    }
}

//CDLTMessageAnalyzer
CDLTMessageAnalyzer::CDLTMessageAnalyzer(const std::weak_ptr<IDLTMessageAnalyzerController>& pController,
                                         const tGroupedViewModelPtr& pGroupedViewModel,
                                         QLabel* pProgressBarLabel, QProgressBar* pProgressBar, CRegexHistoryTextEdit* pRegexLineEdit,
                                         QLabel* pLabel, CPatternsView* pPatternsTableView,  const tPatternsModelPtr& pPatternsModel,
                                         QComboBox* pNumberOfThreadsCombobBox,
                                         QCheckBox* pContinuousSearchCheckBox,
                                         QLabel* pCacheStatusLabel, QTabWidget* pMainTabWidget,
                                         QLineEdit* pPatternsSearchInput,
                                         QComboBox* pRegexSelectionComboBox,
                                         CFiltersView* pFiltersView, const std::shared_ptr<IFiltersModel>& pFiltersModel,
                                         QLineEdit* pFiltersSearchInput,
                                         CUMLView* pUMLView, const std::shared_ptr<CTableMemoryJumper>& pSearchViewTableJumper,
                                         CSearchResultView* pSearchResultView,
                                         const std::shared_ptr<ISearchResultModel>& pSearchResultModel,
                                         const std::weak_ptr<IDLTLogsWrapperCreator>& pDLTLogsWrapperCreator,
                                         const tSettingsManagerPtr& pSettingsManager,
                                         CCustomPlotExtended* pCustomPlotExtended,
                                         const tCoverageNoteProviderPtr& pCoverageNoteProvider):
    IDLTMessageAnalyzerControllerConsumer (pController),
    CSettingsManagerClient(pSettingsManager),
    // default widgets
    mpProgressBarLabel(pProgressBarLabel),
    mpProgressBar(pProgressBar),
    mpRegexTextEdit(pRegexLineEdit),
    mpLabel(pLabel),
    mpNumberOfThreadsCombobBox(pNumberOfThreadsCombobBox),
    mpMainTableView(nullptr),
    mpContinuousSearchCheckBox(pContinuousSearchCheckBox),
    mpCacheStatusLabel(pCacheStatusLabel),
    mpMainTabWidget(pMainTabWidget),
    mpFiltersSearchInput(pFiltersSearchInput),
    //custom widgets and models
    mpGroupedViewModel( pGroupedViewModel ),
    mpRegexSelectionComboBox(pRegexSelectionComboBox),
    mpSearchResultView(pSearchResultView),
    mpSearchResultModel(pSearchResultModel),
    mpPatternsTreeView(pPatternsTableView),
    mpPatternsModel( pPatternsModel ),
    mpFiltersView(pFiltersView),
    mpFiltersModel( pFiltersModel ),
    mpUMLView(pUMLView),
    // internal states
    mSearchRange(),
    mRequestId(INVALID_REQUEST_ID),
    mNumberOfDots(0),
    mbIsConnected(false),
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    mpMessageDecoder(nullptr),
#else
    mDecoderPluginsList(),
    mPluginManager(),
#endif
    mpRegexDirectoryMonitor(nullptr)
  // timers
  #ifdef DEBUG_BUILD
  , mMeasurementNotificationTimer()
  #endif
  , mMeasurementRequestTimer()
  , mpSearchViewTableJumper(pSearchViewTableJumper)
  , mpDLTLogsWrapperCreator(pDLTLogsWrapperCreator)
  , mpCustomPlotExtended(pCustomPlotExtended)
  , mpCoverageNoteProvider(pCoverageNoteProvider)
{
    //////////////METATYPES_REGISTRATION/////////////////////
    qRegisterMetaType<tQStringPtrWrapper>("tQStringPtrWrapper");
    qRegisterMetaType<tIntRange>("tIntRange");
    qRegisterMetaType<const tFoundMatch*>("const tFoundMatch*");

    qRegisterMetaType<eRegexFiltersRowType>("eRegexFiltersRowType");
    qRegisterMetaType<tColorWrapper>("tColorWrapper");

    qRegisterMetaType<tRequestId>("tRequestId");
    qRegisterMetaType<tProcessingStrings>("tProcessingStrings");
    qRegisterMetaType<QRegularExpression>("std::regex");
    qRegisterMetaType<eRequestState>("eRequestState");

    qRegisterMetaType<tFoundMatchesPack>("tFoundMatchesPack");
    qRegisterMetaType<tWorkerId>("tWorkerId");
    qRegisterMetaType<int8_t>("int8_t");
    qRegisterMetaType<tWorkerThreadCookie>("tWorkerThreadCookie");
    qRegisterMetaType<tRegexScriptingMetadata>("tRegexScriptingMetadata");
    /////////////////////////////////////////////////////////

    mpPatternsTreeView->setPatternsSearchInput(pPatternsSearchInput);

    if(nullptr != mpLabel)
    {
        mpLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }

    if(nullptr != mpFiltersModel)
    {
        if(nullptr != mpRegexTextEdit)
        {
            connect( mpRegexTextEdit, &QTextEdit::textChanged, this, [this]()
            {
                mpFiltersModel->setUsedRegex(mpRegexTextEdit->toPlainText());
            });
        }
    }

    if(nullptr != mpProgressBar)
    {
        mpProgressBar->setRange(0, 100);
        mpProgressBar->setValue(0);
        mpProgressBar->repaint();
    }

    if(nullptr != mpPatternsTreeView &&
            nullptr != mpPatternsModel &&
            nullptr != mpFiltersModel)
    {
        mpPatternsModel->updateView();

        for (int c = 0; c < mpPatternsTreeView->header()->count(); ++c)
        {
            mpPatternsTreeView->header()->setSectionResizeMode(
                        c, QHeaderView::ResizeMode::ResizeToContents);
        }

        mpPatternsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        connect ( mpPatternsTreeView, &CPatternsView::patternSelected, [this](const QString& regexCandidate, const QStringList& selectedAliases )
        {
            mpRegexTextEdit->selectAll();
            mpRegexTextEdit->insertPlainText( regexCandidate );
            static_cast<void>(analyze(&selectedAliases));

            if(nullptr != mpFiltersSearchInput)
            {
                mpFiltersSearchInput->setText("");
            }
        });

        connect ( mpFiltersModel.get(), &IFiltersModel::regexUpdatedByUser, this, [this](const QString& regex)
        {
            mpRegexTextEdit->selectAll();
            mpRegexTextEdit->insertPlainText( regex );
            static_cast<void>(analyze());
        });

        if(mpCoverageNoteProvider)
        {
            connect ( mpCoverageNoteProvider.get(), &ICoverageNoteProvider::regexApplicationRequested, this, [this](const QString& regex)
            {
                mpRegexTextEdit->selectAll();
                mpRegexTextEdit->insertPlainText( regex );
                static_cast<void>(analyze());
            });
        }

        connect( mpPatternsTreeView, &CPatternsView::editPatternTriggered, this, [this]()
        {
            editPattern();
        });

        connect( mpPatternsTreeView, &CPatternsView::deletePatternTriggered, this, [this]()
        {
            deletePattern();
        });

        connect( mpPatternsTreeView, &CPatternsView::pastePatternTriggered, this, [this](const CPatternsView::tCopyPastePatternData& copyPastePatternData)
        {
            // if we paste form one file to another
            if(copyPastePatternData.file != getSettingsManager()->getSelectedRegexFile())
            {
                // let's paste all the items.
                for( const auto& copyPastePatternItem : copyPastePatternData.items )
                {
                    processOverwritePattern(copyPastePatternItem.alias, copyPastePatternItem.regex);
                }
            }
        });

        connect( mpPatternsTreeView, &CPatternsView::overwriteFromInputFieldTriggered, this, [this]()
        {
            overwritePattern();
        });
    }

    if(nullptr != mpFiltersSearchInput)
    {
        connect( mpFiltersSearchInput, &QLineEdit::returnPressed, this, [this](){ analyze(); } );
    }

    if(nullptr != mpFiltersModel && nullptr != mpFiltersView)
    {
        connect ( mpFiltersModel.get(), &IFiltersModel::regexUpdatedByUserInvalid,
        [this](const QModelIndex& index, const QString& error)
        {
            mpFiltersView->highlightInvalidRegex(index);
            updateStatusLabel( error, true );
        });
    }

    if(nullptr != mpRegexTextEdit)
    {
        auto showContextMenu = [this](const QPoint &pos)
        {
            QMenu* pContextMenu = mpRegexTextEdit->createStandardContextMenu();

            pContextMenu->addSeparator();

            {
                QAction* pAction = new QAction("Case sensitive search", mpRegexTextEdit);
                connect(pAction, &QAction::triggered, this, [this](bool checked)
                {
                    getSettingsManager()->setCaseSensitiveRegex(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(getSettingsManager()->getCaseSensitiveRegex());
                pContextMenu->addAction(pAction);
            }

            pContextMenu->addSeparator();

            {
                QAction* pAction = new QAction("Save ...", mpRegexTextEdit);
                #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                pAction->setShortcut(Qt::CTRL + Qt::Key_S);
                #else
                pAction->setShortcut(Qt::CTRL | Qt::Key_S);
                #endif
                connect(pAction, &QAction::triggered, this, [this]()
                {
                    addPattern( mpRegexTextEdit->toPlainText() );
                });

                pContextMenu->addAction(pAction);
            }

            pContextMenu->addSeparator();

            auto wrapSelectedText = [this](const QString& addBefore, const QString& addAfter)
            {
                if (mpRegexTextEdit)
                {
                    // Get the selection range
                    int start = mpRegexTextEdit->textCursor().selectionStart();
                    int length = mpRegexTextEdit->textCursor().selectedText().length();

                    if (start >= 0 && length > 0)
                    {
                        // If there is a selection, wrap the selected text
                        QString selectedText = mpRegexTextEdit->textCursor().selectedText();
                        QString wrappedText = addBefore + selectedText + addAfter;

                        // Replace the selected text directly in mpRegexTextEdit
                        mpRegexTextEdit->setSelection(start, length);
                        mpRegexTextEdit->insertPlainText(wrappedText);

                        // Keep the selection that was originally selected
                        mpRegexTextEdit->setSelection(start, wrappedText.length());
                    }
                    else
                    {
                        // If there is no selection, insert addBefore and addAfter at the cursor position
                        int cursorPos = mpRegexTextEdit->textCursor().position();
                        mpRegexTextEdit->insertPlainText(addBefore + addAfter);

                        // Set the new cursor position between addBefore and addAfter
                        mpRegexTextEdit->textCursor().setPosition(cursorPos + addBefore.length());
                    }
                }
            };

            {
                QMenu* pSubMenu = new QMenu("Regex group name glossary", mpRegexTextEdit);

                {
                    QMenu* pSubSubMenu = new QMenu("Highlighting", mpRegexTextEdit);

                    {
                        QAction* pAction = new QAction("Color", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("RGB color", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<RGB_[RED]_[GREEN]_[BLUE]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("'Ok' status", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<OK>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("'Warning' status", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<WARNING>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("'Error' status", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<ERROR>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                {
                    QMenu* pSubSubMenu = new QMenu("Grouped view", mpRegexTextEdit);

                    {
                        QAction* pAction = new QAction("Grouped view level", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<GV>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Ordered grouped view level", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<GV_[INDEX]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                {
                    QMenu* pSubSubMenu = new QMenu("Sequence diagram", mpRegexTextEdit);

                    {
                        QAction* pAction = new QAction("Client name ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<UCL>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Service name ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<US>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Request ( at least one of interaction types is mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<URT>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Response ( at least one of interaction types is mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<URS>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Event ( at least one of interaction types is mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<UEV>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Method ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<UM>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Arguments ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<UA>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Sequence id ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<USID>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Timestamp ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<UTS>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                {
                    QMenu* pSubSubMenu = new QMenu("Plot", mpRegexTextEdit);

                    {
                        QAction* pAction = new QAction("Plot graph name ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PGN_[AXIS_RECT_NAME]_[GRAPH_ID]_[OPTIONAL_PLOT_GRAPH_NAME_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot Y-axis data ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYData_[AXIS_RECT_NAME]_[GRAPH_ID]_[OPTIONAL_VALUE_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot X-axis data ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXData_[AXIS_RECT_NAME]_[GRAPH_ID]_[OPTIONAL_VALUE_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot graph metadata ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PGMD_[AXIS_RECT_NAME]_[GRAPH_ID]_[METADATA_KEY_STRING]_"
                                             "[OPTIONAL_METADATA_VALUE_STRING_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Axis rect type ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PARType_[AXIS_RECT_NAME]_[AXIS_RECT_TYPE_LINEAR_OR_POINT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Axis rect. label ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PARL_[AXIS_RECT_NAME]_[AXIS_RECT_LABEL]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis name ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXN_[AXIS_RECT_NAME]_[NAME]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis name ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYN_[AXIS_RECT_NAME]_[NAME]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis unit ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXN_[AXIS_RECT_NAME]_[UNIT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis unit ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYN_[AXIS_RECT_NAME]_[UNIT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot X-axis time format ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXT_[TIME_FORMAT_FOR_EXAMPLE_4y2Mw2dw2Hw2mw2sw3f]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot Y-axis time format ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYT_[TIME_FORMAT_FOR_EXAMPLE_4y2Mw2dw2Hw2mw2sw3f]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis data max ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXMx_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis data min ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXMn_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis data max ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYMx_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis data min ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYMn_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                {
                    QMenu* pSubSubMenu = new QMenu("Gantt chart", mpRegexTextEdit);

                    {
                        QAction* pAction = new QAction("Axis rect type ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PARType_[AXIS_RECT_NAME]_GANTT>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot graph name ( mandatory )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PGN_[AXIS_RECT_NAME]_[GRAPH_ID]_[OPTIONAL_PLOT_GRAPH_NAME_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Gantt chart event ( mandatory for Gantt charts )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PGE_[AXIS_RECT_NAME]_[GRAPH_ID]_[EVENT_TYPE_start_OR_end]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Gantt chart event row identifier ( optional for Gantt charts )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PGEID_[AXIS_RECT_NAME]_[GRAPH_ID]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot X-axis data ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXData_[AXIS_RECT_NAME]_[GRAPH_ID]_[OPTIONAL_VALUE_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot graph metadata ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PGMD_[AXIS_RECT_NAME]_[GRAPH_ID]_[METADATA_KEY_STRING]_"
                                             "[OPTIONAL_METADATA_VALUE_STRING_IF_IT_SHOULD_NOT_BE_GRABBED_FROM_THE_CAPTURED_CONTENT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Axis rect. label ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PARL_[AXIS_RECT_NAME]_[AXIS_RECT_LABEL]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis name ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXN_[AXIS_RECT_NAME]_[NAME]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis name ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYN_[AXIS_RECT_NAME]_[NAME]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis unit ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXN_[AXIS_RECT_NAME]_[UNIT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis unit ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYN_[AXIS_RECT_NAME]_[UNIT]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot X-axis time format ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXT_[TIME_FORMAT_FOR_EXAMPLE_4y2Mw2dw2Hw2mw2sw3f]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Plot Y-axis time format ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYT_[TIME_FORMAT_FOR_EXAMPLE_4y2Mw2dw2Hw2mw2sw3f]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis data max ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXMx_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("X-axis data min ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PXMn_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis data max ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYMx_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("Y-axis data min ( optional )", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<PYMn_[AXIS_RECT_NAME]_[INTEGER_PART_VALUE]_[REAL_PART_VALUE]_[OPTIONAL_neg]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                {
                    QMenu* pSubSubMenu = new QMenu("Variables", mpRegexTextEdit);

                    {
                        QAction* pAction = new QAction("Variable", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [wrapSelectedText](bool)
                        {
                            wrapSelectedText("(?<VAR_[VARIABLE_NAME]>", ")");
                        });
                        pSubSubMenu->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                pContextMenu->addMenu(pSubMenu);
            }

            pContextMenu->addSeparator();

            {
                QAction* pAction = new QAction("Activate regex history", mpRegexTextEdit);
                #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                pAction->setShortcut(Qt::CTRL + Qt::Key_Space);
                #else
                pAction->setShortcut(Qt::CTRL | Qt::Key_Space);
                #endif
                connect(pAction, &QAction::triggered, this, [this]()
                {
                    mpRegexTextEdit->activateRegexHistory();
                });

                pContextMenu->addAction(pAction);
            }

            {
                QMenu* pSubMenu = new QMenu("Completion settings", mpRegexTextEdit);

                {
                    QAction* pAction = new QAction("Case sensitive", mpRegexTextEdit);
                    connect(pAction, &QAction::triggered, this, [this](bool checked)
                    {
                        getSettingsManager()->setRegexCompletion_CaseSensitive(checked);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(getSettingsManager()->getRegexCompletion_CaseSensitive());
                    pSubMenu->addAction(pAction);
                }

                {
                    QMenu* pSubSubMenu = new QMenu("Search policy", mpRegexTextEdit);

                    QActionGroup* pActionGroup = new QActionGroup(mpRegexTextEdit);
                    pActionGroup->setExclusive(true);

                    {
                        QAction* pAction = new QAction("\"Starts with\"", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [this]()
                        {
                            getSettingsManager()->setRegexCompletion_SearchPolicy(false);
                        });
                        pAction->setCheckable(true);
                        pAction->setChecked(!getSettingsManager()->getRegexCompletion_SearchPolicy());

                        pSubSubMenu->addAction(pAction);
                        pActionGroup->addAction(pAction);
                    }

                    {
                        QAction* pAction = new QAction("\"Contains\"", mpRegexTextEdit);
                        connect(pAction, &QAction::triggered, this, [this]()
                        {
                            getSettingsManager()->setRegexCompletion_SearchPolicy(true);
                        });
                        pAction->setCheckable(true);
                        pAction->setChecked(getSettingsManager()->getRegexCompletion_SearchPolicy());

                        pSubSubMenu->addAction(pAction);
                        pActionGroup->addAction(pAction);
                    }

                    pSubMenu->addMenu(pSubSubMenu);
                }

                pContextMenu->addMenu(pSubMenu);
            }

            pContextMenu->addSeparator();

            {
                QAction* pAction = new QAction("Set regex text edit max height ...", mpRegexTextEdit);
                connect(pAction, &QAction::triggered, this, [this](bool)
                {
                    QDialog dialog(mpRegexTextEdit);
                    QFormLayout form(&dialog);

                    form.addRow(new QLabel("Set regex text edit max height ( number of lines from 1 to 10 ):"));
                    QList<QLineEdit *> fields;

                    QLineEdit* pLineEdit = new QLineEdit(&dialog);

                    {
                        pLineEdit->setText( QString::number( getSettingsManager()->getRegexInputFieldHeight() ) );
                        QString label("Regex text edit max height:");
                        form.addRow(label, pLineEdit);
                        fields << pLineEdit;
                    }

                    dialog.resize(400, dialog.height());

                    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                               Qt::Horizontal, &dialog);
                    form.addRow(&buttonBox);

                    auto accept_handler = [this, &pLineEdit, &dialog]()
                    {
                        if(pLineEdit)
                        {
                            bool ok = false;
                            auto maxHeight = pLineEdit->text().toInt(&ok);

                            if(ok && ( maxHeight > 0 && maxHeight <= 10) )
                            {
                                getSettingsManager()->setRegexInputFieldHeight(maxHeight);
                                dialog.accept();
                            }
                        }
                    };

                    auto reject_handler = [&dialog]()
                    {
                        dialog.reject();
                    };

                    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, accept_handler);
                    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, reject_handler);

                    // Show the dialog as modal
                    dialog.exec();
                });

                pContextMenu->addAction(pAction);
            }

            pContextMenu->exec(mpRegexTextEdit->mapToGlobal(pos));
        };

        connect( mpRegexTextEdit, &QWidget::customContextMenuRequested, this, showContextMenu );
    }

    if(nullptr != mpNumberOfThreadsCombobBox &&
            false == mpController.expired())
    {
        for(auto i = 0; i < mpController.lock()->getMaximumNumberOfThreads(); ++i)
        {
            mpNumberOfThreadsCombobBox->addItem(QString::number(i+1), QVariant(i+1));
        }

        mpNumberOfThreadsCombobBox->setCurrentIndex(mpNumberOfThreadsCombobBox->count() - 1);

        mpNumberOfThreadsCombobBox->repaint();
    }

#ifdef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    {
        QMainWindow* pMainWindow = nullptr;

        foreach(QWidget* pWidget, qApp->topLevelWidgets())
        {
            if(pWidget->inherits("QMainWindow"))
            {
                pMainWindow = qobject_cast<QMainWindow*>( pWidget );
                break;
            }
        }

        if(nullptr != pMainWindow)
        {
            {
                auto pFoundChild = pMainWindow->findChild<QTableView*>( QStringLiteral("tableView") );

                if(nullptr != pFoundChild)
                {
                    setMainTableView(pFoundChild);
                }
            }

            {
                auto pFoundChild = pMainWindow->findChild<QPushButton*>( QStringLiteral("applyConfig") );

                if(nullptr != pFoundChild)
                {
                    connect( pFoundChild, &QPushButton::pressed,
                             this, [this]()
                    {
                        mSearchRange.isFiltered = false; // reset filtering of range
                        cancel();
                    });
                }
            }
        }
    }
#endif

    connect( getSettingsManager().get(), &ISettingsManager::cacheEnabledChanged,
             this, [this](bool val)
    {
        if(nullptr != mpFile)
        {
            mpFile->setEnableCache( val );
            cancel();
        }
    });

    connect( getSettingsManager().get(), &ISettingsManager::cacheMaxSizeMBChanged,
             this, [this](const tCacheSizeMB& val)
    {
        if(nullptr != mpFile)
        {
            mpFile->setMaxCacheSize( MBToB(val) );
            cancel();
        }
    });

    connect( getSettingsManager().get(), &ISettingsManager::RDPModeChanged,
             this, [this](bool val)
    {
        if(nullptr != mpProgressBar &&
                nullptr != mpProgressBarLabel)
        {
            if(true == val)
            {
                mpProgressBar->hide();
                mpProgressBarLabel->hide();
            }
            else
            {
                mpProgressBar->show();
                mpProgressBarLabel->show();
            }
        }
    });

    if(nullptr != mpSearchResultView)
    {
        connect( mpSearchResultView, &CSearchResultView::restartSearch, this, [this]()
        {
            static_cast<void>(analyze());
        });

        connect( mpSearchResultView, &CSearchResultView::searchRangeChanged, this, [this](const tIntRangeProperty& searchRange, bool bReset)
        {
            if(false == bReset)
            {
                mSearchRange = searchRange;
                static_cast<void>(analyze());

                if(nullptr != mpMainTabWidget)
                {
                    if(0 != mpMainTabWidget->count())
                    {
                        mpMainTabWidget->setTabText(0, "Search view (*)");
                    }
                }
            }
            else
            {
                resetSearchRange();
                static_cast<void>(analyze());
            }
        });
    }

    connect( getSettingsManager().get(), &ISettingsManager::regexMonoHighlightingColorChanged,
             this, [this](const QColor&)
    {
        if(nullptr != mpSearchResultModel)
        {
            mpSearchResultModel->updateView();
        }
    });

    connect( getSettingsManager().get(), &ISettingsManager::searchResultHighlightingGradientChanged,
             this, [this](const tHighlightingGradient&)
    {
        if(false == getSettingsManager()->getSearchResultMonoColorHighlighting())
        {
            analyze();
        }
    });

    if(nullptr != mpSearchResultView)
    {
        connect(mpSearchResultView, &CSearchResultView::clearSearchResultsRequested, this, [this](){cancel();});
    }

    mpRegexDirectoryMonitor = std::make_shared<CRegexDirectoryMonitor>();

    if(nullptr != mpRegexDirectoryMonitor)
    {
        connect(mpRegexDirectoryMonitor.get(), &CRegexDirectoryMonitor::regexItemSetChanged,
        this, [this](const CRegexDirectoryMonitor::tFoundRegexItemSet& regexFiles)
        {
            //SEND_MSG(QString("[CDLTMessageAnalyzer]: Fetched selected regex file is - \"%1\"").arg(getSettingsManager()->getSelectedRegexFile()));

            if(nullptr != mpRegexSelectionComboBox)
            {
                if(false == regexFiles.empty()) // check on emptiness
                {
                    unsigned int foundSelectedItemIndex = static_cast<unsigned int>(-1);
                    unsigned int counter = 0;
                    QString selectedItemName = getSettingsManager()->getSelectedRegexFile();

                    mpRegexSelectionComboBox->clear();

                    //SEND_MSG(QString("[CDLTMessageAnalyzer]: Clear files from combo-box"));

                    for(const QString& regexFile : regexFiles)
                    {
                        //SEND_MSG(QString("[CDLTMessageAnalyzer]: Add files combo-box - \"%1\"").arg(regexFile));

                        mpRegexSelectionComboBox->addItem(regexFile);

                        if(selectedItemName == regexFile)
                        {
                            foundSelectedItemIndex = counter;
                        }

                        ++counter;
                    }

                    if(foundSelectedItemIndex != static_cast<unsigned int>(-1))
                    {
                        mpRegexSelectionComboBox->setCurrentIndex(static_cast<int>(foundSelectedItemIndex));
                    }
                    else
                    {
                        // fallback to 0 element of collection ( check on emptiness somwhere above )
                        getSettingsManager()->setSelectedRegexFile(*regexFiles.begin());
                        mpRegexSelectionComboBox->setCurrentIndex(0);
                    }
                }
            }
        });

        mpRegexDirectoryMonitor->setPath(getSettingsManager()->getRegexDirectory());
    }

    connect(getSettingsManager().get(), &ISettingsManager::selectedRegexFileChanged,
    this, [this](const QString& /*regexDirectory*/)
    {
        if(nullptr != mpPatternsModel)
        {
            mpPatternsModel->refreshRegexPatterns();
        }
    });

    connect(getSettingsManager().get(), &ISettingsManager::UML_MaxNumberOfRowsInDiagramChanged,
            this, [this](int)
    {
        createSequenceDiagram();
    });

    connect(getSettingsManager().get(), &ISettingsManager::UML_ShowArgumentsChanged,
            this, [this](bool)
    {
        createSequenceDiagram();
    });

    connect(getSettingsManager().get(), &ISettingsManager::UML_WrapOutputChanged,
            this, [this](bool)
    {
        createSequenceDiagram();
    });

    connect(getSettingsManager().get(), &ISettingsManager::UML_AutonumberChanged,
            this, [this](bool)
    {
        createSequenceDiagram();
    });

    if(nullptr != mpRegexTextEdit)
    {
        mpRegexTextEdit->installEventFilter(this);
    }

    handleLoadedConfig();

    if(nullptr != mpPatternsModel)
    {
        connect(mpPatternsModel.get(), &IPatternsModel::patternsRefreshed, this, [this]()
        {
            updateStatusLabel("Patterns refreshed ...");
        });

        mpPatternsModel->refreshRegexPatterns();
    }

    // set initial cache status
    mpCacheStatusLabel->setText(formCacheStatusString(0,
                                                      MBToB( getSettingsManager()->getCacheMaxSizeMB() ),
                                                      0,
                                                      getSettingsManager()->getCacheEnabled(),
                                                      false));
}

bool CDLTMessageAnalyzer::eventFilter(QObject* pObj, QEvent* pEvent)
{
    bool bResult = false;

    if (pEvent->type() == QEvent::ShortcutOverride )
    {
        QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);
        if(nullptr != mpRegexTextEdit
           && pObj == mpRegexTextEdit
           && true == mpRegexTextEdit->hasFocus() )
        {
            if( true == NShortcuts::isSaveRegexPatternShortcut( pKeyEvent ) )
            {
                addPattern( mpRegexTextEdit->toPlainText() );
                pEvent->accept();
                bResult = true;
            }
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

void CDLTMessageAnalyzer::decodeMsg(QDltMsg& msg) const
{
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    if(nullptr != mpMessageDecoder)
    {
        mpMessageDecoder->decodeMsg(msg, false);
    }
#else
    for( const auto& pDecoderPlugin : mDecoderPluginsList )
    {
        pDecoderPlugin->decodeMsg(msg, false);
    }
#endif
}

void CDLTMessageAnalyzer::createSequenceDiagram() const
{
    if(nullptr != mpSearchResultModel && nullptr != mpUMLView)
    {
        auto UMLResult = mpSearchResultModel->getUMLDiagramContent();

        if(0 != UMLResult.first)
        {
            mpUMLView->generateUMLDiagram(UMLResult.second);
        }
        else
        {
            SEND_WRN( "Can't form empty UML diagram. 0 rows from the \"Search view\" were selected for diagram's creation." );
        }
    }
}

void CDLTMessageAnalyzer::jumpInMainTable(const tMsgId& msgId)
{
    if(nullptr != mpMainTableView)
    {
        if(msgId >= 0)
        {
            auto firstVisibleColumn = -1;

            const int columnsSize = mpMainTableView->model()->columnCount();

            for(int columnCouner = 0; columnCouner < columnsSize; ++columnCouner)
            {
                if(false == mpMainTableView->isColumnHidden(columnCouner))
                {
                    firstVisibleColumn = columnCouner;
                    break;
                }
            }

            if(firstVisibleColumn != -1)
            {
                auto jumpIndex = mpMainTableView->model()->index( msgId, firstVisibleColumn );

                mpMainTableView->setFocus();
                mpMainTableView->scrollTo( jumpIndex, QAbstractItemView::ScrollHint::PositionAtCenter );
                mpMainTableView->setCurrentIndex(jumpIndex);
            }
        }
    }
}

void CDLTMessageAnalyzer::handleLoadedConfig()
{
    if(nullptr != mpNumberOfThreadsCombobBox)
    {
        bool bMatchFound = false;

        const auto& storedNumberOfThreads = getSettingsManager()->getNumberOfThreads();

        for( int i = 0; i < mpNumberOfThreadsCombobBox->count(); ++i )
        {
            if( mpNumberOfThreadsCombobBox->itemData(i).value<int>()
                    == static_cast<int>(storedNumberOfThreads) )
            {
                mpNumberOfThreadsCombobBox->setCurrentIndex(i);
                bMatchFound = true;
                break;
            }
        }

        if(false == bMatchFound)
        {
            mpNumberOfThreadsCombobBox->setCurrentIndex(mpNumberOfThreadsCombobBox->count() - 1);
        }
    }

    if(nullptr != mpContinuousSearchCheckBox)
    {
        Qt::CheckState checkState =
                getSettingsManager()->getContinuousSearch() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        mpContinuousSearchCheckBox->setCheckState( checkState );
    }
}

std::shared_ptr<QRegularExpression> CDLTMessageAnalyzer::createRegex( const QString& regex,
                                                                      const QString& onSuccessMessages,
                                                                      const QString& onFailureMessages,
                                                                      bool appendRegexError,
                                                                      QLineEdit* pRegexLineEdit,
                                                                      QWidget* pErrorAnimationWidget
                                                                      )
{
    QString regex_ = addRegexOptions( regex );

    auto caseSensitiveOption = getSettingsManager()->getCaseSensitiveRegex() ?
                QRegularExpression::NoPatternOption:
                QRegularExpression::CaseInsensitiveOption;

    std::shared_ptr<QRegularExpression> pResult = std::make_shared<QRegularExpression>(regex_, caseSensitiveOption);

    if(0 != regex_.size())
    {
        if(true == pResult->isValid())
        {
            updateStatusLabel( onSuccessMessages, false );
            mpGroupedViewModel->setUsedRegex(regex_);
        }
        else
        {
            QString error(onFailureMessages);

            if(true == appendRegexError)
            {
                error.append(getFormattedRegexError(*pResult));
            }

            updateStatusLabel( error, true );

            if( nullptr != pRegexLineEdit)
            {
                pRegexLineEdit->setFocus();
                auto errorColumn = getRegexErrorColumn(*pResult);
                pRegexLineEdit->setSelection(errorColumn, 1);
            }

            if(nullptr != pErrorAnimationWidget)
            {
                animateError(pErrorAnimationWidget);
            }
        }
    }

    return pResult;
}

std::shared_ptr<QRegularExpression> CDLTMessageAnalyzer::createRegex( const QString& regex,
                                                                      const QString& onSuccessMessages,
                                                                      const QString& onFailureMessages,
                                                                      bool appendRegexError,
                                                                      CRegexHistoryTextEdit* pRegexTextEdit,
                                                                      QWidget* pErrorAnimationWidget
                                                                      )
{
    QString regex_ = addRegexOptions( regex );

    auto caseSensitiveOption = getSettingsManager()->getCaseSensitiveRegex() ?
                QRegularExpression::NoPatternOption:
                QRegularExpression::CaseInsensitiveOption;

    std::shared_ptr<QRegularExpression> pResult = std::make_shared<QRegularExpression>(regex_, caseSensitiveOption);

    if(0 != regex_.size())
    {
        if(true == pResult->isValid())
        {
            updateStatusLabel( onSuccessMessages, false );
            mpGroupedViewModel->setUsedRegex(regex_);
        }
        else
        {
            QString error(onFailureMessages);

            if(true == appendRegexError)
            {
                error.append(getFormattedRegexError(*pResult));
            }

            updateStatusLabel( error, true );

            if( nullptr != pRegexTextEdit)
            {
                pRegexTextEdit->setFocus();
                auto errorColumn = getRegexErrorColumn(*pResult);
                pRegexTextEdit->setSelection(errorColumn, 1);
            }

            if(nullptr != pErrorAnimationWidget)
            {
                animateError(pErrorAnimationWidget);
            }
        }
    }

    return pResult;
}

void CDLTMessageAnalyzer::resetSearchRange()
{
    if(nullptr != mpMainTabWidget)
    {
        if(0 != mpMainTabWidget->count())
        {
            mpMainTabWidget->setTabText(0, "Search view");
        }
    }

    mSearchRange = tIntRangeProperty();
}

void CDLTMessageAnalyzer::setFile(const tFileWrapperPtr& pFile)
{
    mpFile = pFile;

    if(nullptr != mpFile)
    {
        mpFile->setSubFilesHandlingStatus(getSettingsManager()->getSubFilesHandlingStatus());
        mpFile->resetCache();

        connect( getSettingsManager().get(), &ISettingsManager::subFilesHandlingStatusChanged,
                 this, [this](bool val)
        {
            if(nullptr != mpFile)
            {
                mpFile->setSubFilesHandlingStatus(val);
            }
        });
    }

    if(nullptr != mpSearchViewTableJumper)
    {
        mpSearchViewTableJumper->resetSelectedRow();
    }

    resetSearchRange();

    if(nullptr != mpSearchResultView)
    {
        mpSearchResultView->setFile(pFile);
    }

    if(nullptr != mpFile)
    {
        mpFile->setMaxCacheSize( MBToB( static_cast<unsigned int>(getSettingsManager()->getCacheMaxSizeMB()) ) );
        mpFile->setEnableCache( getSettingsManager()->getCacheEnabled() );



#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        if(nullptr != mpMessageDecoder && false == mpDLTLogsWrapperCreator.expired())
        {
            mpFile->setMessageDecoder(mpDLTLogsWrapperCreator.lock()->createMsgDecoder(mpMessageDecoder));
        }  
#else
        if(false == mpDLTLogsWrapperCreator.expired())
        {
            mpFile->setMessageDecoder(mpDLTLogsWrapperCreator.lock()->createMsgDecoder(mDecoderPluginsList));
        }
#endif
    }

    auto updateCacheStatus = [this]()
    {
        if( nullptr != mpCacheStatusLabel )
        {
            if(nullptr != mpFile)
            {
                mpCacheStatusLabel->setText(mpFile->getCacheStatusAsString());
            }
            else
            {
                mpCacheStatusLabel->setText(formCacheStatusString(0,
                                                                  MBToB( getSettingsManager()->getCacheMaxSizeMB() ),
                                                                  0,
                                                                  getSettingsManager()->getCacheEnabled(),
                                                                  false));
            }
        }
    };

    connect( mpFile.get(), &IFileWrapper::isEnabledChanged, this, [updateCacheStatus](bool)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &IFileWrapper::loadChanged, this, [updateCacheStatus](unsigned int)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &IFileWrapper::currentSizeMbChanged, this, [updateCacheStatus](tCacheSizeMB)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &IFileWrapper::maxSizeMbChanged, this, [updateCacheStatus](tCacheSizeMB)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &IFileWrapper::fullChanged, this, [updateCacheStatus](bool)
    {
        updateCacheStatus();
    });
}

bool CDLTMessageAnalyzer::analyze(const QStringList* pSelectedAliases)
{
    if(!mpFile)
    {
        updateStatusLabel( "Initial enabling error! Please, visit the \"https://github.com/svlad-90/DLT-Message-Analyzer/blob/master/md/troubleshooting/troubleshooting.md\", which addresses this issue", true );
        return false;
    }

    if(nullptr != mpUMLView)
    {
        if(true == mpUMLView->isDiagramGenerationInProgress())
        {
            mpUMLView->cancelDiagramGeneration();
        }
        else
        {
            if(true == mpUMLView->isDiagramShown())
            {
                mpUMLView->clearDiagram();
            }
        }
    }

    if(nullptr != mpCustomPlotExtended)
    {
        mpCustomPlotExtended->reset();
        mpCustomPlotExtended->replot();
    }

    if( nullptr == mpProgressBar ||
            nullptr == mpGroupedViewModel ||
            nullptr == mpNumberOfThreadsCombobBox ||
            nullptr == mpSearchResultModel ||
            nullptr == mpContinuousSearchCheckBox ||
            nullptr == mpFiltersView )
    {
        return false;
    }

    if(mpMainTabWidget)
    {
        if(mpMainTabWidget->currentIndex() > (static_cast<int>(eTabIndexes::SEARCH_VIEW)))
        {
            mpMainTabWidget->setCurrentIndex(static_cast<int>(eTabIndexes::SEARCH_VIEW));
        }
    }

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        if(nullptr != mpMessageDecoder && false == mpDLTLogsWrapperCreator.expired())
        {
            mpFile->setMessageDecoder(mpDLTLogsWrapperCreator.lock()->createMsgDecoder(mpMessageDecoder));
        }
#else
        if(false == mpDLTLogsWrapperCreator.expired())
        {
            tryLoadDecoderPlugins();
            mpFile->setMessageDecoder(mpDLTLogsWrapperCreator.lock()->createMsgDecoder(mDecoderPluginsList));
        }
#endif

    mpFile->setMaxCacheSize( MBToB( static_cast<unsigned int>(getSettingsManager()->getCacheMaxSizeMB() ) ) );
    mpFile->setEnableCache( getSettingsManager()->getCacheEnabled() );

    tryStop();

    mpSearchResultModel->resetData();
    mpSearchResultModel->setFile(mpFile);

    mpGroupedViewModel->resetData();

    mpFiltersModel->resetCompletionData();

    QString regex;

    if( nullptr != mpRegexTextEdit )
    {
        regex = mpRegexTextEdit->toPlainText();
    }

    if(nullptr != mpSearchResultView)
    {
        mpSearchResultView->newSearchStarted(regex);
    }

    if(0 != regex.size())
    {
        auto pRegex = createRegex( regex, sDefaultStatusText, "Regex error: ", true, mpRegexTextEdit, mpRegexTextEdit );

        if(nullptr == pRegex || false == pRegex->isValid())
        {
            return false;
        }

        if( nullptr == mpFile ||
                0 == mpFile->size() )
        {
            return false;
        }

        tIntRange requestedMessages;

        if(true == mSearchRange.isSet)
        {
            mSearchRange = mpFile->normalizeSearchRange(mSearchRange);
            requestedMessages.from = mpFile->isFiltered() ? mSearchRange.fromFiltered : mSearchRange.from;
            requestedMessages.to = mpFile->isFiltered() ? mSearchRange.toFiltered : mSearchRange.to;
        }
        else
        {
            requestedMessages.from = 0;
            requestedMessages.to = mpFile->size() - 1;
        }

        tRequestParameters requestParameters
        (
            mpFile,
            requestedMessages.from,
            requestedMessages.to - requestedMessages.from + 1,
            *pRegex,
            mpNumberOfThreadsCombobBox->currentData().value<int>(),
            isContinuousAnalysis(),
            getSettingsManager()->getSearchResultColumnsSearchMap(),
            regex,
            nullptr != pSelectedAliases ? *pSelectedAliases : QStringList()
        );

        releaseMemoryToOS();

        auto requestId = requestAnalyze( requestParameters,
                                         getSettingsManager()->getUML_FeatureActive(),
                                         getSettingsManager()->getPlotViewFeatureActive(),
                                         getSettingsManager()->getGroupedViewFeatureActive());

        setReuqestId(requestId);

        bool bRunning = mRequestId != INVALID_REQUEST_ID;

        if(true == bRunning)
        {
#ifdef DEBUG_BUILD
            mMeasurementNotificationTimer.restart();
#endif
            mMeasurementRequestTimer.restart();

            analysisStatusChanged(true);
            updateProgress(0, eRequestState::PROGRESS, false);
        }

        return bRunning;
    }

    return false;
}

bool CDLTMessageAnalyzer::isContinuousAnalysis() const
{
    bool bResult = false == mSearchRange.isSet;

    if(nullptr != mpContinuousSearchCheckBox)
    {
        bResult = bResult && mpContinuousSearchCheckBox->checkState() == Qt::CheckState::Checked;
    }

    return bResult;
}

void CDLTMessageAnalyzer::cancel()
{
    if( nullptr == mpProgressBar ||
            nullptr == mpGroupedViewModel ||
            nullptr == mpSearchResultModel ||
            nullptr == mpFiltersModel )
    {
        return;
    }

    mpGroupedViewModel->resetData();
    mpSearchResultModel->resetData();

    if(nullptr != mpCustomPlotExtended)
    {
        mpCustomPlotExtended->reset();
        mpCustomPlotExtended->replot();
    }

    mpFiltersModel->resetCompletionData();

    updateProgress(0, eRequestState::SUCCESSFUL, true);

    updateStatusLabel(sDefaultStatusText, false);

    tryStop();

    releaseMemoryToOS();
}

void CDLTMessageAnalyzer::setReuqestId( const tRequestId& val )
{
    mRequestId = val;
}

void CDLTMessageAnalyzer::tryStop()
{
    if(INVALID_REQUEST_ID != mRequestId)
    {
        cancelRequest(mRequestId);
        setReuqestId( INVALID_REQUEST_ID );
        analysisStatusChanged(false);
    }
}

void CDLTMessageAnalyzer::stop(eStopAction stopAction)
{
    if(nullptr != mpProgressBar)
    {
        if(mpProgressBar->value() == mpProgressBar->maximum())
        {
            tryStop();
            updateProgress(100, eRequestState::SUCCESSFUL, false);
        }
        else if(mpProgressBar->value() < mpProgressBar->maximum())
        {
            switch(stopAction)
            {
            case eStopAction::eStopIfNotFinished:
                tryStop();
                break;
            case eStopAction::eCancelIfNotFinished:
                cancel();
                break;
            case eStopAction::eContinueIfNotFinished:
                // do nothing
                break;
            }
        }
    }
}

void CDLTMessageAnalyzer::animateError( QWidget* pAnimationWidget )
{
    if( nullptr == mpProgressBar ||
            nullptr == pAnimationWidget  )
    {
        return;
    }

    if(nullptr != mpRegexTextEdit)
    {
        CBGColorAnimation* pAnimationProxy = new CBGColorAnimation(pAnimationWidget, QPalette::Base);

        QSequentialAnimationGroup* pAnimSeq = new QSequentialAnimationGroup();

        QPropertyAnimation *pColor = new QPropertyAnimation(pAnimationProxy, "color");
        pColor->setDuration(250);
        pColor->setStartValue(qApp->palette().base());
        pColor->setEndValue(QColor(255, 0, 0));

        pAnimSeq->addAnimation(pColor);

        QPropertyAnimation *pUncolor = new QPropertyAnimation(pAnimationProxy, "color");
        pUncolor->setDuration(250);
        pUncolor->setStartValue(QColor(255, 0, 0));
        pUncolor->setEndValue(qApp->palette().base());

        pAnimSeq->addAnimation(pUncolor);

        pAnimSeq->start();

        updateProgress(0, eRequestState::ERROR_STATE, true);
    }
}

void CDLTMessageAnalyzer::updateProgress(int progress, eRequestState requestState, bool silent)
{
    if( nullptr == mpProgressBar )
    {
        return;
    }

    if(false == silent)
    {
        QString progressStr;

        progressStr.append( getName(requestState) ).append("( ").append( QString::number(progress) ).append(" ) ");

        if( eRequestState::PROGRESS == requestState )
        {
            for( int i = 0; i < mNumberOfDots; ++i )
            {
                progressStr.append(".");
            }

            ++mNumberOfDots;

            if( mNumberOfDots == 4 )
            {
                mNumberOfDots = 0;
            }
        }

        updateStatusLabel(progressStr, requestState == eRequestState::ERROR_STATE );
    }

    mpProgressBar->setValue(progress);
    mpProgressBar->repaint();
}

void CDLTMessageAnalyzer::progressNotification(const tProgressNotificationData& progressNotificationData)
{
    //qDebug() << "CDLTMessageAnalyzer::" << __FUNCTION__;

    //#ifdef DEBUG_BUILD
    //        qDebug() << QString("CDLTMessageAnalyzer::progressNotification: start; request id - %1; was waiting for - %2 ms")
    //                    .arg(requestId)
    //                    .arg(QLocale().toString(mMeasurementNotificationTimer.elapsed()), 4);
    //        mMeasurementNotificationTimer.restart();
    //#endif

    if(mRequestId == progressNotificationData.requestId)
    {
        if( nullptr == mpGroupedViewModel ||
            nullptr == mpSearchResultModel ||
            nullptr == mpFiltersModel )
        {
            return;
        }

        //qDebug() << "CDLTMessageAnalyzer::" << __FUNCTION__ << ": progress - " << progress << "; requestStats - " << static_cast<int>(requestState);

        switch(progressNotificationData.requestState)
        {
            case eRequestState::SUCCESSFUL:
            {
                for(auto foundMatchesIt = progressNotificationData.processedMatches.matchedItemVec.begin(); foundMatchesIt !=
                    progressNotificationData.processedMatches.matchedItemVec.end(); ++foundMatchesIt)
                {
                    if(nullptr != *foundMatchesIt)
                    {
                        auto endIt = progressNotificationData.processedMatches.matchedItemVec.end();
                        if(true == isGroupedViewFeatureActiveForCurrentAnalysis())
                        {
                            mpGroupedViewModel->addMatches( progressNotificationData.groupedViewIndices,
                                                            (*foundMatchesIt)->getFoundMatches(),
                                                            foundMatchesIt == --(endIt));
                        }

                        mpFiltersModel->addCompletionData((*foundMatchesIt)->getFoundMatches());
                    }
                }

                updateProgress(progressNotificationData.progress,
                               progressNotificationData.requestState,
                               false);

                if( false == isContinuousAnalysis() )
                {
                    analysisStatusChanged(false);
                    setReuqestId( INVALID_REQUEST_ID );
                }

                {
                    SEND_MSG(QString("CDLTMessageAnalyzer::progressNotification: request id - %1; overall processing took - %2 ms")
                             .arg(progressNotificationData.requestId)
                             .arg(QLocale().toString(mMeasurementRequestTimer.elapsed()), 4));

                    releaseMemoryToOS();

                    mMeasurementRequestTimer.restart();
                }
            }
                break;
            case eRequestState::ERROR_STATE:
            {
                updateStatusLabel("Error ...", true);

                updateProgress(0, eRequestState::ERROR_STATE, false);

                mpGroupedViewModel->resetData();

                setReuqestId( INVALID_REQUEST_ID );
                analysisStatusChanged(false);
            }
                break;
            case eRequestState::PROGRESS:
            {
                for(auto foundMatchesIt = progressNotificationData.processedMatches.matchedItemVec.begin();
                    foundMatchesIt != progressNotificationData.processedMatches.matchedItemVec.end();
                    ++foundMatchesIt)
                {
                    if(nullptr != *foundMatchesIt)
                    {
                        auto endIt = progressNotificationData.processedMatches.matchedItemVec.end();

                        if(true == isGroupedViewFeatureActiveForCurrentAnalysis())
                        {
                            mpGroupedViewModel->addMatches(progressNotificationData.groupedViewIndices,
                                                           (*foundMatchesIt)->getFoundMatches(),
                                                           foundMatchesIt == --(endIt));
                        }

                        mpFiltersModel->addCompletionData((*foundMatchesIt)->getFoundMatches());
                    }
                }

                updateProgress(progressNotificationData.progress,
                               progressNotificationData.requestState, false);

                if(progressNotificationData.progress == 100 && false == mbIsConnected)
                {
                    cancelRequest(progressNotificationData.requestId);
                    updateProgress(progressNotificationData.progress, eRequestState::SUCCESSFUL, false);
                    analysisStatusChanged(false);

                    SEND_MSG(QString("CDLTMessageAnalyzer::progressNotification: request id - %1; overall processing took - %2 ms")
                             .arg(progressNotificationData.requestId)
                             .arg(QLocale().toString(mMeasurementRequestTimer.elapsed()), 4));

                    releaseMemoryToOS();
                }

                break;
            }
        }

        auto additionResult = mpSearchResultModel->addNextMessageIdxVec( progressNotificationData.processedMatches );

        if(nullptr != mpSearchViewTableJumper &&
           nullptr != mpSearchResultModel &&
           additionResult.first &&
           false == progressNotificationData.processedMatches.matchedItemVec.empty())
        {
            if( (additionResult.second.to - additionResult.second.from + 1) ==
                 static_cast<int>(progressNotificationData.processedMatches.matchedItemVec.size()))
            {
                int counter = 0;
                CTableMemoryJumper::tCheckSet checkSet;

                for(const auto& match : progressNotificationData.processedMatches.matchedItemVec)
                {
                    if(nullptr != match)
                    {
                        CTableMemoryJumper::tCheckItem checkItem;
                        checkItem.first = static_cast<CTableMemoryJumper::tRowID>( match->getItemMetadata().msgId );
                        checkItem.second = additionResult.second.from + counter;
                        checkSet.insert(checkItem);
                        ++counter;
                    }
                }

                mpSearchViewTableJumper->checkRows(checkSet);
            }
        }
    }

    //#ifdef DEBUG_BUILD
    //        qDebug() << QString("CDLTMessageAnalyzer::progressNotification: end; request id - %1; method took - %2 ms")
    //                    .arg(requestId)
    //                    .arg(QLocale().toString(mMeasurementNotificationTimer.elapsed()), 4);
    //        mMeasurementNotificationTimer.restart();
    //#endif

}

void CDLTMessageAnalyzer::exportGroupedViewToHTML()
{
    if(nullptr != mpGroupedViewModel )
    {
        if( 0 != mpGroupedViewModel->rowCount() )
        {
            QString HTML_Str;

            auto exportResult= mpGroupedViewModel->exportToHTML(HTML_Str);

            if(true == exportResult.first)
            {
                QString filters("HTML (*.html);;All files (*.*)");
                QString defaultFilter("HTML (*.html)");

                auto filePath = QFileDialog::getSaveFileName(nullptr, "Save file", QCoreApplication::applicationDirPath(), filters, &defaultFilter);

                if(0 != filePath.size())
                {
                    QString absolutePath = QFileInfo(filePath).absoluteFilePath();

                    QFile file(absolutePath);

                    if(true == file.open(QFile::WriteOnly))
                    {
                        QTextStream outputStream(&file);
                        outputStream << HTML_Str;
                        file.close();

                        updateStatusLabel(QString( "HTML report successfully saved to - \"" ).append(filePath).append("\""), false);
                    }
                    else
                    {
                        updateStatusLabel(QString( "Creation of report has failed. Was not able to open file - \"" ).append(filePath).append("\""), true);
                    }
                }
                else
                {
                    updateStatusLabel(QString( "Nothing to store. Result HTML is empty." ), true);
                }
            }
            else
            {
                updateStatusLabel(exportResult.second, true);
            }
        }
        else
        {
            updateStatusLabel(QString( "Nothing to store. 0 results available." ), true);
        }
    }
}

void CDLTMessageAnalyzer::addPattern(const QString& pattern)
{
    if( nullptr == mpPatternsModel ||
        nullptr == mpPatternsTreeView )
    {
        return;
    }

    bool ok;

    auto pRegex = createRegex( pattern, sDefaultStatusText, "Pattern not saved. Regex error: ", true, mpRegexTextEdit, mpRegexTextEdit );

    if(true == pRegex->isValid())
    {
        QString proposedName;

        if(nullptr != mpPatternsTreeView)
        {
            auto selectedRows = mpPatternsTreeView->selectionModel()->selectedRows();

            if(false == selectedRows.empty())
            {
                auto selectedRow = selectedRows[0];
                auto selectedRowTypeIdx = selectedRow.sibling(selectedRow.row(), static_cast<int>(ePatternsColumn::RowType));
                auto selectedRowType = selectedRowTypeIdx.data().value<ePatternsRowType>();

                switch(selectedRowType)
                {
                    case ePatternsRowType::ePatternsRowType_Alias:
                    {
                        auto parentIdx = selectedRow.parent();

                        if(true == parentIdx.isValid())
                        {
                            auto parentAliasIdx = parentIdx.sibling(parentIdx.row(), static_cast<int>(ePatternsColumn::Alias));
                            proposedName = parentAliasIdx.data().value<QString>();
                        }
                    }
                        break;
                    case ePatternsRowType::ePatternsRowType_FakeTreeLevel:
                    {
                        auto aliasIdx = selectedRow.sibling(selectedRow.row(), static_cast<int>(ePatternsColumn::Alias));
                        proposedName = aliasIdx.data().value<QString>();
                    }
                        break;
                }
            }
        }

        QString alias = QInputDialog::getText(  nullptr, "Specify alias",
                                                "Enter alias name:",
                                                QLineEdit::Normal, proposedName, &ok );

        if( ok )
        {
            processOverwritePattern(alias, pattern);
        }
    }
}

void CDLTMessageAnalyzer::updatePatternsInPersistency()
{
    if(nullptr != mpPatternsModel)
    {
        mpPatternsModel->updatePatternsInPersistency();
    }
}

void CDLTMessageAnalyzer::processOverwritePattern(const QString& alias, const QString checkedRegex, const QModelIndex editItem)
{
    if(nullptr == mpPatternsModel || nullptr == mpPatternsTreeView)
        return;

    bool bContinue = false;
    QString warningMsg;

    if(false == editItem.isValid())
    {
        bContinue = true;
    }
    else
    {
        auto rowTypeIdx = editItem.sibling(editItem.row(), static_cast<int>(ePatternsColumn::RowType));
        auto rowType = rowTypeIdx.data().value<ePatternsRowType>();

        switch(rowType)
        {
            case ePatternsRowType::ePatternsRowType_Alias:
            bContinue = !alias.isEmpty();
            if(false == bContinue)
            {
                warningMsg = "Pattern not saved. No alias provided";
            }
                break;
            case ePatternsRowType::ePatternsRowType_FakeTreeLevel:
            bContinue = true;
                break;
        }
    }

    if( true == bContinue ) // if we should continue
    {
        auto foundPattern = mpPatternsModel->search(alias);

        auto overwriteItem = [this, &alias, &foundPattern, &checkedRegex, &editItem](Qt::CheckState isDefault, Qt::CheckState isCombine)
        {
            if(true == foundPattern.bFound)
            {
                QString nodeName = foundPattern.foundIdx == QModelIndex() ?
                            "Root item" :
                            alias;

                QMessageBox::StandardButton reply;

                QString message;

                if( foundPattern.foundIdx == QModelIndex() )
                {
                    message = QString("Do you want to extend already existing tree level - \"")
                            .append(nodeName).append("\"?");
                }
                else
                {
                    auto rowTypeIdx = foundPattern.foundIdx.sibling(foundPattern.foundIdx.row(), static_cast<int>(ePatternsColumn::RowType));
                    auto rowType = rowTypeIdx.data().value<ePatternsRowType>();

                    switch(rowType)
                    {
                        case ePatternsRowType::ePatternsRowType_Alias:
                        message = QString("Do you want to overwrite already existing pattern - \"")
                                .append(nodeName).append("\"?");
                            break;
                        case ePatternsRowType::ePatternsRowType_FakeTreeLevel:
                        message = QString("Do you want to extend already existing tree level - \"")
                                .append(nodeName).append("\"?");
                            break;
                    }
                }

                reply = QMessageBox::question(mpPatternsTreeView,
                                              "Edit pattern", message,
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes)
                {
                    auto editedItemIdx = mpPatternsModel->editData( editItem, alias, checkedRegex, isDefault, isCombine );
                    mpPatternsTreeView->scrollTo(editedItemIdx, QTableView::ScrollHint::PositionAtCenter);
                    mpPatternsTreeView->setCurrentIndex(editedItemIdx);
                    mpPatternsTreeView->setFocus();

                    updateStatusLabel( QString("Pattern \"").append(alias).append("\" overwriten ..."), false);

                    updatePatternsInPersistency();
                }
            }
        };

        auto saveItem = [this, &alias, &checkedRegex, &editItem](bool edit, Qt::CheckState isDefault, Qt::CheckState isCombine)
        {
            if(true == edit)
            {
                auto editedItemIdx = mpPatternsModel->editData(editItem, alias, checkedRegex, isDefault, isCombine);
                mpPatternsTreeView->scrollTo(editedItemIdx, QTableView::ScrollHint::PositionAtCenter);
                mpPatternsTreeView->setCurrentIndex(editedItemIdx);
            }
            else
            {
                QModelIndex modelIndex = mpPatternsModel->addData(alias, checkedRegex);
                mpPatternsTreeView->reset();
                mpPatternsTreeView->scrollTo(modelIndex, QTableView::ScrollHint::PositionAtCenter);
                mpPatternsTreeView->setCurrentIndex(modelIndex);
            }

            mpPatternsTreeView->setFocus();
            updateStatusLabel(QString("Pattern \"").append(alias).append("\" saved ..." ), false);

            updatePatternsInPersistency();
        };

        auto isDefault = V_2_CS( foundPattern.foundIdx.sibling(foundPattern.foundIdx.row(),
                                                       static_cast<int>(ePatternsColumn::Default)).data() );
        auto isCombine = V_2_CS( foundPattern.foundIdx.sibling(foundPattern.foundIdx.row(),
                                                       static_cast<int>(ePatternsColumn::Combine)).data() );

        bool bEditItemValid = editItem.isValid();

        if(false == bEditItemValid) // if there is no edit item available
        {

            if (true == foundPattern.bFound) // if we've found an element
            {
                overwriteItem(isDefault, isCombine); // propose to overwrite
            }
            else // otherwise
            {
                saveItem(false, isDefault, isCombine); // save item
            }
        }
        else // if edit item is available
        {
            if (true == foundPattern.bFound) // if we've found an element
            {
                if(editItem != foundPattern.foundIdx) // and it is not an edit item
                {
                    overwriteItem(isDefault, isCombine); // propose to overwrite
                }
                else // otherwise
                {
                    saveItem(true, isDefault, isCombine); // save item
                }
            }
            else // otherwise
            {
                saveItem(true,
                         V_2_CS( editItem.sibling(editItem.row(),
                                           static_cast<int>(ePatternsColumn::Default)).data() ),
                         V_2_CS( editItem.sibling(editItem.row(),
                                           static_cast<int>(ePatternsColumn::Combine)).data() ) ); // edit item
            }
        }
    }
    else
    {
        updateStatusLabel(warningMsg, true);
    }
}

void CDLTMessageAnalyzer::deletePattern()
{
    if( nullptr == mpPatternsModel ||
            nullptr == mpPatternsTreeView )
    {
        return;
    }

    auto selectedRows = mpPatternsTreeView->selectionModel()->selectedRows();

    if(1 == selectedRows.size())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(mpPatternsTreeView, "Pattern deletion", "Delete selected pattern?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            mpPatternsModel->removeData(selectedRows[0]);

            updateStatusLabel("Pattern deleted ...", false);

            updatePatternsInPersistency();
        }
        else
        {
            updateStatusLabel(sDefaultStatusText, false);
        }
    }
    else
    {
        updateStatusLabel("Deletion of pattern is possible when 1 element is selected!", true);
    }
}

void CDLTMessageAnalyzer::overwritePattern()
{
    if( nullptr == mpPatternsModel ||
            nullptr == mpPatternsTreeView ||
            nullptr == mpRegexTextEdit )
    {
        return;
    }

    auto selectedRows = mpPatternsTreeView->selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Alias));

    if(1 == selectedRows.size())
    {
        const auto& selectedRow = selectedRows[0];

        QString alias = selectedRow.data().value<QString>();
        QString regex = mpRegexTextEdit->toPlainText();
        auto pParsedRegex = createRegex( regex, sDefaultStatusText, "Pattern not updated. Regex error: ", true, mpRegexTextEdit, mpRegexTextEdit );

        if(nullptr != pParsedRegex && true == pParsedRegex->isValid())
        {
            processOverwritePattern(alias, regex, selectedRow);
        }
    }
    else
    {
        updateStatusLabel("Overwrite operation is possible when 1 element is selected!", true);
    }
}

void CDLTMessageAnalyzer::editPattern()
{
    if( nullptr == mpPatternsModel ||
            nullptr == mpPatternsTreeView )
    {
        return;
    }

    auto selectedRows = mpPatternsTreeView->selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Alias));

    if(1 == selectedRows.size())
    {
        auto selectedRow = selectedRows[0];

        QDialog dialog(mpPatternsTreeView);
        // Use a layout allowing to have a label next to each field
        QFormLayout form(&dialog);

        // Add some text above the fields
        form.addRow(new QLabel("Edit pattern"));

        // Add the lineEdits with their respective labels
        QList<QLineEdit *> fields;

        QLineEdit *aliasLineEdit = new QLineEdit(&dialog);

        {
            Q_ASSERT(mpPatternsModel != nullptr);
            aliasLineEdit->setText( mpPatternsModel->getAliasEditName(selectedRow) );
            QString label("Alias:");
            form.addRow(label, aliasLineEdit);
            fields << aliasLineEdit;
        }

        auto rowTypeIdx = selectedRow.sibling(selectedRow.row(), static_cast<int>(ePatternsColumn::RowType));

        QLineEdit *pRegexLineEdit = nullptr;

        if(ePatternsRowType::ePatternsRowType_Alias ==
           rowTypeIdx.data().value<ePatternsRowType>())
        {
            pRegexLineEdit = new QLineEdit(&dialog);

            {
                pRegexLineEdit->setText(
                            selectedRow.sibling(selectedRow.row(),
                                                static_cast<int>(ePatternsColumn::Regex)).data().value<QString>());
                QString label("Regex:");
                form.addRow(label, pRegexLineEdit);
                fields << pRegexLineEdit;
            }
        }

        dialog.resize(400, dialog.height());

        // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);

        auto accept_handler = [this, &pRegexLineEdit, &aliasLineEdit, &dialog, &selectedRow]()
        {
            if(nullptr != pRegexLineEdit)
            {
                auto pRegex = createRegex( pRegexLineEdit->text(), sDefaultStatusText, "Pattern not updated. Regex error: ", true, pRegexLineEdit, pRegexLineEdit );

                if(nullptr != pRegex && true == pRegex->isValid())
                {
                    processOverwritePattern( aliasLineEdit->text(), pRegexLineEdit->text(), selectedRow);
                    dialog.accept();
                }
            }
            else
            {
                processOverwritePattern( aliasLineEdit->text(), "", selectedRow);
                dialog.accept();
            }
        };

        auto reject_handler = [this, &dialog]()
        {
            updateStatusLabel("Pattern not saved. Operation rejected.", false);
            dialog.reject();
        };

        QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, accept_handler);
        QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, reject_handler);

        // Show the dialog as modal
        dialog.exec();
    }
    else
    {
        updateStatusLabel("Edit of pattern is possible when 1 element is selected!", true);
    }
}

void CDLTMessageAnalyzer::updateStatusLabel( const QString& text, bool isError )
{
    if(nullptr != mpLabel)
    {
        mpLabel->setText( text );

        if(true == isError)
        {
            mpLabel->setStyleSheet("QLabel { background-color: red; color : rgb(255,255,255); border: 3px solid white; }");
        }
        else
        {
            mpLabel->setStyleSheet("QLabel { background-color: gray; color : black; border: 3px solid white; }");
        }
    }
}

#ifdef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
void CDLTMessageAnalyzer::tryLoadDecoderPlugins()
{
    if(true == mDecoderPluginsList.empty())
    {
        /* load plugins from subdirectory plugins, from directory if set in settings and from /usr/share/dlt-viewer/plugins in Linux */
        mPluginManager.loadPlugins("");

        auto allPlugins = mPluginManager.getPlugins();

        for(auto& pDecoderPlugin : allPlugins)
        {
            if(nullptr != pDecoderPlugin && true == pDecoderPlugin->isDecoder())
            {
                pDecoderPlugin->setMode(QDltPlugin::Mode::ModeEnable);
                qDebug() << "Decode plugin added - " << pDecoderPlugin->getName();
            }
        }

        /* update plugin widgets */

        mDecoderPluginsList = mPluginManager.getDecoderPlugins();
    }
}
#endif

void CDLTMessageAnalyzer::searchView_clicked_jumpTo_inMainTable(const QModelIndex &index)
{
    if(nullptr != mpSearchResultModel &&
            nullptr != mpMainTableView)
    {
        auto fileIdx = mpSearchResultModel->getFileIdx(index);

        if(fileIdx >= 0)
        {
            auto firstVisibleColumn = -1;

            const int columnsSize = mpMainTableView->model()->columnCount();

            for(int columnCouner = 0; columnCouner < columnsSize; ++columnCouner)
            {
                if(false == mpMainTableView->isColumnHidden(columnCouner))
                {
                    firstVisibleColumn = columnCouner;
                    break;
                }
            }

            if(firstVisibleColumn != -1)
            {
                auto jumpIndex = mpMainTableView->model()->index( fileIdx, firstVisibleColumn );

                mpMainTableView->setFocus();
                mpMainTableView->scrollTo( jumpIndex, QAbstractItemView::ScrollHint::PositionAtCenter );
                mpMainTableView->setCurrentIndex(jumpIndex);
            }
        }
    }
}

void CDLTMessageAnalyzer::connectionChanged(bool connected)
{
    mbIsConnected = connected;
}

void CDLTMessageAnalyzer::autoscrollStateChanged(bool state)
{
    mpSearchResultView->setAutoScroll(state);
    if(state)
    {
       mpSearchResultView->scrollToBottom();
    }
}

void CDLTMessageAnalyzer::triggerContinuousAnalysis(bool checked)
{
    if(false == checked)
    {
        if(true == mbIsConnected)
        {
            stop(eStopAction::eCancelIfNotFinished);
        }
        else
        {
            stop(eStopAction::eCancelIfNotFinished);
        }
    }

    getSettingsManager()->setContinuousSearch(checked);
}

void CDLTMessageAnalyzer::triggerNumberOfThreads()
{
    if(nullptr != mpNumberOfThreadsCombobBox)
    {
        getSettingsManager()->setNumberOfThreads( mpNumberOfThreadsCombobBox->currentData().value<int>() );
    }
}

void CDLTMessageAnalyzer::triggerRegexConfig()
{
    if(nullptr != mpRegexSelectionComboBox)
    {
        getSettingsManager()->setSelectedRegexFile(mpRegexSelectionComboBox->currentText());
    }
}

void CDLTMessageAnalyzer::filterPatterns( const QString& filter )
{
    if(nullptr != mpPatternsModel)
    {
        mpPatternsModel->filterPatterns(filter);
    }
}

void CDLTMessageAnalyzer::filterRegexTokens( const QString& filter )
{
    if(nullptr != mpFiltersModel)
    {
        mpFiltersModel->filterRegexTokens(filter);
    }
}

void CDLTMessageAnalyzer::setMainTableView( QTableView* pMainTableView )
{
    mpMainTableView = pMainTableView;
}

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
void CDLTMessageAnalyzer::configurationChanged()
{
    mSearchRange.isFiltered = false; // reset filtering of range
    cancel();
}

void CDLTMessageAnalyzer::setMessageDecoder( QDltMessageDecoder* pMessageDecoder )
{
    mpMessageDecoder = pMessageDecoder;
}
#endif

PUML_PACKAGE_BEGIN(DMA_Plugin_API)
    PUML_CLASS_BEGIN_CHECKED(CDLTMessageAnalyzer)
        PUML_INHERITANCE_CHECKED(IDLTMessageAnalyzerControllerConsumer, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(IGroupedViewModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CPatternsView, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IPatternsModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CFiltersView, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IFiltersModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CUMLView, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IFileWrapper, 1, 1, uses)
        PUML_USE_DEPENDENCY_CHECKED(CBGColorAnimation, 1, 1, uses)
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QDltMessageDecoder, 1, 1, uses)
#else
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QDltPluginManager, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QDltPlugin, 1, many, uses)
#endif
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CRegexDirectoryMonitor, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CTableMemoryJumper, 1, 1, gets and uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CSearchResultView, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ISearchResultModel, 1, 1, gets and uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, gets and uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ICoverageNoteProvider, 1, 1, uses)
        PUML_USE_DEPENDENCY_CHECKED(IDLTMessageAnalyzerController, 1, 1, gets and feeds to IDLTMessageAnalyzerControllerConsumer)
        PUML_USE_DEPENDENCY_CHECKED(CRegexHistoryTextEdit, 1, 1, uses and passes)
    PUML_CLASS_END()
PUML_PACKAGE_END()
