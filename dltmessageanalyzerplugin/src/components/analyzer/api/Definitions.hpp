#pragma once

#include "stdint.h"

#include "QRegularExpression"

#include "common/Definitions.hpp"

struct tRequestParameters
{
    tRequestParameters();

    /**
     * @param pClient - client, which should be notified about the progress
     * @param pFile - file, which is used for analysis
     * @param fromMessage - from which message to analyze
     * @param numberOfMessages - until which message to analyze
     * @param regex - regex, to be used for analysis
     * @param numberOfThreads - number of threads to be used for analysis
     * @param regexScriptingMetadata - scripting metadata
     * @param isContinuous - whether search is continuous. This parameter can be ignored by one-shot implementations
     */
    tRequestParameters(
    const tFileWrapperPtr& pFile_,
    const int& fromMessage_,
    const int& numberOfMessages_,
    const QRegularExpression& regex_,
    const int& numberOfThreads_,
    bool isContinuous_);

    tFileWrapperPtr pFile;
    int fromMessage;
    int numberOfMessages;
    QRegularExpression regex;
    int numberOfThreads;
    bool isContinuous;
};

Q_DECLARE_METATYPE(tRequestParameters)

struct tProgressNotificationData
{
    tProgressNotificationData();

   /*
    * @param requestId - id of the request, about which we notify the client
    * @param requestState - the state of the request
    * @param progress - progress in percents
    * @param processedMatches - found matches
    * @param isContinuous - whether analysis is continuous.
    */
    tProgressNotificationData(
    const tRequestId& requestId_,
    const eRequestState& requestState_,
    const int8_t& progress_,
    const tFoundMatchesPack& processedMatches_,
    bool bUML_Req_Res_Ev_DuplicateFound_);

    tRequestId requestId;
    eRequestState requestState;
    int8_t progress;
    tFoundMatchesPack processedMatches;
    bool bUML_Req_Res_Ev_DuplicateFound;
};

Q_DECLARE_METATYPE(tProgressNotificationData)
