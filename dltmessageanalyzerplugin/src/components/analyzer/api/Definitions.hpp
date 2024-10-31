#pragma once

#include <set>

#include "stdint.h"

#include "QRegularExpression"

#include "common/Definitions.hpp"

struct tRequestParameters
{
    tRequestParameters();

    /**
     * @param pFile - file, which is used for analysis
     * @param fromMessage - from which message to analyze
     * @param numberOfMessages - until which message to analyze
     * @param regex - regex, to be used for analysis
     * @param numberOfThreads - number of threads to be used for analysis
     * @param isContinuous - whether search is continuous. This parameter
     * can be ignored by one-shot implementations
     * @param searchColumns - the seaarch columns visibility map
     * @param regexStr - regex string that was queried for the search
     * @param selectedAliases - the selected alises that were used to form a regex
     */
    tRequestParameters(
    const tFileWrapperPtr& pFile_,
    const int& fromMessage_,
    const int& numberOfMessages_,
    const QRegularExpression& regex_,
    const int& numberOfThreads_,
    bool isContinuous_,
    const tSearchResultColumnsVisibilityMap& searchColumns_,
    const QString& regexStr_,
    const QStringList& selectedAliases_);

    tFileWrapperPtr pFile;
    int fromMessage;
    int numberOfMessages;
    QRegularExpression regex;
    int numberOfThreads;
    bool isContinuous;
    tSearchResultColumnsVisibilityMap searchColumns;
    QString regexStr;
    QStringList selectedAliases;
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
    bool bUML_Req_Res_Ev_DuplicateFound_,
    const tGroupedViewIndices& groupedViewIndices_
    );

    tRequestId requestId;
    eRequestState requestState;
    int8_t progress;
    tFoundMatchesPack processedMatches;
    bool bUML_Req_Res_Ev_DuplicateFound;
    tGroupedViewIndices groupedViewIndices;
};

Q_DECLARE_METATYPE(tProgressNotificationData)
