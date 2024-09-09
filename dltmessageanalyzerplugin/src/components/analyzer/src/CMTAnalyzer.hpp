/**
 * @file    CMTAnalyzer.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CMTAnalyzer class
 */
#pragma once

#include <list>

#include <QRegularExpression>

#include "../api/Definitions.hpp"

#include "../api/IDLTMessageAnalyzerController.hpp"
#include "CDLTRegexAnalyzerWorker.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

//Forward declarations
class QThread;
class IDLTMessageAnalyzerControllerConsumer;
class CDLTRegexAnalyzerWorker;
typedef int tWorkerId;

// CMTAnalyzer
class CMTAnalyzer: public IDLTMessageAnalyzerController,
                   public CSettingsManagerClient
{
    Q_OBJECT
    public:
        CMTAnalyzer(const tSettingsManagerPtr& pSettingsManager);
        ~CMTAnalyzer() override;

        //IDLTMessageAnalyzerController implementation
        tRequestId requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                                   const tRequestParameters& requestParameters,
                                   const tRegexScriptingMetadata& regexScriptingMetadata ) override;
        void cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient, const tRequestId& requestId ) override;
        int getMaximumNumberOfThreads() const override;

private slots:
    void portionRegexAnalysisFinished( const tPortionRegexAnalysisFinishedData& portionRegexAnalysisFinishedData );

private: // methods

    struct tRequestData
    {
        tRequestData( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient_,
                      const tFileWrapperPtr& pFile_,
                      const int& fromMessage_,
                      const int& numberOfMessages,
                      const QRegularExpression& regex_,
                      const tRegexScriptingMetadata& regexScriptingMetadata_,
                      const int& numberOfThreads_,
                      const tSearchResultColumnsVisibilityMap& searchColumns_,
                      const QString& regexStr_,
                      const QStringList& selectedLiases_);
        std::weak_ptr<IDLTMessageAnalyzerControllerConsumer> pClient;
        tFileWrapperPtr pFile; // file, which should be analyzed
        int requestedRegexMatches;  // number of strings, which were requested to be analyzed from the regex analyzer thread
        int processedRegexMatches; // number of strings, which were already analyzed by the regex analyzer thread
        int numberOfMessagesToBeAnalyzed; // overall number of messages, which were requested to be analyzed
        QRegularExpression regex; // the regular expression, with which we are working
        tRegexScriptingMetadata regexScriptingMetadata;
        int numberOfThreads; // number of threads, to be used for analysis
        tSearchResultColumnsVisibilityMap searchColumns; // columns in which we search for the match.
        QString regexStr; // the string using which the 'regex' field was creted
        QStringList selectedLiases; // the regex aliasaes that were used to form the query
        int fromMessage; // from which message to start analysis
        tWorkerThreadCookie workerThreadCookieCounter;
        bool bUML_Req_Res_Ev_DuplicateFound = false;

        struct tPendingResultsItem
        {
            bool isResultAvailable = false;
            tFoundMatchesPack foundMatchesPack;
            int numberOfProcessedString;
            int workerId;
        };

        typedef QMap<tWorkerThreadCookie, tPendingResultsItem> tPendingResultsMap;
        tPendingResultsMap pendingResults; // final results, which are waiting to be norified to the client.
    };

    typedef QMap<tRequestId, tRequestData> tRequestMap;

    // be aware, that this method might erase and invalidate requestIt from collection, which contains it
    void sendError( tRequestMap::iterator& requestIt );

    bool regexAnalysisIteration( tRequestMap::iterator& inputIt, const tWorkerId& workerId = -1 );

private: //fields
    typedef std::shared_ptr<QThread> tQThreadPtr;

    typedef CDLTRegexAnalyzerWorker* tDLTRegexAnalyzerWorkerPtr;
    struct tWorkerItem
    {
        tWorkerItem(const tQThreadPtr& pQThread_, const tDLTRegexAnalyzerWorkerPtr& pDLTRegexAnalyzer_);
        tQThreadPtr pQThread;
        tDLTRegexAnalyzerWorkerPtr pDLTRegexAnalyzer;
    };

    typedef QMap<tWorkerId, tWorkerItem> tWorkerItemMap;
    tWorkerItemMap mWorkerItemMap;

    tRequestMap mRequestMap;
    tRequestId mRequestIdCounter;
};
