/**
 * @file    CContinuousAnalyzer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CContinuousAnalyzer class
 */

#include "qdlt.h"

#include "CContinuousAnalyzer.hpp"
#include "CMTAnalyzer.hpp"
#include "IDLTMessageAnalyzerControllerConsumer.hpp"
#include "functional"
#include "QTimer"
#include "QDebug"

#include "../dltWrappers/CDLTFileWrapper.hpp"

static const int WAITING_TIME = 50;

CSubConsumer::CSubConsumer(const tDLTMessageAnalyzerControllerPtr& pController):
    IDLTMessageAnalyzerControllerConsumer(pController),
    mCallback()
{}

void CSubConsumer::setCallback(const tCallback& callback)
{
    mCallback = callback;
}

void CSubConsumer::progressNotification( const tRequestId& requestId,
                                         const eRequestState& requestState,
                                         const int8_t& progress,
                                         const tFoundMatchesPack& processedMatches)
{
    if( mCallback )
    {
        mCallback( requestId, requestState, progress, processedMatches );
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

    auto callback = [this](const tRequestId& requestId,
            const eRequestState& requestState,
            const int8_t& progress,
            const tFoundMatchesPack& processedMatches)
    {
        progressNotification( requestId,
                              requestState,
                              progress,
                              processedMatches );
    };

    pConsumer->setCallback( callback );

    mpSubConsumer = pConsumer;
}

void CContinuousAnalyzer::progressNotification(const tRequestId& requestId,
                                               const eRequestState& requestState,
                                               const int8_t& progress,
                                               const tFoundMatchesPack& processedMatches)
{
    auto foundSubRequest = mSubRequestDataMap.find(requestId);

    if(mSubRequestDataMap.end() != foundSubRequest)
    {
        auto foundRequest = mRequestDataMap.find(foundSubRequest.value());

        if(mRequestDataMap.end() != foundRequest)
        {
            if(false == foundRequest.value().bIsContinuousAnalysis)
            {
                if(false == foundRequest.value().pClient.expired())
                {
                    QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                              Q_ARG(tRequestId, foundSubRequest.value()),
                                              Q_ARG(eRequestState, requestState),
                                              Q_ARG(int8_t, progress),
                                              Q_ARG(tFoundMatchesPack, processedMatches));
                }

                if(eRequestState::ERROR_STATE == requestState ||
                        eRequestState::SUCCESSFUL == requestState)
                {
                    mRequestDataMap.erase(foundRequest);
                    mSubRequestDataMap.erase(foundSubRequest);
                }
            }
            else
            {
                switch(requestState)
                {
                    case eRequestState::PROGRESS:
                    {
                        if(false == foundRequest.value().pClient.expired())
                        {
                            int8_t injectedProgress = foundRequest.value().bContinuousModeActive ? 100u : progress;

                            QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                                      Q_ARG(tRequestId, foundSubRequest.value()),
                                                      Q_ARG(eRequestState, requestState),
                                                      Q_ARG(int8_t, injectedProgress),
                                                      Q_ARG(tFoundMatchesPack, processedMatches));
                        }
                    }
                        break;
                    case eRequestState::ERROR_STATE:
                    {
                        if(false == foundRequest.value().pClient.expired())
                        {
                            QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                                      Q_ARG(tRequestId, foundSubRequest.value()),
                                                      Q_ARG(eRequestState, requestState),
                                                      Q_ARG(int8_t, progress),
                                                      Q_ARG(tFoundMatchesPack, processedMatches));
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
                            QMetaObject::invokeMethod(foundRequest.value().pClient.lock().get(), "progressNotification", Qt::QueuedConnection,
                                                      Q_ARG(tRequestId, foundSubRequest.value()),
                                                      Q_ARG(eRequestState, eRequestState::PROGRESS),
                                                      Q_ARG(int8_t, 100),
                                                      Q_ARG(tFoundMatchesPack, processedMatches));
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
        qDebug() << "Warning. Progress notification for irrelevant request id - " << requestId;
    }
}

tRequestId CContinuousAnalyzer::requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                           const tDLTFileWrapperPtr& pFile,
                           const int& fromMessage,
                           const int& numberOfMessages,
                           const QRegularExpression& regex,
                           const int& numberOfThreads,
                           const tRegexScriptingMetadata& regexScriptingMetadata,
                           bool isContinuous )
{
    tRequestId resultRequestId = INVALID_REQUEST_ID;

    if(nullptr != mpSubAnalyzer)
    {
        tRequestId subRequestId = mpSubAnalyzer->requestAnalyze(mpSubConsumer, pFile,fromMessage,numberOfMessages,regex,numberOfThreads,regexScriptingMetadata,isContinuous);

        if(INVALID_REQUEST_ID != subRequestId)
        {
            resultRequestId = ++mRequestIdCounter;
            tRequestData requestData( resultRequestId, pClient, subRequestId, isContinuous, pFile, regex, numberOfThreads, regexScriptingMetadata );
            requestData.fromMessage = fromMessage;
            requestData.toMessage = fromMessage + numberOfMessages;

            mRequestDataMap.insert(resultRequestId, requestData);
            mSubRequestDataMap.insert( subRequestId, resultRequestId );
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
                tRequestId subRequestId = mpSubAnalyzer->requestAnalyze(mpSubConsumer,
                                                                        inputIt->mpFile,
                                                                        inputIt->fromMessage,
                                                                        inputIt->toMessage - inputIt->fromMessage,
                                                                        inputIt->regex,
                                                                        inputIt->numberOfThreads,
                                                                        inputIt->regexScriptingMetadata,
                                                                        inputIt->bIsContinuousAnalysis);

                if(INVALID_REQUEST_ID != subRequestId)
                {
                    mSubRequestDataMap.insert( subRequestId, inputIt->requestId);
                    inputIt.value().subRequestId = subRequestId;
                }
                else
                {
                    if(false == inputIt.value().pClient.expired())
                    {
                        inputIt.value().pClient.lock()->progressNotification(inputIt.value().requestId,
                                                                             eRequestState::ERROR_STATE,
                                                                             0,
                                                                             tFoundMatchesPack());
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
                inputIt.value().pClient.lock()->progressNotification(inputIt.value().requestId,
                                                                     eRequestState::ERROR_STATE,
                                                                     0,
                                                                     tFoundMatchesPack());
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
            const tDLTFileWrapperPtr& pFile,
            const QRegularExpression& regex_,
            const int& numberOfThreads_,
            const tRegexScriptingMetadata& regexScriptingMetadata_): requestId(requestId_),
            pClient(pClient_),
            subRequestId(subRequestId_),
            bIsContinuousAnalysis(bIsContinuousAnalysis_),
            fromMessage(0),
            toMessage(0),
            mpFile(pFile),
            regex(regex_),
            regexScriptingMetadata(regexScriptingMetadata_),
            numberOfThreads(numberOfThreads_),
            bContinuousModeActive(false)
{}
