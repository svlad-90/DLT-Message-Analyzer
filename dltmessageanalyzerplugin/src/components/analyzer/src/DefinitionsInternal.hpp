#pragma once

#include "QRegularExpression"

#include "common/Definitions.hpp"

enum class ePortionAnalysisState
{
    ePortionAnalysisState_SUCCESSFUL,
    ePortionAnalysisState_ERROR
};

struct tPortionRegexAnalysisFinishedData
{
    tPortionRegexAnalysisFinishedData();

    tPortionRegexAnalysisFinishedData(
    const tRequestId& requestId_,
    int numberOfProcessedString_,
    ePortionAnalysisState portionAnalysisState_,
    const tFoundMatchesPack& processedMatches_,
    const tWorkerId& workerId_,
    const tWorkerThreadCookie& workerThreadCookie_,
    bool bUML_Req_Res_Ev_DuplicateFound_);

    tRequestId requestId;
    int numberOfProcessedString;
    ePortionAnalysisState portionAnalysisState;
    tFoundMatchesPack processedMatches;
    tWorkerId workerId;
    tWorkerThreadCookie workerThreadCookie;
    bool bUML_Req_Res_Ev_DuplicateFound;
};

Q_DECLARE_METATYPE(tPortionRegexAnalysisFinishedData)

struct tAnalyzePortionData
{
    tAnalyzePortionData();

    tAnalyzePortionData(
    const tRequestId& requestId_,
    const tProcessingStrings& processingStrings_,
    const QRegularExpression& regex_,
    const tRegexScriptingMetadata& regexMetadata_,
    const tWorkerThreadCookie& workerThreadCookie_ );

    tRequestId requestId;
    tProcessingStrings processingStrings;
    QRegularExpression regex;
    tRegexScriptingMetadata regexMetadata;
    tWorkerThreadCookie workerThreadCookie;
};

Q_DECLARE_METATYPE(tAnalyzePortionData)
