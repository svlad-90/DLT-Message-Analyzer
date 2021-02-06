#include "DefinitionsInternal.hpp"

tPortionRegexAnalysisFinishedData::tPortionRegexAnalysisFinishedData():
requestId(INVALID_REQUEST_ID),
numberOfProcessedString(),
portionAnalysisState(),
processedMatches(),
workerId(),
workerThreadCookie(),
bUML_Req_Res_Ev_DuplicateFound(false)
{}

tPortionRegexAnalysisFinishedData::tPortionRegexAnalysisFinishedData(
const tRequestId& requestId_,
int numberOfProcessedString_,
ePortionAnalysisState portionAnalysisState_,
const tFoundMatchesPack& processedMatches_,
const tWorkerId& workerId_,
const tWorkerThreadCookie& workerThreadCookie_,
bool bUML_Req_Res_Ev_DuplicateFound_):
requestId(requestId_),
numberOfProcessedString(numberOfProcessedString_),
portionAnalysisState(portionAnalysisState_),
processedMatches(processedMatches_),
workerId(workerId_),
workerThreadCookie(workerThreadCookie_),
bUML_Req_Res_Ev_DuplicateFound(bUML_Req_Res_Ev_DuplicateFound_)
{}

tAnalyzePortionData::tAnalyzePortionData():
requestId(INVALID_REQUEST_ID),
processingStrings(),
regex(),
regexMetadata(),
workerThreadCookie()
{}

tAnalyzePortionData::tAnalyzePortionData(
const tRequestId& requestId_,
const tProcessingStrings& processingStrings_,
const QRegularExpression& regex_,
const tRegexScriptingMetadata& regexMetadata_,
const tWorkerThreadCookie& workerThreadCookie_ ):
requestId(requestId_),
processingStrings(processingStrings_),
regex(regex_),
regexMetadata(regexMetadata_),
workerThreadCookie(workerThreadCookie_)
{}
