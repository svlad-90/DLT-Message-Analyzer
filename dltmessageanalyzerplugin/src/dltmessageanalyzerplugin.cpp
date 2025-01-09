/**
 * @file    dltmessageanalyzerplugin.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the DLTMessageAnalyzerPlugin class
 */

#include <QtGui>
#include <QMetaObject>
#include "QMenu"
#include "QLineEdit"
#include "QApplication"

#include "qcustomplot.h"

#include "dltmessageanalyzerplugin.hpp"
#include "plugin/api/CDLTMessageAnalyzer.hpp"
#include "components/patternsView/api/CPatternsView.hpp"

#include "DMA_Plantuml.hpp"
#include "components/log/api/CLog.hpp"

#include "plugin/api/form.h"

#include "components/analyzer/api/CAnalyzerComponent.hpp"
#include "components/log/api/CLogComponent.hpp"
#include "components/searchView/api/CSearchViewComponent.hpp"
#include "components/groupedView/api/CGroupedViewComponent.hpp"
#include "components/patternsView/api/CPatternsViewComponent.hpp"
#include "components/filtersView/api/CFiltersViewComponent.hpp"
#include "components/plant_uml/api/CUMLViewComponent.hpp"
#include "components/plotView/api/CPlotViewComponent.hpp"
#include "components/logo/api/CLogoComponent.hpp"
#include "components/logsWrapper/api/CLogsWrapperComponent.hpp"
#include "components/settings/api/CSettingsComponent.hpp"

#include "components/logsWrapper/api/IFileWrapper.hpp"
#include "components/logsWrapper/api/IMsgWrapper.hpp"

#include "components/filtersView/api/CFiltersView.hpp"
#include "components/regexHistory/api/CRegexHistoryComponent.hpp"

#include "components/searchView/api/ISearchResultModel.hpp"
#include "components/groupedView/api/CGroupedView.hpp"
#include "components/searchView/api/CSearchResultView.hpp"
#include "components/groupedView/api/IGroupedViewModel.hpp"
#include "components/coverageNote/api/CCoverageNoteComponent.hpp"

#include "DMA_Plantuml.hpp"

Q_DECLARE_METATYPE(tMsgWrapperPtr)

DLTMessageAnalyzerPlugin::DLTMessageAnalyzerPlugin():
mpForm(nullptr),
mpDLTMessageAnalyzer(nullptr),
mpFile(nullptr),
mLastAvailableNumberOfMsg(0),
mConnecitonsMap(),
mConnectionState(QDltConnection::QDltConnectionState::QDltConnectionOffline),
mbAnalysisRunning(false),
mComponents(),
mpSearchViewComponent(nullptr),
mpGroupedViewComponent(nullptr),
mpPatternsViewComponent(nullptr),
mpFiltersViewComponent(nullptr),
mpUMLViewComponent(nullptr),
mpLogoComponent(nullptr),
mpLogsWrapperComponent(nullptr),
mpRegexHistoryComponent(nullptr),
mpCoverageNoteComponent(nullptr),
mpSettingsComponent(nullptr),
mpAnalyzerComponent(nullptr),
mDisconnectionTimer()
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
,mpMainTableView(nullptr)
#endif
{
    //qDebug() << "DLTMessageAnalyzerPlugin lives in thread - " << QThread::currentThreadId();
    qRegisterMetaType<tMsgWrapperPtr>("tMsgWrapperPtr");

    DMA::PlantUML::Creator::getInstance().initialize();
    DMA::PlantUML::Creator::getInstance().setBackgroundColor("#FEFEFE");

    connect(&mDisconnectionTimer, &QTimer::timeout, [this]()
    {
        if( mpDLTMessageAnalyzer )
        {
            mpDLTMessageAnalyzer->connectionChanged(false);
            if(nullptr != mpSettingsComponent->getSettingsManager() &&
                true == mpSettingsComponent->getSettingsManager()->getContinuousSearch())
            {
                mpDLTMessageAnalyzer->stop(CDLTMessageAnalyzer::eStopAction::eStopIfNotFinished);
            }
        }

        mDisconnectionTimer.stop();
    });
}

QString DLTMessageAnalyzerPlugin::name()
{
    return QString("DLTMessageAnalyzerPlugin");
}

QString DLTMessageAnalyzerPlugin::pluginVersion()
{
    return DLT_MESSAGE_ANALYZER_PLUGIN_VERSION;
}

QString DLTMessageAnalyzerPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DLTMessageAnalyzerPlugin::pluginData() const
{
    return QString("You are using %1 | Version: v.%2 | Author - %3")
            .arg(DLT_MESSAGE_ANALYZER_NAME)
            .arg(DLT_MESSAGE_ANALYZER_PLUGIN_VERSION)
            .arg(DLT_MESSAGE_ANALYZER_PLUGIN_AUTHOR);
}

QString DLTMessageAnalyzerPlugin::description()
{
    return QString();
}

QString DLTMessageAnalyzerPlugin::error()
{
    return QString();
}

bool DLTMessageAnalyzerPlugin::loadConfig(QString /*filename*/)
{
    return true;
}

bool DLTMessageAnalyzerPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DLTMessageAnalyzerPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DLTMessageAnalyzerPlugin::initViewer()
{    
    {
        auto pSettingsComponent = std::make_shared<CSettingsComponent>();
        mpSettingsComponent = pSettingsComponent;

        auto initResult = pSettingsComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pSettingsComponent->getName()));
        }

        mComponents.push_back(pSettingsComponent);
    }

    // it also uses the settings manager, thus we create the form after the
    // settings component
    mpForm = new Form(this, mpSettingsComponent->getSettingsManager());

    std::shared_ptr<IDLTMessageAnalyzerController> pAnalyzerController = nullptr;

    {
        auto pAnalyzerComponent = std::make_shared<CAnalyzerComponent>(mpSettingsComponent->getSettingsManager());

        mpAnalyzerComponent = pAnalyzerComponent;

        auto initResult = pAnalyzerComponent->startInit();

        if(true == initResult.bIsOperationSuccessful)
        {
            pAnalyzerController = pAnalyzerComponent->getAnalyzerController();
        }
        else
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pAnalyzerComponent->getName()));
        }

        mComponents.push_back(pAnalyzerComponent);
    }

    {
        auto pLogComponent = std::make_shared<CLogComponent>(mpForm->getConsoleViewInput(),
                                                             mpForm->getMainTabWidget(),
                                                             mpForm->getConsoleViewTab(),
                                                             mpForm->getConsoleView(),
                                                             mpSettingsComponent->getSettingsManager());

        auto initResult = pLogComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pLogComponent->getName()));
        }

        mComponents.push_back(pLogComponent);
    }

    {
        auto pCoverageNoteComponent = std::make_shared<CCoverageNoteComponent>(mpSettingsComponent->getSettingsManager(),
                                                                               mpForm->getCNCommentTextEdit(),
                                                                               mpForm->getCNItemsTableView(),
                                                                               mpForm->getCNMessageTextEdit(),
                                                                               mpForm->getCNRegexTextEdit(),
                                                                               mpForm->getCNUseRegexButton(),
                                                                               mpForm->getCNCurrentFileLineEdit(),
                                                                               mpForm->getFilesLineEdit());
        mpCoverageNoteComponent = pCoverageNoteComponent;

        auto initResult = pCoverageNoteComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pCoverageNoteComponent->getName()));
        }

        mComponents.push_back(pCoverageNoteComponent);
    }

    {
        auto pSearchViewComponent = std::make_shared<CSearchViewComponent>(mpForm->getMainTabWidget(),
                                                                           mpForm->getSearchResultTableView(),
                                                                           mpSettingsComponent->getSettingsManager(),
                                                                           mpCoverageNoteComponent->getCoverageNoteProviderPtr());
        mpSearchViewComponent = pSearchViewComponent;

        auto initResult = pSearchViewComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pSearchViewComponent->getName()));
        }

        mComponents.push_back(pSearchViewComponent);
    }

    {
        auto pGroupedViewComponent = std::make_shared<CGroupedViewComponent>(mpForm->getGroupedResultView(),
                                                                             mpSettingsComponent->getSettingsManager());
        mpGroupedViewComponent = pGroupedViewComponent;

        auto initResult = pGroupedViewComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pGroupedViewComponent->getName()));
        }

        mComponents.push_back(pGroupedViewComponent);
    }

    {
        auto pPatternsViewComponent = std::make_shared<CPatternsViewComponent>(mpForm->getPatternsTableView(),
                                                                               mpSettingsComponent->getSettingsManager());
        mpPatternsViewComponent = pPatternsViewComponent;

        auto initResult = pPatternsViewComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pPatternsViewComponent->getName()));
        }

        mComponents.push_back(pPatternsViewComponent);
    }

    {
        auto pFiltersViewComponent = std::make_shared<CFiltersViewComponent>(mpForm->getFiltersView(),
                                                                             mpSettingsComponent->getSettingsManager());
        mpFiltersViewComponent = pFiltersViewComponent;

        auto initResult = pFiltersViewComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pFiltersViewComponent->getName()));
        }

        mComponents.push_back(pFiltersViewComponent);
    }

    {
        auto pUMLViewComponent = std::make_shared<CUMLViewComponent>(mpForm->getUMLView(),
                                                                     mpSettingsComponent->getSettingsManager(),
                                                                     mpForm->getUMLCreateDiagramFromTextButton(),
                                                                     mpForm->getUMLTextEditor());
        mpUMLViewComponent = pUMLViewComponent;

        auto initResult = pUMLViewComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pUMLViewComponent->getName()));
        }

        mComponents.push_back(pUMLViewComponent);
    }

    {
        auto pPlotViewComponent = std::make_shared<CPlotViewComponent>(mpForm->getCustomPlot(),
                                                                       mpForm->getCreatePlotButton(),
                                                                       mpSearchViewComponent->getSearchResultModel(),
                                                                       mpSettingsComponent->getSettingsManager());
        mpPlotViewComponent = pPlotViewComponent;

        auto initResult = pPlotViewComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pPlotViewComponent->getName()));
        }
        else
        {
            connect(pPlotViewComponent.get(), &CPlotViewComponent::messageIdSelected, this, [this](const tMsgId& msgId)
            {
                if(nullptr != mpDLTMessageAnalyzer)
                {
                    mpDLTMessageAnalyzer->jumpInMainTable(msgId);
                }
            });
        }

        mComponents.push_back(pPlotViewComponent);
    }

    {
        auto pLogoComponent = std::make_shared<CLogoComponent>(mpForm->getLogo());
        mpLogoComponent = pLogoComponent;

        auto initResult = pLogoComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pLogoComponent->getName()));
        }

        mComponents.push_back(pLogoComponent);
    }

    {
        auto pLogsWrapperComponent = std::make_shared<CLogsWrapperComponent>();
        mpLogsWrapperComponent = pLogsWrapperComponent;

        auto initResult = pLogsWrapperComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pLogsWrapperComponent->getName()));
        }

        mComponents.push_back(pLogsWrapperComponent);
    }

    {
        auto pRegexHistoryComponent = std::make_shared<CRegexHistoryComponent>(mpSettingsComponent->getSettingsManager(),
                                                                               mpForm->getRegexTextEdit(),
                                                                               mpPatternsViewComponent->getPatternsView(),
                                                                               mpAnalyzerComponent->getAnalyzerController());
        mpRegexHistoryComponent = pRegexHistoryComponent;

        auto initResult = pRegexHistoryComponent->startInit();

        if(false == initResult.bIsOperationSuccessful)
        {
            SEND_ERR(QString("Failed to initialize %1").arg(pRegexHistoryComponent->getName()));
        }

        mComponents.push_back(pRegexHistoryComponent);
    }

    connect(mpGroupedViewComponent->getGroupedView(), &CGroupedView::searchViewHighlightingRequested,
            this, [this](const tMsgIdSet& msgs)
    {
        auto pSearchModel = mpSearchViewComponent->getSearchResultModel();
        auto* pSearchView = mpSearchViewComponent->getSearchResultView();

        if(false == msgs.empty() &&
           nullptr != mpSearchViewComponent->getSearchResultModel() &&
           nullptr != mpSearchViewComponent->getSearchResultView() &&
           nullptr != mpGroupedViewComponent->getGroupedView())
        {
            pSearchModel->setHighlightedRows(msgs);
            auto jumpRow = pSearchModel->getRowByMsgId(*msgs.begin());

            if(jumpRow >= 0)
            {
                pSearchView->clearSelection();
                pSearchView->selectRow(jumpRow);

                auto selectedRows = pSearchView->selectionModel()->selectedRows();

                if(false == selectedRows.empty())
                {
                    pSearchView->scrollTo(selectedRows[0]);
                }

                if(nullptr != mpForm)
                {
                    auto* pMainWidget = mpForm->getMainTabWidget();

                    if(nullptr != pMainWidget)
                    {
                        // switch to search view.
                        pMainWidget->setCurrentIndex(0);
                    }
                }
            }
        }
    });

    connect( qApp, &QApplication::aboutToQuit, [this]()
    {
        for(auto& pComponent : mComponents)
        {
            if(nullptr != pComponent)
            {
                auto shutdownResult = pComponent->startShutdown();

                if(false == shutdownResult.bIsOperationSuccessful)
                {
                    SEND_ERR(QString("Failed to shutdown %1").arg(pComponent->getName()));
                }
            }
        }

        mComponents.clear();

        if(nullptr != mpSearchViewComponent)
        {
            mpSearchViewComponent.reset();
        }

        if(nullptr != mpGroupedViewComponent)
        {
            mpGroupedViewComponent.reset();
        }

        if(nullptr != mpPatternsViewComponent)
        {
            mpPatternsViewComponent.reset();
        }
    });

    if(nullptr != mpForm->getFiltersView())
    {
        mpForm->getFiltersView()->setRegexInputField(mpForm->getRegexTextEdit());
    }

    mpDLTMessageAnalyzer = IDLTMessageAnalyzerControllerConsumer::createInstance<CDLTMessageAnalyzer>(pAnalyzerController,
                                                                                                      mpGroupedViewComponent->getGroupedViewModel(),
                                                                                                      mpForm->getProgresBarLabel(),
                                                                                                      mpForm->getProgresBar(),
                                                                                                      mpForm->getRegexTextEdit(),
                                                                                                      mpForm->getErrorLabel(),
                                                                                                      mpPatternsViewComponent->getPatternsView(),
                                                                                                      mpPatternsViewComponent->getPatternsModel(),
                                                                                                      mpForm->getNumberOfThreadsComboBox(),
                                                                                                      mpForm->getContinuousSearchCheckBox(),
                                                                                                      mpForm->getCacheStatusLabel(),
                                                                                                      mpForm->getMainTabWidget(),
                                                                                                      mpForm->getPatternSearchInput(),
                                                                                                      mpForm->getConfigComboBox(),
                                                                                                      mpFiltersViewComponent->getFiltersView(),
                                                                                                      mpFiltersViewComponent->getFiltersModel(),
                                                                                                      mpForm->getFiltersSearchInput(),
                                                                                                      mpUMLViewComponent->getUMLView(),
                                                                                                      mpSearchViewComponent->getTableMemoryJumper(),
                                                                                                      mpSearchViewComponent->getSearchResultView(),
                                                                                                      mpSearchViewComponent->getSearchResultModel(),
                                                                                                      mpLogsWrapperComponent,
                                                                                                      mpSettingsComponent->getSettingsManager(),
                                                                                                      mpPlotViewComponent->getPlot(),
                                                                                                      mpCoverageNoteComponent->getCoverageNoteProviderPtr());

    connect(mpForm->getRegexTextEdit(), &CRegexHistoryTextEdit::returnPressed,
            this, [this]()
    {
        if(nullptr != mpDLTMessageAnalyzer)
        {
            if(nullptr != mpForm)
            {
                auto pRegexLineEdit = mpForm->getRegexTextEdit();
                if(nullptr != pRegexLineEdit && false == pRegexLineEdit->getIgnoreReturnKeyEvent() )
                {
                    mpDLTMessageAnalyzer->analyze();
                }
            }
        }
    });

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->setMainTableView(mpMainTableView);
    }

    if(nullptr != mpForm->getSearchResultTableView())
    {
        mpForm->getSearchResultTableView()->setMainTableView(mpMainTableView);
    }

    if(nullptr != mpCoverageNoteComponent)
    {
        mpCoverageNoteComponent->setMainTableView(mpMainTableView);
    }
#endif

    connect( this, &DLTMessageAnalyzerPlugin::analysisWithEmptyStringRequested, mpForm->getPatternsTableView(), &CPatternsView::applyPatternsCombination );

    auto pAnalyzeButton = mpForm->getAnalyzeButton();

    if(nullptr != mpDLTMessageAnalyzer &&
       nullptr != pAnalyzeButton)
    {
        auto analysisStatusChanged = [this, pAnalyzeButton](bool analysisRunning)
        {
            mbAnalysisRunning = analysisRunning;

            if(true == mbAnalysisRunning)
            {
                pAnalyzeButton->setText("Stop");
            }
            else
            {
                pAnalyzeButton->setText("Analyze");
            }
        };

        connect( mpDLTMessageAnalyzer.get(), &CDLTMessageAnalyzer::analysisStatusChanged, analysisStatusChanged );
    }

    if(nullptr != mpDLTMessageAnalyzer)
    {
        connect(mpDLTMessageAnalyzer.get(), &CDLTMessageAnalyzer::analysisStatusChanged, [this](bool analysisRunning)
        {
            if( true == analysisRunning && mpDLTMessageAnalyzer && mpFile )
            {
                switchFromFileView();
            }

            if( false == analysisRunning )
            {
                auto* pMainWidget = mpForm->getMainTabWidget();

                if(nullptr != pMainWidget)
                {
                    const auto currentIndex = pMainWidget->currentIndex();
                    if(2 == currentIndex)
                    {
                        if(mpGroupedViewComponent->getGroupedViewModel())
                        {
                            mpGroupedViewComponent->getGroupedViewModel()->sortByCurrentSortingColumn();
                        }
                    }
                }
            }
        });
    }

    if(mpCoverageNoteComponent->getCoverageNoteProviderPtr() &&
       mpSearchViewComponent->getSearchResultView())
    {
        connect(mpCoverageNoteComponent->getCoverageNoteProviderPtr().get(),
                &ICoverageNoteProvider::addCommentFromMainTableRequested,
                this, [this]()
        {
            if(mpSearchViewComponent)
            {
                mpSearchViewComponent->getSearchResultView()->addCommentFromMainTable();
            }
        });
    }

    return mpForm;
}

void DLTMessageAnalyzerPlugin::selectedIdxMsg(int, QDltMsg &/*msg*/) {
}

void DLTMessageAnalyzerPlugin::selectedIdxMsgDecoded(int, QDltMsg &/*msg*/){

}

void DLTMessageAnalyzerPlugin::initFileStart(QDltFile *file)
{
    if(nullptr != mpLogsWrapperComponent)
    {
        mpFile = mpLogsWrapperComponent->createDLTFileWrapper(file);

        if( mpDLTMessageAnalyzer )
        {
           mpDLTMessageAnalyzer->setFile(mpFile);
           mpDLTMessageAnalyzer->cancel();
        }

        auto* pFilesLineEdit = mpForm->getFilesLineEdit();

        if(nullptr != pFilesLineEdit)
        {
            if(nullptr != mpFile)
            {
                QString text;

                const int iSize = mpFile->getNumberOfFiles();

                if(iSize > 0)
                {
                    pFilesLineEdit->show();

                    for(int i = 0; i < iSize; ++i)
                    {
                        text.append(mpFile->getFileName(i)).append("\n");
                    }

                    pFilesLineEdit->setText(text);
                }
                else
                {
                    pFilesLineEdit->hide();
                }
            }
            else
            {
                pFilesLineEdit->hide();
            }
        }

        //mpForm->setEnabled(false);
    }
}

void DLTMessageAnalyzerPlugin::initMsg(int, QDltMsg &)
{

}

void DLTMessageAnalyzerPlugin::initMsgDecoded(int index, QDltMsg &msg)
{
    if(nullptr != mpLogsWrapperComponent)
    {
        if(nullptr != mpDLTMessageAnalyzer)
        {
            mpDLTMessageAnalyzer->decodeMsg(msg);
        }

        tMsgWrapperPtr pMessageWrapper = mpLogsWrapperComponent->createDLTMsgWrapper(msg);

        //qDebug() << "initMsgDecoded comes from thread - " << QThread::currentThreadId();
        QMetaObject::invokeMethod(this, "initMsgDecodedForwarded", Qt::QueuedConnection,
                                  Q_ARG(int, index),
                                  Q_ARG(tMsgWrapperPtr, pMessageWrapper));
    }
}

void DLTMessageAnalyzerPlugin::initMsgDecodedForwarded(int index, tMsgWrapperPtr pMessageWrapper)
{
    if(nullptr != mpFile)
    {
        mpFile->cacheMsgWrapper(index, pMessageWrapper);
    }
}

void DLTMessageAnalyzerPlugin::initFileFinish()
{
    //mpForm->setEnabled(true);
}

bool DLTMessageAnalyzerPlugin::stateChanged(int, QDltConnection::QDltConnectionState connectionState,QString hostname)
{

    auto foundElement = mConnecitonsMap.find(hostname);

    bool bCheck = false;

    if(foundElement != mConnecitonsMap.end()) // if element is already known
    {
        // and it's status has changed from or to Online
        if(foundElement.value() != connectionState &&
           (foundElement.value() == QDltConnection::QDltConnectionOnline
         || connectionState == QDltConnection::QDltConnectionOnline) )
        {
            bCheck = true; // we should not further check anything
        }
    }
    else // if element is not known
    {
        // then we should check
        bCheck = true;
    }

    if(true == bCheck) // if something has changed
    {
        mConnecitonsMap[hostname] = connectionState;

        mConnectionState = QDltConnection::QDltConnectionOffline;

        for(const auto& connectionStateItem : mConnecitonsMap)
        {
            switch(connectionStateItem)
            {
                case QDltConnection::QDltConnectionOnline:
                {
                    mConnectionState = connectionStateItem;
                }
                    break;
                default:
                {
                    // do nothing
                }
                    break;
            }

            if( QDltConnection::QDltConnectionOnline == mConnectionState )
            {
                break;
            }
        }

        switch(mConnectionState)
        {
            case QDltConnection::QDltConnectionOnline:
            {
            if( mpDLTMessageAnalyzer )
                {
                    mpDLTMessageAnalyzer->connectionChanged(true);
                }

                mDisconnectionTimer.stop();
            }
            break;
            case QDltConnection::QDltConnectionOffline:
            {
                mDisconnectionTimer.start(500);
            }
            break;
            default:
            {
                // do nothing
            }
            break;
        }
    }

    return true;
}

bool DLTMessageAnalyzerPlugin::autoscrollStateChanged(bool autoScoll)
{
    if( mpDLTMessageAnalyzer )
    {
       mpDLTMessageAnalyzer->autoscrollStateChanged(autoScoll);
    }
    return true;
}

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
void DLTMessageAnalyzerPlugin::initMainTableView(QTableView* pMainTableView)
{
    mpMainTableView = pMainTableView;

    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->setMainTableView(mpMainTableView);
    }
}

void DLTMessageAnalyzerPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->setMessageDecoder(pMessageDecoder);
    }
}

void DLTMessageAnalyzerPlugin::configurationChanged()
{
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->configurationChanged();
    }
}
#endif

void DLTMessageAnalyzerPlugin::updateFileStart()
{

}

void DLTMessageAnalyzerPlugin::updateMsg(int, QDltMsg &)
{

}

void DLTMessageAnalyzerPlugin::updateMsgDecoded(int, QDltMsg &)
{

}

void DLTMessageAnalyzerPlugin::updateFileFinish()
{

}

void DLTMessageAnalyzerPlugin::analyze()
{
    if( mpDLTMessageAnalyzer && mpForm )
    {
        if( false == mpForm->getRegexTextEdit()->toPlainText().isEmpty() ) // if search string is non-empty
        {
            bool bRunning = mpDLTMessageAnalyzer->analyze();

            if(true == bRunning)
            {
                switchFromFileView();
            }
        }
        else // otherwise
        {
            analysisWithEmptyStringRequested(); // let's try to apply combination of patterns
        }
    }
}

void DLTMessageAnalyzerPlugin::switchFromFileView()
{
    auto* pMainWidget = mpForm->getMainTabWidget();

    if(nullptr != pMainWidget)
    {
        const auto currentIndex = pMainWidget->currentIndex();
        if(2 == currentIndex)
        {
            pMainWidget->setCurrentIndex(0);
        }
    }
}

void DLTMessageAnalyzerPlugin::filterPatterns( const QString& filter )
{
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->filterPatterns( filter );
    }
}

void DLTMessageAnalyzerPlugin::filterRegexTokens( const QString& filter )
{
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->filterRegexTokens( filter );
    }
}

void DLTMessageAnalyzerPlugin::createSequenceDiagram()
{
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->createSequenceDiagram();
    }
}

void DLTMessageAnalyzerPlugin::resetCache()
{
    if(nullptr != mpFile)
    {
        mpFile->resetCache();
    }
}

void DLTMessageAnalyzerPlugin::stop()
{
    if( mpDLTMessageAnalyzer )
    {
       mpDLTMessageAnalyzer->stop(CDLTMessageAnalyzer::eStopAction::eStopIfNotFinished);
    }
}

void DLTMessageAnalyzerPlugin::addPattern(const QString& pattern)
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->addPattern(pattern);
    }
}

void DLTMessageAnalyzerPlugin::deletePattern()
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->deletePattern();
    }
}

void DLTMessageAnalyzerPlugin::editPattern()
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->editPattern();
    }
}

void DLTMessageAnalyzerPlugin::exportToHTML()
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->exportGroupedViewToHTML();
    }
}

void DLTMessageAnalyzerPlugin::searchView_clicked_jumpTo_inMainTable(const QModelIndex &index)
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->searchView_clicked_jumpTo_inMainTable(index);
    }
}

void DLTMessageAnalyzerPlugin::triggerContinuousAnalysis(bool checked)
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->triggerContinuousAnalysis(checked);
    }
}

void DLTMessageAnalyzerPlugin::triggerNumberOfThreads()
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->triggerNumberOfThreads();
    }
}

void DLTMessageAnalyzerPlugin::triggerRegexConfig()
{
    if( mpDLTMessageAnalyzer )
    {
        mpDLTMessageAnalyzer->triggerRegexConfig();
    }
}

void DLTMessageAnalyzerPlugin::analysisTrigger()
{
    if(false == mbAnalysisRunning)
    {
        analyze();
    }
    else
    {
        stop();
    }
}

#ifndef QT5
Q_EXPORT_PLUGIN2(dltmessageanalyzerplugin, DLTMessageAnalyzerPlugin);
#endif

PUML_PACKAGE_BEGIN(DMA_Plugin)
    PUML_CLASS_BEGIN_CHECKED(DLTMessageAnalyzerPlugin)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_INHERITANCE_CHECKED(QDLTPluginInterface, implements)
        PUML_INHERITANCE_CHECKED(QDltPluginViewerInterface, implements)
        PUML_INHERITANCE_CHECKED(QDltPluginControlInterface, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(Form, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CDLTMessageAnalyzer, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CAnalyzerComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CLogComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CGroupedViewComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CSearchViewComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CFiltersViewComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CPatternsViewComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CUMLViewComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CLogoComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CLogsWrapperComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CPlotViewComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CRegexHistoryComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CCoverageNoteComponent, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QTimer, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
