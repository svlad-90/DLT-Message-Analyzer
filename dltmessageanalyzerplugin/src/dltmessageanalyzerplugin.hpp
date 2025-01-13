/**
 * @file    dltmessageanalyzerplugin.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the DLTMessageAnalyzerPlugin class
 */

#pragma once

#include "memory"

#include <QObject>
#include <QTimer>

#include "plugininterface.h"

#include "common/Definitions.hpp"

#define DLT_MESSAGE_ANALYZER_NAME "DLT-Message-Analyzer"
#define DLT_MESSAGE_ANALYZER_PLUGIN_VERSION "1.0.29"
#define DLT_MESSAGE_ANALYZER_PLUGIN_AUTHOR "Vladyslav Goncharuk <svlad1990@gmail.com>"

class CDLTMessageAnalyzer;
typedef std::shared_ptr<CDLTMessageAnalyzer> tDLTMessageAnalyzerPtr;
typedef std::shared_ptr<QString> tQStringPtr;
typedef QMap<int, tQStringPtr > tMsgMap;
class IDLTMessageAnalyzerController;
class CSearchViewComponent;
class CGroupedViewComponent;
class CPatternsViewComponent;
class CFiltersViewComponent;
class CUMLViewComponent;
class CPlotViewComponent;
class CLogoComponent;
class CLogsWrapperComponent;
class CRegexHistoryComponent;
class CSettingsComponent;
class CAnalyzerComponent;
class CCoverageNoteComponent;
class Form;

namespace DMA
{
    class IComponent;
}

class DLTMessageAnalyzerPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DLTMessageAnalyzerPlugin")
#endif

public:
    DLTMessageAnalyzerPlugin();

    //general trigger
    void analysisTrigger();
    void analyze();
    void stop();

    // grouped view triggers
    void exportToHTML();

    // patterns triggers
    void addPattern(const QString& pattern);
    void deletePattern();
    void editPattern();

    // search view triggers
    void searchView_clicked_jumpTo_inMainTable(const QModelIndex &index);

    void triggerContinuousAnalysis(bool checked);
    void triggerNumberOfThreads();
    void triggerRegexConfig();

    void resetCache();

    QString pluginData() const;

    void filterPatterns( const QString& filter );
    void filterRegexTokens( const QString& filter );
    void createSequenceDiagram();

signals:
    void analysisWithEmptyStringRequested();

public slots:
    void initMsgDecodedForwarded(int index, tMsgWrapperPtr pMsg);

private: // methods

    bool initControl(QDltControl *) final override { return true; }
    bool initConnections(QStringList) final override { return true; }
    bool controlMsg(int, QDltMsg &) final override { return true; }
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname) final override;
    bool autoscrollStateChanged(bool) final override;

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    void initMainTableView(QTableView* pMainTableView) override;
    void initMessageDecoder(QDltMessageDecoder* pMessageDecoder) override;
    void configurationChanged() override;
#endif

    void switchFromFileView();

    /* QDLTPluginInterface interface */
    QString name() final override;
    QString pluginVersion() final override;
    QString pluginInterfaceVersion() final override;
    QString description() final override;
    QString error() final override;
    bool loadConfig(QString filename) final override;
    bool saveConfig(QString filename) final override;
    QStringList infoConfig() final override;

    /* QDltPluginViewerInterface */
    QWidget* initViewer() final override;
    void initFileStart(QDltFile *file) final override;
    void initFileFinish() final override;
    void initMsg(int index, QDltMsg &msg) final override;
    void initMsgDecoded(int index, QDltMsg &msg) final override;
    void updateFileStart() final override;
    void updateMsg(int index, QDltMsg &msg) final override;
    void updateMsgDecoded(int index, QDltMsg &msg) final override;
    void updateFileFinish() final override;
    void selectedIdxMsg(int index, QDltMsg &msg) final override;
    void selectedIdxMsgDecoded(int index, QDltMsg &msg) final override;

private: // members

    /* internal variables */
    Form *mpForm;
    tDLTMessageAnalyzerPtr mpDLTMessageAnalyzer;
    tFileWrapperPtr mpFile;
    int mLastAvailableNumberOfMsg;
    QMap<QString, QDltConnection::QDltConnectionState> mConnecitonsMap;
    QDltConnection::QDltConnectionState mConnectionState;
    bool mbAnalysisRunning;

    typedef std::shared_ptr<DMA::IComponent> tComponentPtr;
    typedef std::vector<tComponentPtr> tComponentPtrVec;

    tComponentPtrVec mComponents;

    std::shared_ptr<CSearchViewComponent> mpSearchViewComponent;
    std::shared_ptr<CGroupedViewComponent> mpGroupedViewComponent;
    std::shared_ptr<CPatternsViewComponent> mpPatternsViewComponent;
    std::shared_ptr<CFiltersViewComponent> mpFiltersViewComponent;
    std::shared_ptr<CUMLViewComponent> mpUMLViewComponent;
    std::shared_ptr<CPlotViewComponent> mpPlotViewComponent;
    std::shared_ptr<CLogoComponent> mpLogoComponent;
    std::shared_ptr<CLogsWrapperComponent> mpLogsWrapperComponent;
    std::shared_ptr<CRegexHistoryComponent> mpRegexHistoryComponent;
    std::shared_ptr<CCoverageNoteComponent> mpCoverageNoteComponent;
    std::shared_ptr<CSettingsComponent> mpSettingsComponent;
    std::shared_ptr<CAnalyzerComponent> mpAnalyzerComponent;

    QTimer mDisconnectionTimer;

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    QTableView* mpMainTableView;
#endif
};
