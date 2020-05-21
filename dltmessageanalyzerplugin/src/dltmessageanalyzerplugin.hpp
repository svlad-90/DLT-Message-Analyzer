/**
 * @file    dltmessageanalyzerplugin.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the DLTMessageAnalyzerPlugin class
 */

#ifndef DLTMESSAGEANALYZERPLUGIN_H
#define DLTMESSAGEANALYZERPLUGIN_H

#include "memory"

#include <QObject>

#include "plugininterface.h"
#include "form.h"

#include "Definitions.hpp"


#define DLT_MESSAGE_ANALYZER_PLUGIN_VERSION "1.0.22"

class CDLTMessageAnalyzer;
typedef std::shared_ptr<CDLTMessageAnalyzer> tDLTMessageAnalyzerPtr;
typedef std::shared_ptr<QString> tQStringPtr;
typedef QMap<int, tQStringPtr > tMsgMap;
class IDLTMessageAnalyzerController;

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
    ~DLTMessageAnalyzerPlugin() override;

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

signals:
    void analysisWithEmptyStringRequested();

public slots:
    void initMsgDecodedForwarded(int index, tDLTMsgWrapperPtr pMsg);

private: // methods

    bool initControl(QDltControl *) final override { return true; }
    bool initConnections(QStringList) final override { return true; }
    bool controlMsg(int, QDltMsg &) final override { return true; }
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname) final override;
    bool autoscrollStateChanged(bool) final override { return true; }

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
    std::shared_ptr<IDLTMessageAnalyzerController> mpMessageAnalyzerController;
    tDLTFileWrapperPtr mpFile;
    int mLastAvailableNumberOfMsg;
    QMap<QString, QDltConnection::QDltConnectionState> mConnecitonsMap;
    QDltConnection::QDltConnectionState mConnectionState;
    bool mbAnalysisRunning;
};

#endif // DLTMESSAGEANALYZERPLUGIN_H
