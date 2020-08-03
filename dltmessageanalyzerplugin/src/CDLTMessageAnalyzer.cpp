/**
 * @file    CDLTMessageAnalyzer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTMessageAnalyzer class
 */

#include "CDLTMessageAnalyzer.hpp"

#include <QtDebug>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include "common/CTreeItem.hpp"
#include "patternsView/CPatternsModel.hpp"
#include <QRegExp>
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

#include "searchView/CSearchResultView.hpp"
#include "groupedView/CGroupedView.hpp"
#include "settings/CSettingsManager.hpp"
#include "dltWrappers/CDLTFileWrapper.hpp"
#include "common/CRegexDirectoryMonitor.hpp"

#include "groupedView/CGroupedViewModel.hpp"
#include "searchView/CSearchResultModel.hpp"
#include "analyzer/IDLTMessageAnalyzerController.hpp"
#include "common/CBGColorAnimation.hpp"
#include "patternsView/CPatternsView.hpp"
#include "filtersView/CFiltersView.hpp"
#include "filtersView/CFiltersModel.hpp"
#include "log/CConsoleCtrl.hpp"
#include "common/CTableMemoryJumper.hpp"
#include "log/CConsoleInputProcessor.hpp"
#include "uml/CUMLView.hpp"

//CDLTMessageAnalyzer
CDLTMessageAnalyzer::CDLTMessageAnalyzer(const std::weak_ptr<IDLTMessageAnalyzerController>& pController,
                                         CGroupedView* pGroupedView, QLabel* pProgressBarLabel, QProgressBar* pProgressBar, QLineEdit* regexLineEdit,
                                         QLabel* pLabel, CPatternsView* pPatternsTableView, QComboBox* pNumberOfThreadsCombobBox,
                                         CSearchResultView* pSearchResultTableView,
                                         QCheckBox* pContinuousSearchCheckBox,
                                         QLabel* pCacheStatusLabel, QTabWidget* pMainTabWidget,
                                         QLineEdit* pPatternsSearchInput, QComboBox* pRegexSelectionComboBox,
                                         CFiltersView* pFiltersView, QLineEdit* pFiltersSearchInput,
                                         QLineEdit* pConsoleViewInput, CUMLView* pUMLView
                                         ):
    IDLTMessageAnalyzerControllerConsumer (pController),
    // default widgets
    mpProgressBarLabel(pProgressBarLabel),
    mpProgressBar(pProgressBar),
    mpRegexLineEdit(regexLineEdit),
    mpLabel(pLabel),
    mpNumberOfThreadsCombobBox(pNumberOfThreadsCombobBox),
    mpMainTableView(nullptr),
    mpContinuousSearchCheckBox(pContinuousSearchCheckBox),
    mpCacheStatusLabel(pCacheStatusLabel),
    mpMainTabWidget(pMainTabWidget),
    mpFiltersSearchInput(pFiltersSearchInput),
    //custom widgets and models
    mpGroupedResultView(pGroupedView),
    mpGroupedViewModel( new CGroupedViewModel() ),
    mpRegexSelectionComboBox(pRegexSelectionComboBox),
    mpSearchResultTableView(pSearchResultTableView),
    mpSearchResultModel( new CSearchResultModel() ),
    mpPatternsTreeView(pPatternsTableView),
    mpAvailablePatternsModel( new CPatternsModel() ),
    mpFiltersView(pFiltersView),
    mpFiltersModel( new CFiltersModel() ),
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
#endif
    mPluginManager(),
    mpRegexDirectoryMonitor(nullptr)
  // timers
  #ifdef DEBUG_BUILD
  , mMeasurementNotificationTimer()
  #endif
  , mMeasurementRequestTimer(),
    mpSearchViewTableJumper(std::make_shared<CTableMemoryJumper>(mpSearchResultTableView)),
    mpConsoleInputProcessor(std::make_shared<CConsoleInputProcessor>(pConsoleViewInput))
{
    //////////////METATYPES_REGISTRATION/////////////////////
    qRegisterMetaType<tRangePtrWrapper>("tRangePtrWrapper");
    qRegisterMetaType<tRange>("tRange");
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

    if( nullptr != mpGroupedResultView &&
            nullptr != mpGroupedViewModel )
    {
        mpGroupedResultView->setModel(mpGroupedViewModel);
    }

    if(nullptr != mpFiltersView &&
            nullptr != mpFiltersModel)
    {
        mpFiltersView->setSpecificModel(mpFiltersModel);

        if(nullptr != mpRegexLineEdit)
        {
            connect( mpRegexLineEdit, &QLineEdit::textChanged, [this](const QString& regex)
            {
                mpFiltersModel->setUsedRegex(regex);
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
            nullptr != mpAvailablePatternsModel &&
            nullptr != mpFiltersModel)
    {
        mpPatternsTreeView->setSpecificModel(mpAvailablePatternsModel);
        mpAvailablePatternsModel->updateView();

        for (int c = 0; c < mpPatternsTreeView->header()->count(); ++c)
        {
            mpPatternsTreeView->header()->setSectionResizeMode(
                        c, QHeaderView::ResizeMode::ResizeToContents);
        }

        mpPatternsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        connect ( mpPatternsTreeView, &CPatternsView::patternSelected, [this](const QString& regexCandidate)
        {
            mpRegexLineEdit->selectAll();
            mpRegexLineEdit->insert( regexCandidate );
            static_cast<void>(analyze());

            if(nullptr != mpFiltersSearchInput)
            {
                mpFiltersSearchInput->setText("");
            }
        });

        connect ( mpFiltersModel, &CFiltersModel::regexUpdatedByUser, [this](const QString& regex)
        {
            mpRegexLineEdit->selectAll();
            mpRegexLineEdit->insert( regex );
            static_cast<void>(analyze());
        });

        connect( mpPatternsTreeView, &CPatternsView::editPatternTriggered, [this]()
        {
            editPattern();
        });

        connect( mpPatternsTreeView, &CPatternsView::deletePatternTriggered, [this]()
        {
            deletePattern();
        });

        connect( mpPatternsTreeView, &CPatternsView::pastePatternTriggered, [this](const CPatternsView::tCopyPastePatternData& copyPastePatternData)
        {
            // if we paste form one file to another
            if(copyPastePatternData.file != CSettingsManager::getInstance()->getSelectedRegexFile())
            {
                // let's paste all the items.
                for( const auto& copyPastePatternItem : copyPastePatternData.items )
                {
                    processOverwritePattern(copyPastePatternItem.alias, copyPastePatternItem.regex);
                }
            }
        });

        connect( mpPatternsTreeView, &CPatternsView::overwriteFromInputFieldTriggered, [this]()
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
        connect ( mpFiltersModel, &CFiltersModel::regexUpdatedByUserInvalid,
        [this](const QModelIndex& index, const QString& error)
        {
            mpFiltersView->highlightInvalidRegex(index);
            updateStatusLabel( error, true );
        });
    }

    if(nullptr != mpRegexLineEdit)
    {
        auto showContextMenu = [this](const QPoint &pos)
        {
            QMenu* pContextMenu = mpRegexLineEdit->createStandardContextMenu();

            pContextMenu->addSeparator();

            {
                QAction* pAction = new QAction("Case sensitive search", this);
                connect(pAction, &QAction::triggered, [](bool checked)
                {
                    CSettingsManager::getInstance()->setCaseSensitiveRegex(checked);
                });
                pAction->setCheckable(true);
                pAction->setChecked(CSettingsManager::getInstance()->getCaseSensitiveRegex());
                pContextMenu->addAction(pAction);
            }

            pContextMenu->addSeparator();

            {
                QAction* pAction = new QAction("Save ...", mpRegexLineEdit);
                connect(pAction, &QAction::triggered, [this]()
                {
                    addPattern( mpRegexLineEdit->text() );
                });

                pContextMenu->addAction(pAction);
            }

            pContextMenu->exec(mpRegexLineEdit->mapToGlobal(pos));
        };

        connect( mpRegexLineEdit, &QWidget::customContextMenuRequested, showContextMenu );
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

    if(nullptr != mpSearchResultTableView &&
            nullptr != mpSearchResultModel)
    {
        mpSearchResultTableView->setModel(mpSearchResultModel);
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
                             [this]()
                    {
                        mSearchRange.isFiltered = false; // reset filtering of range
                        cancel();
                    });
                }
            }
        }
    }
#endif

    connect( qApp, &QApplication::aboutToQuit, []()
    {
        CSettingsManager::getInstance()->storeConfigs();
    });

    connect( CSettingsManager::getInstance().get(), &CSettingsManager::cacheEnabledChanged, [this](bool val)
    {
        if(nullptr != mpFile)
        {
            mpFile->setEnableCache( val );
            cancel();
        }
    });

    connect( CSettingsManager::getInstance().get(), &CSettingsManager::cacheMaxSizeMBChanged, [this](const tCacheSizeMB& val)
    {
        if(nullptr != mpFile)
        {
            mpFile->setMaxCacheSize( MBToB(val) );
            cancel();
        }
    });

    connect( CSettingsManager::getInstance().get(), &CSettingsManager::RDPModeChanged, [this](bool val)
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

    if(nullptr != mpSearchResultTableView)
    {
        connect( mpSearchResultTableView, &CSearchResultView::restartSearch, [this]()
        {
            static_cast<void>(analyze());
        });

        connect( mpSearchResultTableView, &CSearchResultView::searchRangeChanged, [this](const tRangeProperty& searchRange, bool bReset)
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

    connect( CSettingsManager::getInstance().get(), &CSettingsManager::regexMonoHighlightingColorChanged, [this](const QColor&)
    {
        if(nullptr != mpSearchResultModel)
        {
            mpSearchResultModel->updateView();
        }
    });

    connect( CSettingsManager::getInstance().get(), &CSettingsManager::searchResultHighlightingGradientChanged, [this](const tHighlightingGradient&)
    {
        if(false == CSettingsManager::getInstance()->getSearchResultMonoColorHighlighting())
        {
            analyze();
        }
    });

    if(nullptr != mpSearchResultTableView)
    {
        connect(mpSearchResultTableView, &CSearchResultView::clearSearchResultsRequested, [this](){cancel();});
    }

    mpRegexDirectoryMonitor = std::make_shared<CRegexDirectoryMonitor>();

    if(nullptr != mpRegexDirectoryMonitor)
    {
        connect(mpRegexDirectoryMonitor.get(), &CRegexDirectoryMonitor::regexItemSetChanged,
        [this](const CRegexDirectoryMonitor::tFoundRegexItemSet& regexFiles)
        {
            //SEND_MSG(QString("[CDLTMessageAnalyzer]: Fetched selected regex file is - \"%1\"").arg(CSettingsManager::getInstance()->getSelectedRegexFile()));

            if(nullptr != mpRegexSelectionComboBox)
            {
                if(false == regexFiles.empty()) // check on emptiness
                {
                    unsigned int foundSelectedItemIndex = static_cast<unsigned int>(-1);
                    unsigned int counter = 0;
                    QString selectedItemName = CSettingsManager::getInstance()->getSelectedRegexFile();

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
                        CSettingsManager::getInstance()->setSelectedRegexFile(*regexFiles.begin());
                        mpRegexSelectionComboBox->setCurrentIndex(0);
                    }
                }
            }
        });

        mpRegexDirectoryMonitor->setPath(CSettingsManager::getInstance()->getRegexDirectory());
    }

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::selectedRegexFileChanged,
    [this](const QString& /*regexDirectory*/)
    {
        handleLoadedRegexConfig();
    });

    if(nullptr != mpSearchResultTableView)
    {
        connect(mpSearchResultTableView->selectionModel(), &QItemSelectionModel::currentChanged,
        [this](const QModelIndex &current, const QModelIndex &)
        {
            if(nullptr != mpSearchResultTableView)
            {
                auto index = current.sibling(current.row(), static_cast<int>(eSearchResultColumn::Index));
                mpSearchViewTableJumper->setSelectedRow(index.data().value<int>());
            }
        });
    }

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::UML_MaxNumberOfRowsInDiagramChanged, [this](int)
    {
        createSequenceDiagram();
    });

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::UML_ShowArgumentsChanged, [this](bool)
    {
        createSequenceDiagram();
    });

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::UML_WrapOutputChanged, [this](bool)
    {
        createSequenceDiagram();
    });

    handleLoadedConfig();
    handleLoadedRegexConfig();

    // set initial cache status
    mpCacheStatusLabel->setText(CDLTFileWrapper::formCacheStatusString(0,
                                                                       MBToB( CSettingsManager::getInstance()->getCacheMaxSizeMB() ),
                                                                       0,
                                                                       CSettingsManager::getInstance()->getCacheEnabled(),
                                                                       false));
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

void CDLTMessageAnalyzer::handleLoadedRegexConfig()
{
    if(nullptr != mpAvailablePatternsModel)
    {
        mpAvailablePatternsModel->resetData();

        const auto& aliases = CSettingsManager::getInstance()->getAliases();
        for(const auto& alias : aliases)
        {
            mpAvailablePatternsModel->addData(alias.alias, alias.regex, alias.isDefault ? Qt::Checked : Qt::Unchecked );
        }
    }
}

void CDLTMessageAnalyzer::handleLoadedConfig()
{
    if(nullptr != mpNumberOfThreadsCombobBox)
    {
        bool bMatchFound = false;

        const auto& storedNumberOfThreads = CSettingsManager::getInstance()->getNumberOfThreads();

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
                CSettingsManager::getInstance()->getContinuousSearch() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        mpContinuousSearchCheckBox->setCheckState( checkState );
    }
}

CDLTMessageAnalyzer::~CDLTMessageAnalyzer()
{
    //qDebug() << __FUNCTION__;

    if(mpGroupedViewModel)
    {
        delete mpGroupedViewModel;
        mpGroupedViewModel = nullptr;
    }

    if(mpAvailablePatternsModel)
    {
        delete mpAvailablePatternsModel;
        mpAvailablePatternsModel = nullptr;
    }

    if(mpSearchResultModel)
    {
        delete mpSearchResultModel;
        mpSearchResultModel = nullptr;
    }

    if(mpFiltersModel)
    {
        delete mpFiltersModel;
        mpFiltersModel = nullptr;
    }
}

std::shared_ptr<QRegularExpression> CDLTMessageAnalyzer::createRegex( const QString& regex,
                                                                      const QString& onSuccessMessages,
                                                                      const QString& onFailureMessages,
                                                                      bool appendRegexError,
                                                                      QWidget* pErrorAnimationWidget
                                                                      )
{
    //each regex is prefixed with "(?J)" in order to allow duplicated group names
    QString regex_ = QString("(?J)").append(regex);

    auto caseSensitiveOption = CSettingsManager::getInstance()->getCaseSensitiveRegex() ?
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
                error.append(pResult->errorString());
            }

            updateStatusLabel( error, true );

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

    mSearchRange = tRangeProperty();
}

void CDLTMessageAnalyzer::setFile(const tDLTFileWrapperPtr& pFile)
{
    if(nullptr != mpFile)
    {
        mpFile->setSubFilesHandlingStatus(CSettingsManager::getInstance()->getSubFilesHandlingStatus());
        mpFile->resetCache();

        connect( CSettingsManager::getInstance().get(), &CSettingsManager::subFilesHandlingStatusChanged, [this](bool val)
        {
            if(nullptr != mpFile)
            {
                mpFile->setSubFilesHandlingStatus(val);
            }
        });
    }

    if(nullptr != mpSearchResultTableView)
    {
        mpSearchResultTableView->setFile(pFile);
    }

    resetSearchRange();

    mpFile = pFile;

    if(nullptr != mpSearchViewTableJumper)
    {
        mpSearchViewTableJumper->resetSelectedRow();
    }

    if(nullptr != mpFile)
    {
        mpFile->setMaxCacheSize( MBToB( static_cast<unsigned int>(CSettingsManager::getInstance()->getCacheMaxSizeMB()) ) );
        mpFile->setEnableCache( CSettingsManager::getInstance()->getCacheEnabled() );

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        if(nullptr != mpMessageDecoder)
        {
            mpFile->setMessageDecoder(mpMessageDecoder);
        }
#else
        mpFile->setDecoderPlugins(mDecoderPluginsList);
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
                mpCacheStatusLabel->setText(CDLTFileWrapper::formCacheStatusString(0,
                                                                                   MBToB( CSettingsManager::getInstance()->getCacheMaxSizeMB() ),
                                                                                   0,
                                                                                   CSettingsManager::getInstance()->getCacheEnabled(),
                                                                                   false));
            }
        }
    };

    connect( mpFile.get(), &CDLTFileWrapper::isEnabledChanged, [updateCacheStatus](bool)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &CDLTFileWrapper::loadChanged, [updateCacheStatus](unsigned int)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &CDLTFileWrapper::currentSizeMbChanged, [updateCacheStatus](tCacheSizeMB)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &CDLTFileWrapper::maxSizeMbChanged, [updateCacheStatus](tCacheSizeMB)
    {
        updateCacheStatus();
    });

    connect( mpFile.get(), &CDLTFileWrapper::fullChanged, [updateCacheStatus](bool)
    {
        updateCacheStatus();
    });
}

bool CDLTMessageAnalyzer::analyze()
{
    if(!mpFile)
    {
        updateStatusLabel( "Initial enabling error! Caused by limitations of dlt-viewer's plugin API. Please, use \"File->Clear\" or \"Ctrl+K\" to recover", true );
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

    if( nullptr == mpProgressBar ||
            nullptr == mpGroupedViewModel ||
            nullptr == mpNumberOfThreadsCombobBox ||
            nullptr == mpSearchResultModel ||
            nullptr == mpContinuousSearchCheckBox )
    {
        return false;
    }

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    if(nullptr != mpMessageDecoder)
    {
        mpFile->setMessageDecoder(mpMessageDecoder);
    }
#else
    tryLoadDecoderPlugins();
    mpFile->setDecoderPlugins(mDecoderPluginsList);
#endif

    mpFile->setMaxCacheSize( MBToB( static_cast<unsigned int>(CSettingsManager::getInstance()->getCacheMaxSizeMB() ) ) );
    mpFile->setEnableCache( CSettingsManager::getInstance()->getCacheEnabled() );

    tryStop();

    mpSearchResultModel->resetData();
    mpSearchResultModel->setFile(mpFile);

    mpGroupedViewModel->resetData();

    QString regex;

    if( nullptr != mpRegexLineEdit )
    {
        regex = mpRegexLineEdit->text();
    }

    if(0 != regex.size())
    {
        auto pRegex = createRegex( regex, sDefaultStatusText, "Regex error: ", true, mpRegexLineEdit );

        if(nullptr == pRegex || false == pRegex->isValid())
        {
            return false;
        }

        if( nullptr == mpFile ||
                0 == mpFile->size() )
        {
            return false;
        }

        tRange requestedMessages;

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

        auto requestId = requestAnalyze( mpFile,
                                     requestedMessages.from,
                                     requestedMessages.to - requestedMessages.from + 1,
                                     *pRegex,
                                     mpNumberOfThreadsCombobBox->currentData().value<int>(),
                                     isContinuousAnalysis());

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
            nullptr == mpSearchResultModel )
    {
        return;
    }

    mpGroupedViewModel->resetData();
    mpSearchResultModel->resetData();

    updateProgress(0, eRequestState::SUCCESSFUL, true);

    updateStatusLabel(sDefaultStatusText, false);

    tryStop();
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

    if(nullptr != mpRegexLineEdit)
    {
        CBGColorAnimation* pAnimationProxy = new CBGColorAnimation(pAnimationWidget, QPalette::Base);

        QSequentialAnimationGroup* pAnimSeq = new QSequentialAnimationGroup();

        QPropertyAnimation *pColor = new QPropertyAnimation(pAnimationProxy, "color");
        pColor->setDuration(250);
        pColor->setStartValue(QColor(255, 255, 255));
        pColor->setEndValue(QColor(255, 0, 0));

        pAnimSeq->addAnimation(pColor);

        QPropertyAnimation *pUncolor = new QPropertyAnimation(pAnimationProxy, "color");
        pUncolor->setDuration(250);
        pUncolor->setStartValue(QColor(255, 0, 0));
        pUncolor->setEndValue(QColor(255, 255, 255));

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

void CDLTMessageAnalyzer::progressNotification(const tRequestId &requestId,
                                               const eRequestState &requestState,
                                               const int8_t &progress,
                                               const tFoundMatchesPack &processedMatches)
{
    //qDebug() << "CDLTMessageAnalyzer::" << __FUNCTION__;

    //#ifdef DEBUG_BUILD
    //        qDebug() << QString("CDLTMessageAnalyzer::progressNotification: start; request id - %1; was waiting for - %2 ms")
    //                    .arg(requestId)
    //                    .arg(QLocale().toString(mMeasurementNotificationTimer.elapsed()), 4);
    //        mMeasurementNotificationTimer.restart();
    //#endif

    if(mRequestId == requestId)
    {
        if( nullptr == mpGroupedViewModel ||
                nullptr == mpSearchResultModel )
        {
            return;
        }

        //qDebug() << "CDLTMessageAnalyzer::" << __FUNCTION__ << ": progress - " << progress << "; requestStats - " << static_cast<int>(requestState);

        switch(requestState)
        {
            case eRequestState::SUCCESSFUL:
            {
                for(auto foundMatchesIt = processedMatches.matchedItemVec.begin(); foundMatchesIt != processedMatches.matchedItemVec.end(); ++foundMatchesIt)
                {
                    auto foundMatchesCopy = foundMatchesIt->getFoundMatches();
                    auto endIt = processedMatches.matchedItemVec.end();
                    mpGroupedViewModel->addMatches(foundMatchesCopy, foundMatchesIt == --(endIt));
                }

                updateProgress(progress, requestState, false);

                if( false == isContinuousAnalysis() )
                {
                    analysisStatusChanged(false);
                    setReuqestId( INVALID_REQUEST_ID );
                }

                {
                    SEND_MSG(QString("CDLTMessageAnalyzer::progressNotification: request id - %1; overall processing took - %2 ms")
                             .arg(requestId)
                             .arg(QLocale().toString(mMeasurementRequestTimer.elapsed()), 4));

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
                for(auto foundMatchesIt = processedMatches.matchedItemVec.begin();
                    foundMatchesIt != processedMatches.matchedItemVec.end();
                    ++foundMatchesIt)
                {
                    auto foundMatchesCopy = foundMatchesIt->getFoundMatches();
                    auto endIt = processedMatches.matchedItemVec.end();
                    mpGroupedViewModel->addMatches(foundMatchesCopy, foundMatchesIt == --(endIt));
                }

                updateProgress(progress, requestState, false);

                if(progress == 100 && false == mbIsConnected)
                {
                    cancelRequest(requestId);
                    updateProgress(progress, eRequestState::SUCCESSFUL, false);
                    analysisStatusChanged(false);

                    SEND_MSG(QString("CDLTMessageAnalyzer::progressNotification: request id - %1; overall processing took - %2 ms")
                             .arg(requestId)
                             .arg(QLocale().toString(mMeasurementRequestTimer.elapsed()), 4));
                }

                break;
            }
        }

        auto additionResult = mpSearchResultModel->addNextMessageIdxVec( processedMatches );

        if(nullptr != mpSearchViewTableJumper &&
           nullptr != mpSearchResultModel &&
           additionResult.first &&
           false == processedMatches.matchedItemVec.empty())
        {
            if( (additionResult.second.to - additionResult.second.from + 1) == static_cast<int>(processedMatches.matchedItemVec.size()))
            {
                int counter = 0;
                CTableMemoryJumper::tCheckSet checkSet;

                for(const auto& match : processedMatches.matchedItemVec)
                {
                    CTableMemoryJumper::tCheckItem checkItem;
                    checkItem.first = static_cast<CTableMemoryJumper::tRowID>( match.getItemMetadata().msgId );
                    checkItem.second = additionResult.second.from + counter;
                    checkSet.insert(checkItem);
                    ++counter;
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
    if( nullptr == mpAvailablePatternsModel ||
        nullptr == mpPatternsTreeView )
    {
        return;
    }

    bool ok;

    auto pRegex = createRegex( pattern, sDefaultStatusText, "Pattern not saved. Regex error: ", true, mpRegexLineEdit );

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
    if(nullptr != mpAvailablePatternsModel)
    {
        mpAvailablePatternsModel->updatePatternsInPersistency();
    }
}

void CDLTMessageAnalyzer::processOverwritePattern(const QString& alias, const QString checkedRegex, const QModelIndex editItem)
{
    if(nullptr == mpAvailablePatternsModel || nullptr == mpPatternsTreeView)
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
        auto foundPattern = mpAvailablePatternsModel->search(alias);

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
                    auto editedItemIdx = mpAvailablePatternsModel->editData( editItem, alias, checkedRegex, isDefault, isCombine );
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
                auto editedItemIdx = mpAvailablePatternsModel->editData(editItem, alias, checkedRegex, isDefault, isCombine);
                mpPatternsTreeView->scrollTo(editedItemIdx, QTableView::ScrollHint::PositionAtCenter);
                mpPatternsTreeView->setCurrentIndex(editedItemIdx);
            }
            else
            {
                QModelIndex modelIndex = mpAvailablePatternsModel->addData(alias, checkedRegex);
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
    if( nullptr == mpAvailablePatternsModel ||
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
            mpAvailablePatternsModel->removeData(selectedRows[0]);

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
    if( nullptr == mpAvailablePatternsModel ||
            nullptr == mpPatternsTreeView ||
            nullptr == mpRegexLineEdit )
    {
        return;
    }

    auto selectedRows = mpPatternsTreeView->selectionModel()->selectedRows(static_cast<int>(ePatternsColumn::Alias));

    if(1 == selectedRows.size())
    {
        const auto& selectedRow = selectedRows[0];

        QString alias = selectedRow.data().value<QString>();
        QString regex = mpRegexLineEdit->text();
        auto pParsedRegex = createRegex( regex, sDefaultStatusText, "Pattern not updated. Regex error: ", true, mpRegexLineEdit );

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
    if( nullptr == mpAvailablePatternsModel ||
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
            Q_ASSERT(mpAvailablePatternsModel != nullptr);
            aliasLineEdit->setText( mpAvailablePatternsModel->getAliasEditName(selectedRow) );
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
                auto pRegex = createRegex( pRegexLineEdit->text(), sDefaultStatusText, "Pattern not updated. Regex error: ", true, pRegexLineEdit );

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
            auto jumpIndex = mpMainTableView->model()->index( fileIdx, index.column() );
            mpMainTableView->setFocus();
            mpMainTableView->scrollTo( jumpIndex, QAbstractItemView::ScrollHint::PositionAtCenter );
            mpMainTableView->setCurrentIndex(jumpIndex);
        }
    }
}

void CDLTMessageAnalyzer::connectionChanged(bool connected)
{
    mbIsConnected = connected;
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

    CSettingsManager::getInstance()->setContinuousSearch(checked);
}

void CDLTMessageAnalyzer::triggerNumberOfThreads()
{
    if(nullptr != mpNumberOfThreadsCombobBox)
    {
        CSettingsManager::getInstance()->setNumberOfThreads( mpNumberOfThreadsCombobBox->currentData().value<int>() );
    }
}

void CDLTMessageAnalyzer::triggerRegexConfig()
{
    if(nullptr != mpRegexSelectionComboBox)
    {
        CSettingsManager::getInstance()->setSelectedRegexFile(mpRegexSelectionComboBox->currentText());
    }
}

void CDLTMessageAnalyzer::filterPatterns( const QString& filter )
{
    if(nullptr != mpAvailablePatternsModel)
    {
        mpAvailablePatternsModel->filterPatterns(filter);
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
