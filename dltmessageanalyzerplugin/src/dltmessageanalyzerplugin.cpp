/**
 * @file    dltmessageanalyzerplugin.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the DLTMessageAnalyzerPlugin class
 */

#include <QtGui>
#include <QMetaObject>
#include "QMenu"
#include "QLineEdit"

#include "dltmessageanalyzerplugin.hpp"
#include "CDLTMessageAnalyzer.hpp"
#include "analyzer/CContinuousAnalyzer.hpp"
#include "analyzer/CMTAnalyzer.hpp"
#include "dltWrappers/CDLTFileWrapper.hpp"
#include "settings/CSettingsManager.hpp"
#include "dltWrappers/CDLTMsgWrapper.hpp"
#include "patternsView/CPatternsView.hpp"
#include "filtersView/CFiltersView.hpp"

Q_DECLARE_METATYPE(tDltMsgWrapperPtr)

DLTMessageAnalyzerPlugin::DLTMessageAnalyzerPlugin():
mpForm(nullptr),
mpDLTMessageAnalyzer(nullptr),
mpMessageAnalyzerController(nullptr),
mpFile(nullptr),
mLastAvailableNumberOfMsg(0),
mConnecitonsMap(),
mConnectionState(QDltConnection::QDltConnectionState::QDltConnectionOffline),
mbAnalysisRunning(false)
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
,mpMainTableView(nullptr)
#endif
{
    //qDebug() << "DLTMessageAnalyzerPlugin lives in thread - " << QThread::currentThreadId();
    qRegisterMetaType<tDltMsgWrapperPtr>("tDLTMsgWrapperPtr");
}

DLTMessageAnalyzerPlugin::~DLTMessageAnalyzerPlugin()
{
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
    mpForm = new Form(this);

    auto pMTController = IDLTMessageAnalyzerController::createInstance<CMTAnalyzer>();
    mpMessageAnalyzerController = IDLTMessageAnalyzerController::createInstance<CContinuousAnalyzer>(pMTController);

    if(nullptr != mpForm->getFiltersView())
    {
        mpForm->getFiltersView()->setRegexInputField(mpForm->getRegexLineEdit());
    }

    mpDLTMessageAnalyzer = IDLTMessageAnalyzerControllerConsumer::createInstance<CDLTMessageAnalyzer>(mpMessageAnalyzerController,
                                                                                                      mpForm->getGroupedResultView(),
                                                                                                      mpForm->getProgresBarLabel(),
                                                                                                      mpForm->getProgresBar(),
                                                                                                      mpForm->getRegexLineEdit(),
                                                                                                      mpForm->getErrorLabel(),
                                                                                                      mpForm->getPatternsTableView(),
                                                                                                      mpForm->getNumberOfThreadsComboBox(),
                                                                                                      mpForm->getSearchResultTableView(),
                                                                                                      mpForm->getContinuousSearchCheckBox(),
                                                                                                      mpForm->getCacheStatusLabel(),
                                                                                                      mpForm->getMainTabWidget(),
                                                                                                      mpForm->getPatternSearchInput(),
                                                                                                      mpForm->getConfigComboBox(),
                                                                                                      mpForm->getFiltersView(),
                                                                                                      mpForm->getFiltersSearchInput(),
                                                                                                      mpForm->getConsoleViewInput(),
                                                                                                      mpForm->getUMLView());

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->setMainTableView(mpMainTableView);
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
    mpFile = std::make_shared<CDLTFileWrapper>(file);

    mpDLTMessageAnalyzer->setFile(mpFile);

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

void DLTMessageAnalyzerPlugin::initMsg(int, QDltMsg &)
{

}

void DLTMessageAnalyzerPlugin::initMsgDecoded(int index, QDltMsg &msg)
{
    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->decodeMsg(msg);
    }

    tDltMsgWrapperPtr pMessageWrapper = std::make_shared<CDLTMsgWrapper>(msg);

    //qDebug() << "initMsgDecoded comes from thread - " << QThread::currentThreadId();
    QMetaObject::invokeMethod(this, "initMsgDecodedForwarded", Qt::QueuedConnection,
                              Q_ARG(int, index),
                              Q_ARG(tDLTMsgWrapperPtr, pMessageWrapper));
}

void DLTMessageAnalyzerPlugin::initMsgDecodedForwarded(int index, tDLTMsgWrapperPtr pMessageWrapper)
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

    mConnecitonsMap[hostname] = connectionState;

    mConnectionState = QDltConnection::QDltConnectionOffline;

    for(const auto& connectionStateItem : mConnecitonsMap)
    {
        switch(connectionStateItem)
        {
            case QDltConnection::QDltConnectionOnline:
            {
                mConnectionState = connectionState;
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

    if( QDltConnection::QDltConnectionOnline == mConnectionState )
    {
        if( mpDLTMessageAnalyzer )
        {
           mpDLTMessageAnalyzer->connectionChanged(true);
        }
    }
    else
    {
        if( mpDLTMessageAnalyzer )
        {
           mpDLTMessageAnalyzer->connectionChanged(false);
           mpDLTMessageAnalyzer->stop(CDLTMessageAnalyzer::eStopAction::eStopIfNotFinished);
        }
    }

    return true;
}

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
void DLTMessageAnalyzerPlugin::initMainTableView(QTableView* pMainTableView)
{
    mpMainTableView = pMainTableView;

    if(nullptr != mpDLTMessageAnalyzer)
    {
        mpDLTMessageAnalyzer->setMainTableView(pMainTableView);
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
        if( false == mpForm->getRegexLineEdit()->text().isEmpty() ) // if search string is non-empty
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
