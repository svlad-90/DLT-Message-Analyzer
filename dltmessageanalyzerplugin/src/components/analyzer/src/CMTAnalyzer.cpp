/**
 * @file    CMTAnalyzer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CMTAnalyzer class
 */

#include <assert.h>

#include "QThread"
#include "qdebug.h"
#include <QMetaObject>

#include "qdlt.h"

#include "CMTAnalyzer.hpp"
#include "dltWrappers/CDLTFileWrapper.hpp"
#include "../api/IDLTMessageAnalyzerControllerConsumer.hpp"
#include "dltWrappers/CDLTMsgWrapper.hpp"
#include "log/CLog.hpp"
#include "common/cpp_extensions.hpp"

#include "DMA_Plantuml.hpp"

//#define DEBUG_MESSAGES

Q_DECLARE_METATYPE(tWorkerId)

//Static fields and constants
static const int CHUNK_SIZE = 4000;

//CMTAnalyzer
CMTAnalyzer::CMTAnalyzer():
mWorkerItemMap(),
mRequestIdCounter( static_cast<uint64_t>(-1) )
{
    auto threadsNumber = QThread::idealThreadCount();

    // Safe one thread for GUI reflectiveness
    //threadsNumber = ( threadsNumber > 1 ) ? threadsNumber - 1 : threadsNumber ;

    for(int i = 0; i < threadsNumber; ++i )
    {
        CDLTRegexAnalyzerWorker* pWorker = new CDLTRegexAnalyzerWorker();

        // instance of CDLTRegexAnalyzerWorker, located in QThread will send updates to CMTAnalyzer
        connect(pWorker, &CDLTRegexAnalyzerWorker::portionAnalysisFinished, this, &CMTAnalyzer::portionRegexAnalysisFinished, Qt::QueuedConnection);

        tQThreadPtr pWorkerThread = cpp_14::make_unique<QThread>();

        // After worker thread will be stopped, worker object should be aynchronously deleted.
        connect(pWorkerThread.get(), &QThread::finished, pWorker, &QObject::deleteLater, Qt::QueuedConnection);

        pWorker->moveToThread(pWorkerThread.get());

        tWorkerItem workerItem( pWorkerThread, pWorker );

        mWorkerItemMap.insert( pWorker->getWorkerId(), workerItem );

        pWorkerThread->start();
    }
}

CMTAnalyzer::~CMTAnalyzer()
{
    for(auto & workerItem : mWorkerItemMap)
    {
        workerItem.pQThread->quit();
        workerItem.pQThread->wait();
        workerItem.pQThread->deleteLater();
    }

    mWorkerItemMap.clear();
}

tRequestId CMTAnalyzer::requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                                        const tDLTFileWrapperPtr& pFile,
                                        const int& fromMessage,
                                        const int& numberOfMessages,
                                        const QRegularExpression& regex,
                                        const int& numberOfThreads,
                                        const tRegexScriptingMetadata& regexScriptingMetadata,
                                        bool)
{
    tRequestId requestId = INVALID_REQUEST_ID;

    if(nullptr != pFile &&
       false == pClient.expired() &&
       nullptr != pClient.lock() )
    {
        connect(this,
                &IDLTMessageAnalyzerController::progressNotification,
                [pClient](const tRequestId& requestId_,
                const eRequestState& requestState,
                const int8_t& progress,
                const tFoundMatchesPack& processedMatches)
        {
            if(false == pClient.expired())
            {
                pClient.lock()->progressNotification(requestId_,
                                                     requestState,
                                                     progress,
                                                     processedMatches);
            }
        });

        if(true == regex.isValid())
        {
            tRequestData requestData( pClient, pFile, fromMessage, numberOfMessages, regex, regexScriptingMetadata, numberOfThreads );

            if(0 != requestData.numberOfMessagesToBeAnalyzed)
            {
                requestId = ++mRequestIdCounter;
                auto it = mRequestMap.insert(requestId, requestData);
                bool bResult = regexAnalysisIteration(it);

                if(false == bResult)
                {
                    requestId = INVALID_REQUEST_ID;
                    --mRequestIdCounter;
                }
            }
        }
    }

    return requestId;
}

void CMTAnalyzer::sendError( tRequestMap::iterator& requestIt )
{
    if(requestIt != mRequestMap.end())
    {
        auto pClient = requestIt.value().pClient;

        if(false == pClient.expired())
        {
            SEND_ERR(QString("[CMTAnalyzer][%1] request \"%2\" has finished with error")
                     .arg(__FUNCTION__)
                     .arg(requestIt.key()));

            QMetaObject::invokeMethod(pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                      Q_ARG(tRequestId, requestIt.key()),
                                      Q_ARG(eRequestState, eRequestState::ERROR_STATE),
                                      Q_ARG(int8_t, 100),
                                      Q_ARG(tFoundMatchesPack, tFoundMatchesPack()));
        }

        mRequestMap.erase(requestIt);
    }
}

bool CMTAnalyzer::regexAnalysisIteration(tRequestMap::iterator& inputIt, const tWorkerId& workerId)
{
    bool bResult = true;

    const tRequestId requestId = inputIt.key();

    const tDLTFileWrapperPtr& pFile = inputIt.value().pFile;

    auto analysisIterationSpecificThread = [pFile](const tRequestId& requestId_,
            tRequestMap::iterator& inputIt_,
            const tWorkerId& workerId_,
            const tWorkerItem& workerItem)
    {
        const auto startRange = inputIt_.value().fromMessage + inputIt_.value().requestedRegexMatches;
        const auto endRange = inputIt_.value().fromMessage + inputIt_.value().numberOfMessagesToBeAnalyzed;

        if(startRange < endRange)
        {
            auto chunkSize = ( endRange - startRange ) > CHUNK_SIZE ? CHUNK_SIZE : ( endRange - startRange );

            tProcessingStrings processingStrings;
            processingStrings.reserve(chunkSize);

            for(int j= 0; j < chunkSize; ++j)
            {
                auto msgIdxFiltered = startRange + j;
                auto msgIdx = pFile->getMsgRealPos( msgIdxFiltered );
                auto pMsg = pFile->getMsg(msgIdx);

                if(nullptr != pMsg)
                {
                    QString appId = pMsg->getApid();
                    QString ctId = pMsg->getCtid();

                    auto appIdSize = appId.size();
                    tIntRange appRange( 0, appIdSize - 1 );
                    auto ctIdSize = ctId.size();
                    tIntRange ctRange( appRange.to + 2, appRange.to + 2 + ctIdSize - 1 );

                    QString payloadStr = pMsg->getPayload();
                    auto payloadSize = payloadStr.size();

                    tQStringPtr pStr = std::make_shared<QString>();
                    pStr->reserve(appId.size() + ctId.size() + payloadStr.size() + 2);
                    pStr->append( appId ).append(" ").append( ctId ).append(" ").append(payloadStr);

                    tIntRange payloadRange( ctRange.to + 2, ctRange.to + 2 + payloadSize - 1 );

                    tFieldRanges fieldRanges;

                    fieldRanges.insert(eSearchResultColumn::Apid, appRange);
                    fieldRanges.insert(eSearchResultColumn::Ctid, ctRange);
                    fieldRanges.insert(eSearchResultColumn::Payload, payloadRange);

                    tItemMetadata itemMetadata( msgIdx,
                                                msgIdxFiltered,
                                                fieldRanges,
                                                pStr->size(),
                                                pMsg->getSize(),
                                                pMsg->getTimestamp());

                    processingStrings.push_back( tProcessingStringItem( itemMetadata, pStr ) );
                }
                else
                {
                    qDebug() << "Failed to get msg with idx - " << startRange + j;
                    tItemMetadata itemMetadata( msgIdx,
                                                msgIdxFiltered,
                                                tFieldRanges(),
                                                0,
                                                0u,
                                                0u);
                    processingStrings.push_back( tProcessingStringItem( itemMetadata, std::make_shared<QString>( "" ) ) ); // TODO: optimize
                }
            }

            inputIt_.value().requestedRegexMatches+=static_cast<int>(processingStrings.size());

            auto workerThreadCookie = inputIt_.value().workerThreadCookieCounter++;
            tRequestData::tPendingResultsItem pendingResultsItem;
            pendingResultsItem.workerId = workerId_;
            inputIt_->pendingResults.insert( workerThreadCookie, pendingResultsItem );

#ifdef DEBUG_MESSAGES
                    SEND_MSG(QString("[CMTAnalyzer][%1] Start regex worker-thread execution for chunk. "
                                     "request id - %2; "
                                     "chunk size - %3; "
                                     "worker thread id - %4; "
                                     "workerThreadCookie - %5;")
                             .arg(__FUNCTION__)
                             .arg(requestId_)
                             .arg(processingStrings.size())
                             .arg(workerId_)
                             .arg(workerThreadCookie));
#endif

            QMetaObject::invokeMethod(workerItem.pDLTRegexAnalyzer, "analyzePortion", Qt::QueuedConnection,
                                      Q_ARG(tRequestId, requestId_),
                                      Q_ARG(tProcessingStrings, processingStrings),
                                      Q_ARG(QRegularExpression, inputIt_->regex),
                                      Q_ARG(tRegexScriptingMetadata, inputIt_->regexScriptingMetadata),
                                      Q_ARG(tWorkerThreadCookie, workerThreadCookie));
        }
    };

    int fileSize = pFile->size();

    if( 0 != fileSize )
    {
        if(0 != inputIt->numberOfMessagesToBeAnalyzed)
        {
            if(-1 == workerId)
            {
                auto it = mWorkerItemMap.begin();
                auto i = 0;

                for(; it != mWorkerItemMap.end() && i < inputIt->numberOfThreads; ++it, ++i)
                {
                    // let's feed it some new activity
                    analysisIterationSpecificThread(requestId, inputIt, it.key(), it.value());
                }
            }
            else
            {
                auto foundWorkerItem = mWorkerItemMap.find(workerId);

                if(mWorkerItemMap.end() != foundWorkerItem)
                {
                    analysisIterationSpecificThread(requestId, inputIt, workerId, foundWorkerItem.value());
                }
            }
        }
    }
    else // SUDDENLY! File became empty, while it was not for this request.
    {
        //end up request
        bResult = false;
        SEND_ERR( QString("[CMTAnalyzer][%1] QDltFile has became empty! Stop currently processed request ( id - %2 )")
                  .arg(__FUNCTION__)
                  .arg(requestId));
    }

    return bResult;
}

void CMTAnalyzer::portionRegexAnalysisFinished( const tRequestId& requestId,
                                                int numberOfProcessedString,
                                                CDLTRegexAnalyzerWorker::ePortionAnalysisState portionAnalysisState,
                                                const tFoundMatchesPack& processedMatches,
                                                const tWorkerId&,
                                                const tWorkerThreadCookie& workerThreadCookie )
{
    auto requestIt = mRequestMap.find(requestId);

    if(requestIt != mRequestMap.end())
    {
        auto pClient = requestIt.value().pClient;

#ifdef DEBUG_MESSAGES
        SEND_MSG(QString("[CMTAnalyzer][%1] Finish regex worker-thread execution for chunk. "
                         "request id - %2; "
                         "chunk size - %3; "
                         "worker thread id - %4; "
                         "workerThreadCookie - %5; ")
                 .arg(__FUNCTION__)
                 .arg(requestId)
                 .arg(processedMatches.matchedItemVec.size())
                 .arg(workerId)
                 .arg(workerThreadCookie));
#endif

        switch(portionAnalysisState)
        {
            case CDLTRegexAnalyzerWorker::ePortionAnalysisState::ePortionAnalysisState_SUCCESSFUL:
            {
                auto foundPendingItem = requestIt.value().pendingResults.find( workerThreadCookie );

                if( foundPendingItem != requestIt.value().pendingResults.end() )
                {
                    foundPendingItem->isResultAvailable = true;
                    foundPendingItem->foundMatchesPack = processedMatches;
                    foundPendingItem->numberOfProcessedString = numberOfProcessedString;

                    for(auto it = requestIt.value().pendingResults.begin(); it != requestIt.value().pendingResults.end(); )
                    {
                        if(true == it->isResultAvailable)
                        {
                            requestIt.value().processedRegexMatches+=it->numberOfProcessedString;

                            if( false == requestIt.value().pClient.expired() )
                            {
                                bool analysisFinished = requestIt.value().processedRegexMatches >= requestIt.value().numberOfMessagesToBeAnalyzed;

                                int8_t progress =
                                static_cast<int8_t>( ( static_cast<double>( requestIt.value().processedRegexMatches + requestIt.value().processedRegexMatches ) / ( requestIt.value().numberOfMessagesToBeAnalyzed * 2 ) ) * 100 );

                                auto requestStatus = analysisFinished ? eRequestState::SUCCESSFUL : eRequestState::PROGRESS;
                                auto pClient_ = requestIt.value().pClient.lock().get();

                                QMetaObject::invokeMethod(pClient_, "progressNotification", Qt::QueuedConnection,
                                                          Q_ARG(tRequestId, requestId),
                                                          Q_ARG(eRequestState, requestStatus),
                                                          Q_ARG(int8_t, progress),
                                                          Q_ARG(tFoundMatchesPack, it->foundMatchesPack));

                                auto specificWorkerId = it->workerId;

                                it = requestIt.value().pendingResults.erase(it);

                                //do not use it after this line

                                if(requestIt.value().requestedRegexMatches < requestIt.value().numberOfMessagesToBeAnalyzed) // if not all entries were requested
                                {
                                    bool bResult = regexAnalysisIteration(requestIt, specificWorkerId);

                                    if(false == bResult)
                                    {
                                        sendError(requestIt);
                                        break; // stop loop
                                    }
                                }
                                else // if all entries were requested
                                {
                                    if(true == analysisFinished) // if all entries are processed
                                    {
                                        mRequestMap.erase(requestIt);
                                        break; // stop loop
                                    }
                                }
                            }
                            else
                            {
                                mRequestMap.erase(requestIt);
                                break; // stop loop
                            }
                        }
                        else
                        {
                            break; // Important! Stop loop here to preserve order of delivered results
                        }
                    }
                }
            }
                break;
            case CDLTRegexAnalyzerWorker::ePortionAnalysisState::ePortionAnalysisState_ERROR:
            {
                if( false == requestIt.value().pClient.expired() )
                {
                    sendError(requestIt);
                }
            }
                break;
        }
    }
}

void CMTAnalyzer::cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient, const tRequestId& requestId )
{
    //qDebug() << "CMTAnalyzer::" << __FUNCTION__;

    auto foundRequest = mRequestMap.find(requestId);

    if(foundRequest != mRequestMap.end())
    {
        if(false == foundRequest->pClient.expired() &&
           false == pClient.expired() ) // if both clients are alive
        {
            if(foundRequest->pClient.lock() == pClient.lock()) // ONLY if client addresses are equal
            {
                mRequestMap.erase(foundRequest); // let's remove the request
            }
        }
    }
}

int CMTAnalyzer::getMaximumNumberOfThreads() const
{
    return QThread::idealThreadCount();
}

//CMTAnalyzer::tWorkerItem
CMTAnalyzer::tWorkerItem::tWorkerItem(const tQThreadPtr& pQThread_, const tDLTRegexAnalyzerWorkerPtr& pDLTRegexAnalyzer_):
    pQThread(std::move(pQThread_)), pDLTRegexAnalyzer(pDLTRegexAnalyzer_)
{

}

//CMTAnalyzer::tRequestData
CMTAnalyzer::tRequestData::tRequestData( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient_,
                                         const tDLTFileWrapperPtr& pFile_,
                                         const int& fromMessage_,
                                         const int& numberOfMessages_,
                                         const QRegularExpression& regex_,
                                         const tRegexScriptingMetadata& regexScriptingMetadata_,
                                         const int& numberOfThreads_):
    pClient(pClient_),
    pFile(pFile_),
    requestedRegexMatches(0),
    processedRegexMatches(0),
    numberOfMessagesToBeAnalyzed(numberOfMessages_),
    regex(regex_),
    regexScriptingMetadata(regexScriptingMetadata_),
    numberOfThreads(numberOfThreads_),
    fromMessage(fromMessage_),
    workerThreadCookieCounter(0),
    pendingResults()
{
    if( numberOfMessagesToBeAnalyzed >= pFile->size() )
    {
        if(nullptr != pFile)
        {
            numberOfMessagesToBeAnalyzed = pFile->size() - fromMessage;
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_Analyzer)
    PUML_CLASS_BEGIN_CHECKED(CMTAnalyzer)
        PUML_INHERITANCE_CHECKED(IDLTMessageAnalyzerController, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QThread, 1, *, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CDLTRegexAnalyzerWorker, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
