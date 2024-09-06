/**
 * @file    CContinuousAnalyzer.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CContinuousAnalyzer class
 */
#pragma once

#include "../api/IDLTMessageAnalyzerController.hpp"
#include "../api/IDLTMessageAnalyzerControllerConsumer.hpp"
#include "functional"
#include "QRegularExpression"

class IDLTMessageAnalyzerControllerConsumer;

/**
 * @brief The CSubConsumer class - this internal class is used as an extention of IDLTMessageAnalyzerControllerConsumer.
 * It allows to delegate all responses to real client through the injected callback.
 * Note! Internal class still declared in header, as it needs to participate in queued communications, thus needs to be a Q_OBJECT
 */
class CSubConsumer : public IDLTMessageAnalyzerControllerConsumer
{
    Q_OBJECT
public:
    typedef std::function<void(const tProgressNotificationData&)> tCallback;
    CSubConsumer(const tDLTMessageAnalyzerControllerPtr& pController);
    void setCallback(const tCallback& callback);
    void progressNotification( const tProgressNotificationData& progressNotificationData ) override;
private:
    tCallback mCallback;
};

/**
 * @brief The CContinuousAnalyzer class - class, which re-implements the IDLTMessageAnalyzerController API, and adds the
 * "continuous analysis" capability to it.
 * It uses the undelying sub-analyzer, which is also implementation of IDLTMessageAnalyzerController in order to perform one-shot analysis
 * iterations.
 */
class CContinuousAnalyzer : public IDLTMessageAnalyzerController
{
    Q_OBJECT
public:
    CContinuousAnalyzer(const tDLTMessageAnalyzerControllerPtr& pSubAnalyzer);

    //IDLTMessageAnalyzerController implementation

    /**
     * @brief requestAnalyze - check IDLTMessageAnalyzerController for details
     */
    tRequestId requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                               const tRequestParameters& requestParameters,
                               const tRegexScriptingMetadata& regexScriptingMetadata ) override;

    /**
     * @brief requestAnalyze - check IDLTMessageAnalyzerController for details
     */
    void cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient, const tRequestId& requestId ) override;

    /**
     * @brief requestAnalyze - check IDLTMessageAnalyzerController for details
     */
    int getMaximumNumberOfThreads() const override;

private:// methods
    struct tRequestData;
    typedef QMap<tRequestId, tRequestData> tRequestDataMap;

    void triggerContinuousAnalysisIteration(const tRequestDataMap::iterator& inputIt);
    void progressNotification(const tProgressNotificationData& progressNotificationData);

private:// fields
    std::shared_ptr<IDLTMessageAnalyzerControllerConsumer> mpSubConsumer;
    tDLTMessageAnalyzerControllerPtr mpSubAnalyzer;

    struct tRequestData
    {
        tRequestData(const tRequestId& requestId_,
                     const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient_,
                     const tRequestId& subRequestId_,
                     bool bIsContinuousAnalysis_,
                     const tFileWrapperPtr& pFile,
                     const QRegularExpression& regex_,
                     const int& numberOfThreads_,
                     const tRegexScriptingMetadata& regexScriptingMetadata_,
                     const tSearchResultColumnsVisibilityMap& searchColumns_,
                     const QString& regexStr_,
                     const QStringList& selectedAlises_);
        tRequestId requestId;
        std::weak_ptr<IDLTMessageAnalyzerControllerConsumer> pClient;
        tRequestId subRequestId;
        bool bIsContinuousAnalysis;
        int fromMessage;
        int toMessage;
        tFileWrapperPtr mpFile;
        QRegularExpression regex;
        tRegexScriptingMetadata regexScriptingMetadata;
        int numberOfThreads;
        bool bContinuousModeActive;
        tSearchResultColumnsVisibilityMap searchColumns;
        QString regexStr;
        QStringList selectedAlises;
    };

    tRequestDataMap mRequestDataMap;
    typedef QMap<tRequestId, tRequestId> tSubRequestDataMap;
    tSubRequestDataMap mSubRequestDataMap;
    tRequestId mRequestIdCounter;
};
