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
#include "../api/IDLTMessageAnalyzerControllerConsumer.hpp"
#include "components/logsWrapper/api/IFileWrapper.hpp"
#include "components/logsWrapper/api/IMsgWrapper.hpp"
#include "components/log/api/CLog.hpp"
#include "common/cpp_extensions.hpp"

#include "DMA_Plantuml.hpp"

//#define DEBUG_MESSAGES

Q_DECLARE_METATYPE(tWorkerId)

//Static fields and constants
static const int CHUNK_SIZE = 4000;

//CMTAnalyzer
CMTAnalyzer::CMTAnalyzer(const tSettingsManagerPtr& pSettingsManager):
CSettingsManagerClient(pSettingsManager),
mWorkerItemMap(),
mRequestIdCounter( static_cast<uint64_t>(-1) )
{
    auto threadsNumber = QThread::idealThreadCount();

    // Safe one thread for GUI reflectiveness
    //threadsNumber = ( threadsNumber > 1 ) ? threadsNumber - 1 : threadsNumber ;

    for(int i = 0; i < threadsNumber; ++i )
    {
        CDLTRegexAnalyzerWorker* pWorker = new CDLTRegexAnalyzerWorker(getSettingsManager());

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
                                        const tRequestParameters& requestParameters,
                                        const tRegexScriptingMetadata& regexScriptingMetadata )
{
    tRequestId requestId = INVALID_REQUEST_ID;

    if(nullptr != requestParameters.pFile &&
       false == pClient.expired() &&
       nullptr != pClient.lock() )
    {
        connect(this,
                &IDLTMessageAnalyzerController::progressNotification,
                [pClient](const tProgressNotificationData& progressNotificationData)
        {
            if(false == pClient.expired())
            {
                pClient.lock()->progressNotification( progressNotificationData );
            }
        });

        if(true == requestParameters.regex.isValid())
        {
            tRequestData requestData( pClient,
                                      requestParameters.pFile,
                                      requestParameters.fromMessage,
                                      requestParameters.numberOfMessages,
                                      requestParameters.regex,
                                      regexScriptingMetadata,
                                      requestParameters.numberOfThreads,
                                      requestParameters.searchColumns,
                                      requestParameters.regexStr,
                                      requestParameters.selectedAliases);

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

    const tFileWrapperPtr& pFile = inputIt.value().pFile;

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

            const auto& searchColumnsMap = qAsConst(inputIt_.value().searchColumns);
            std::set<eSearchResultColumn> searchColumnsSet;

            for (auto iter = searchColumnsMap.begin(); iter != searchColumnsMap.end(); ++iter)
            {
                if(iter.value())
                {
                    searchColumnsSet.insert(iter.key());
                }
            }

            for(int j= 0; j < chunkSize; ++j)
            {
                auto msgIdxInMainTable = startRange + j;
                auto msgIdx = pFile->getMsgIdFromIndexInMainTable( msgIdxInMainTable );
                auto pMsg = pFile->getMsg(msgIdx);

                if(nullptr != pMsg)
                {
                    tQStringPtr pStr = std::make_shared<QString>();

                    tFieldRanges fieldRanges;
                    int rangeCounter = 0;

                    for (auto iter = searchColumnsSet.begin(); iter != searchColumnsSet.end(); ++iter)
                    {
                        int newRangeCounter = rangeCounter;

                        auto pStringData = getDataStrFromMsg(msgIdx, pMsg, *iter);

                        if(nullptr != pStringData)
                        {
                            pStr->append(*pStringData);
                            newRangeCounter += pStringData->size() - 1;

                            if (std::next(iter) != searchColumnsSet.end())
                            {
                                pStr->append(" ");
                                newRangeCounter += 2;
                            }

                            tIntRange strRange(rangeCounter, rangeCounter + pStringData->size() - 1);
                            fieldRanges.insert(*iter, strRange);

                            rangeCounter = newRangeCounter;
                        }
                    }

                    tItemMetadata itemMetadata( msgIdx,
                                                msgIdxInMainTable,
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
                                                msgIdxInMainTable,
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

            tAnalyzePortionData analyzePortionData
            (
                requestId_,
                processingStrings,
                inputIt_->regex,
                inputIt_->regexScriptingMetadata,
                workerThreadCookie
            );

            QMetaObject::invokeMethod(workerItem.pDLTRegexAnalyzer, "analyzePortion", Qt::QueuedConnection,
                                      Q_ARG(tAnalyzePortionData, analyzePortionData));
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

void CMTAnalyzer::portionRegexAnalysisFinished( const tPortionRegexAnalysisFinishedData& portionRegexAnalysisFinishedData )
{
    auto requestIt = mRequestMap.find(portionRegexAnalysisFinishedData.requestId);

    if(requestIt != mRequestMap.end())
    {
        if(true == portionRegexAnalysisFinishedData.bUML_Req_Res_Ev_DuplicateFound &&
           false == requestIt->bUML_Req_Res_Ev_DuplicateFound)
        {
            requestIt->bUML_Req_Res_Ev_DuplicateFound = true;
            SEND_WRN(QString("[CMTAnalyzer][%1] Warning! Duplicated (UEV|URT|URS) UML items detected in the result string."
                             "\"First win\" strategy will be applied. Check the input regex. "
                             "The result diagram might be not what you expect to get.").arg(__FUNCTION__));
        }

        auto pClient = requestIt.value().pClient;

#ifdef DEBUG_MESSAGES
        SEND_MSG(QString("[CMTAnalyzer][%1] Finish regex worker-thread execution for chunk. "
                         "request id - %2; "
                         "chunk size - %3; "
                         "worker thread id - %4; "
                         "workerThreadCookie - %5; ")
                 .arg(__FUNCTION__)
                 .arg(portionRegexAnalysisFinishedData.requestId)
                 .arg(portionRegexAnalysisFinishedData.processedMatches.matchedItemVec.size())
                 .arg(portionRegexAnalysisFinishedData.workerId)
                 .arg(portionRegexAnalysisFinishedData.workerThreadCookie));
#endif

        switch(portionRegexAnalysisFinishedData.portionAnalysisState)
        {
            case ePortionAnalysisState::ePortionAnalysisState_SUCCESSFUL:
            {
                auto foundPendingItem = requestIt.value().pendingResults.find( portionRegexAnalysisFinishedData.workerThreadCookie );

                if( foundPendingItem != requestIt.value().pendingResults.end() )
                {
                    foundPendingItem->isResultAvailable = true;
                    foundPendingItem->foundMatchesPack = portionRegexAnalysisFinishedData.processedMatches;
                    foundPendingItem->numberOfProcessedString = portionRegexAnalysisFinishedData.numberOfProcessedString;

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

                                tProgressNotificationData progressNotificationData
                                ( portionRegexAnalysisFinishedData.requestId,
                                  requestStatus,
                                  progress,
                                  it->foundMatchesPack,
                                  portionRegexAnalysisFinishedData.bUML_Req_Res_Ev_DuplicateFound,
                                  requestIt->regexScriptingMetadata.getGroupedViewIndices() );

                                QMetaObject::invokeMethod(pClient_, "progressNotification", Qt::QueuedConnection,
                                                          Q_ARG(tProgressNotificationData, progressNotificationData));

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
            case ePortionAnalysisState::ePortionAnalysisState_ERROR:
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
                                         const tFileWrapperPtr& pFile_,
                                         const int& fromMessage_,
                                         const int& numberOfMessages_,
                                         const QRegularExpression& regex_,
                                         const tRegexScriptingMetadata& regexScriptingMetadata_,
                                         const int& numberOfThreads_,
                                         const tSearchResultColumnsVisibilityMap& searchColumns_,
                                         const QString& regexStr_,
                                         const QStringList& selectedLiases_):
    pClient(pClient_),
    pFile(pFile_),
    requestedRegexMatches(0),
    processedRegexMatches(0),
    numberOfMessagesToBeAnalyzed(numberOfMessages_),
    regex(regex_),
    regexScriptingMetadata(regexScriptingMetadata_),
    numberOfThreads(numberOfThreads_),
    searchColumns(searchColumns_),
    regexStr(regexStr_),
    selectedLiases(selectedLiases_),
    fromMessage(fromMessage_),
    workerThreadCookieCounter(0),
    bUML_Req_Res_Ev_DuplicateFound(false),
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
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QThread, 1, *, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CDLTRegexAnalyzerWorker, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
