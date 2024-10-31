/**
 * @file    CContinuousAnalyzer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CContinuousAnalyzer class
 */

#include "qdlt.h"

#include "CContinuousAnalyzer.hpp"
#include "../api/IDLTMessageAnalyzerControllerConsumer.hpp"
#include "functional"
#include "QTimer"
#include "QDebug"

#include "components/logsWrapper/api/IFileWrapper.hpp"

#include "DMA_Plantuml.hpp"

static const int WAITING_TIME = 50;

CSubConsumer::CSubConsumer(const tDLTMessageAnalyzerControllerPtr& pController):
    IDLTMessageAnalyzerControllerConsumer(pController),
    mCallback()
{}

void CSubConsumer::setCallback(const tCallback& callback)
{
    mCallback = callback;
}

void CSubConsumer::progressNotification( const tProgressNotificationData& progressNotificationData )
{
    if( mCallback )
    {
        mCallback( progressNotificationData );
    }
}

//CContinuousAnalyzer
CContinuousAnalyzer::CContinuousAnalyzer(const tDLTMessageAnalyzerControllerPtr& pSubAnalyzer):
IDLTMessageAnalyzerController(),
mpSubConsumer( nullptr ),
mpSubAnalyzer( pSubAnalyzer ),
mRequestDataMap(),
mSubRequestDataMap(),
mRequestIdCounter(static_cast<uint64_t>(-1))
{
    auto pConsumer = IDLTMessageAnalyzerControllerConsumer::createInstance<CSubConsumer>(pSubAnalyzer);

    auto callback = [this](const tProgressNotificationData& progressNotificationData)
    {
        progressNotification( progressNotificationData );
    };

    pConsumer->setCallback( callback );

    mpSubConsumer = pConsumer;
}

void CContinuousAnalyzer::progressNotification( const tProgressNotificationData& progressNotificationData )
{
    auto foundSubRequest = mSubRequestDataMap.find(progressNotificationData.requestId);

    if(mSubRequestDataMap.end() != foundSubRequest)
    {
        auto foundRequest = mRequestDataMap.find(foundSubRequest.value());

        if(mRequestDataMap.end() != foundRequest)
        {
            if(false == foundRequest.value().bIsContinuousAnalysis)
            {
                if(false == foundRequest.value().pClient.expired())
                {
                    auto progressNotificationDataCopy = progressNotificationData;
                    progressNotificationDataCopy.requestId = foundRequest.key();

                    QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                              Q_ARG(tProgressNotificationData, progressNotificationDataCopy));

                    if(eRequestState::ERROR_STATE == progressNotificationDataCopy.requestState ||
                            eRequestState::SUCCESSFUL == progressNotificationDataCopy.requestState)
                    {
                        analysisFinished(foundRequest.key());
                        mRequestDataMap.erase(foundRequest);
                        mSubRequestDataMap.erase(foundSubRequest);
                    }
                }
                else
                {
                    if(eRequestState::ERROR_STATE == progressNotificationData.requestState ||
                            eRequestState::SUCCESSFUL == progressNotificationData.requestState)
                    {
                        analysisFinished(foundRequest.key());
                        mRequestDataMap.erase(foundRequest);
                        mSubRequestDataMap.erase(foundSubRequest);
                    }
                }
            }
            else
            {
                switch(progressNotificationData.requestState)
                {
                    case eRequestState::PROGRESS:
                    {
                        if(false == foundRequest.value().pClient.expired())
                        {
                            int8_t injectedProgress = foundRequest.value().bContinuousModeActive ? 100u : progressNotificationData.progress;

                            tProgressNotificationData resultProgressNotificationData
                            (foundSubRequest.value(),
                            progressNotificationData.requestState,
                            injectedProgress,
                            progressNotificationData.processedMatches,
                            progressNotificationData.bUML_Req_Res_Ev_DuplicateFound,
                            progressNotificationData.groupedViewIndices);

                            QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                                      Q_ARG(tProgressNotificationData, resultProgressNotificationData));
                        }
                    }
                        break;
                    case eRequestState::ERROR_STATE:
                    {
                        if(false == foundRequest.value().pClient.expired())
                        {
                            tProgressNotificationData resultProgressNotificationData
                            (foundSubRequest.value(),
                            progressNotificationData.requestState,
                            progressNotificationData.progress,
                            progressNotificationData.processedMatches,
                            progressNotificationData.bUML_Req_Res_Ev_DuplicateFound,
                            progressNotificationData.groupedViewIndices);

                            QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                                      Q_ARG(tProgressNotificationData, resultProgressNotificationData));
                        }

                        mSubRequestDataMap.erase(foundSubRequest);

                        foundRequest.value().subRequestId = INVALID_REQUEST_ID;
                        mRequestDataMap.erase(foundRequest);
                    }
                        break;
                    case eRequestState::SUCCESSFUL:
                    {
                        if(false == foundRequest.value().pClient.expired())
                        {
                            tProgressNotificationData resultProgressNotificationData
                            (foundSubRequest.value(),
                            eRequestState::PROGRESS,
                            100,
                            progressNotificationData.processedMatches,
                            progressNotificationData.bUML_Req_Res_Ev_DuplicateFound,
                            progressNotificationData.groupedViewIndices);

                            QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                                      Q_ARG(tProgressNotificationData, resultProgressNotificationData));
                        }

                        mSubRequestDataMap.erase(foundSubRequest); // sub-request is not relevant anymore

                        foundRequest.value().subRequestId = INVALID_REQUEST_ID;
                        foundRequest.value().bContinuousModeActive = true;
                        triggerContinuousAnalysisIteration(foundRequest);
                    }
                        break;
                }
            }
        }
    }
    else
    {
        qDebug() << "Warning. Progress notification for irrelevant request id - " << progressNotificationData.requestId;
    }
}

tRequestId CContinuousAnalyzer::requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                                                const tRequestParameters& requestParameters,
                                                const tRegexScriptingMetadata& regexScriptingMetadata )
{
    tRequestId resultRequestId = INVALID_REQUEST_ID;

    if(nullptr != mpSubAnalyzer)
    {
        tRequestId subRequestId = mpSubAnalyzer->requestAnalyze(mpSubConsumer,
                                                                requestParameters,
                                                                regexScriptingMetadata);

        if(INVALID_REQUEST_ID != subRequestId)
        {
            resultRequestId = ++mRequestIdCounter;
            tRequestData requestData( resultRequestId,
                                      pClient,
                                      subRequestId,
                                      requestParameters.isContinuous,
                                      requestParameters.pFile,
                                      requestParameters.regex,
                                      requestParameters.numberOfThreads,
                                      regexScriptingMetadata,
                                      requestParameters.searchColumns,
                                      requestParameters.regexStr,
                                      requestParameters.selectedAliases);
            requestData.fromMessage = requestParameters.fromMessage;
            requestData.toMessage = requestParameters.fromMessage + requestParameters.numberOfMessages;

            mRequestDataMap.insert(resultRequestId, requestData);
            mSubRequestDataMap.insert( subRequestId, resultRequestId );

            analysisStarted(resultRequestId, requestParameters.regexStr, requestParameters.selectedAliases);
        }
    }

    return resultRequestId;
}

void CContinuousAnalyzer::cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>&, const tRequestId& requestId )
{
    if(nullptr != mpSubAnalyzer)
    {
        auto foundRequest = mRequestDataMap.find(requestId);

        if(mRequestDataMap.end() != foundRequest)
        {
            if(INVALID_REQUEST_ID != foundRequest.value().subRequestId)
            {
                mpSubAnalyzer->cancelRequest(mpSubConsumer, foundRequest.value().subRequestId);

                auto foundSubRequest = mSubRequestDataMap.find( foundRequest.value().subRequestId );

                if(foundSubRequest != mSubRequestDataMap.end())
                {
                    mSubRequestDataMap.erase(foundSubRequest);
                }
            }

            mRequestDataMap.erase(foundRequest);
            analysisFinished(foundRequest.key());
        }
    }
}

int CContinuousAnalyzer::getMaximumNumberOfThreads() const
{
    int result = 0;

    if(nullptr != mpSubAnalyzer)
    {
        result = mpSubAnalyzer->getMaximumNumberOfThreads();
    }

    return result;
}

void CContinuousAnalyzer::triggerContinuousAnalysisIteration(const tRequestDataMap::iterator& inputIt)
{
    auto fileSize = inputIt->mpFile->size();

    if(fileSize > 0)
    {
        if(fileSize > ( inputIt.value().toMessage ) )
        {
            // analyze
            inputIt->fromMessage = inputIt->toMessage;
            inputIt->toMessage = fileSize;

            if(nullptr != mpSubAnalyzer)
            {
                tRequestParameters requestParameters
                (
                    inputIt->mpFile,
                    inputIt->fromMessage,
                    inputIt->toMessage - inputIt->fromMessage,
                    inputIt->regex,
                    inputIt->numberOfThreads,
                    inputIt->bIsContinuousAnalysis,
                    inputIt->searchColumns,
                    inputIt->regexStr,
                    inputIt->selectedAlises
                );

                tRequestId subRequestId = mpSubAnalyzer->requestAnalyze(mpSubConsumer,
                                                                        requestParameters,
                                                                        inputIt->regexScriptingMetadata);

                if(INVALID_REQUEST_ID != subRequestId)
                {
                    mSubRequestDataMap.insert( subRequestId, inputIt->requestId);
                    inputIt.value().subRequestId = subRequestId;
                }
                else
                {
                    if(false == inputIt.value().pClient.expired())
                    {
                        tProgressNotificationData progressNotificationData
                        (
                            inputIt.value().requestId,
                            eRequestState::ERROR_STATE,
                            0,
                            tFoundMatchesPack(),
                            false,
                            tGroupedViewIndices()
                        );

                        inputIt.value().pClient.lock()->progressNotification(progressNotificationData);
                    }

                    mRequestDataMap.erase( inputIt );
                }
            }
        }
        else if( fileSize == ( inputIt.value().toMessage ) )
        {
            // wait more
            auto requestId = inputIt.value().requestId;
            QTimer::singleShot(WAITING_TIME, this, [this, requestId]()
            {
                auto foundRequest = mRequestDataMap.find(requestId);

                if(mRequestDataMap.end() != foundRequest) // NOLINT
                {
                    triggerContinuousAnalysisIteration(foundRequest); // NOLINT
                }
            });
        }
        else
        {
            // error
            if(false == inputIt.value().pClient.expired())
            {
                tProgressNotificationData progressNotificationData
                (
                    inputIt.value().requestId,
                    eRequestState::ERROR_STATE,
                    0,
                    tFoundMatchesPack(),
                    false,
                    tGroupedViewIndices()
                );

                inputIt.value().pClient.lock()->progressNotification(progressNotificationData);
            }

            mRequestDataMap.erase( inputIt );
        }
    }
}

//CContinuousAnalyzer::tRequestData
CContinuousAnalyzer::tRequestData::tRequestData(const tRequestId& requestId_,
            const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient_,
            const tRequestId& subRequestId_,
            bool bIsContinuousAnalysis_,
            const tFileWrapperPtr& pFile,
            const QRegularExpression& regex_,
            const int& numberOfThreads_,
            const tRegexScriptingMetadata& regexScriptingMetadata_,
            const tSearchResultColumnsVisibilityMap& searchColumns_,
            const QString& regexStr_,
            const QStringList& selectedAlises_): requestId(requestId_),
            pClient(pClient_),
            subRequestId(subRequestId_),
            bIsContinuousAnalysis(bIsContinuousAnalysis_),
            fromMessage(0),
            toMessage(0),
            mpFile(pFile),
            regex(regex_),
            regexScriptingMetadata(regexScriptingMetadata_),
            numberOfThreads(numberOfThreads_),
            bContinuousModeActive(false),
            searchColumns(searchColumns_),
            regexStr(regexStr_),
            selectedAlises(selectedAlises_)
{}

PUML_PACKAGE_BEGIN(DMA_Analyzer)
    PUML_CLASS_BEGIN_CHECKED(CSubConsumer)
        PUML_INHERITANCE_CHECKED(IDLTMessageAnalyzerControllerConsumer, implements)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN_CHECKED(CContinuousAnalyzer)
        PUML_INHERITANCE_CHECKED(IDLTMessageAnalyzerController, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(IDLTMessageAnalyzerControllerConsumer, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IDLTMessageAnalyzerController, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
