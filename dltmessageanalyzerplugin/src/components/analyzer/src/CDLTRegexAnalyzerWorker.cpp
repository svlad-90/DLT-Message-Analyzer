/**
 * @file    CDLTRegexAnalyzerWorker.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTRegexAnalyzerWorker class
 */

#include "atomic"

#include "QDebug"

#include "common/Definitions.hpp"

#ifdef DEBUG_BUILD
#include "QElapsedTimer"
#endif

#include "components/settings/api/ISettingsManager.hpp"
#include "components/log/api/CLog.hpp"
#include "CDLTRegexAnalyzerWorker.hpp"

#include "DMA_Plantuml.hpp"

Q_DECLARE_METATYPE(ePortionAnalysisState)

static std::atomic<tWorkerId> sWorkerIdCounter(0);

//CDLTRegexAnalyzerWorker
CDLTRegexAnalyzerWorker::CDLTRegexAnalyzerWorker(const tSettingsManagerPtr& pSettingsManager):
CSettingsManagerClient(pSettingsManager),
mWorkerId(++sWorkerIdCounter),
mColors()
{
    qRegisterMetaType<tFoundMatchesPack>("tFoundMatchesPack");
    qRegisterMetaType<ePortionAnalysisState>("ePortionAnalysisState");
    qRegisterMetaType<tWorkerId>("tWorkerId");
    qRegisterMetaType<int8_t>("int8_t");
    qRegisterMetaType<tWorkerThreadCookie>("tWorkerThreadCookie");
    qRegisterMetaType<tRegexScriptingMetadata>("tRegexScriptingMetadata");
    qRegisterMetaType<tRequestParameters>("tRequestParameters");
    qRegisterMetaType<tProgressNotificationData>("tProgressNotificationData");
    qRegisterMetaType<tPortionRegexAnalysisFinishedData>("tPortionRegexAnalysisFinishedData");
    qRegisterMetaType<tAnalyzePortionData>("tAnalyzePortionData");

    connect( getSettingsManager().get(), &ISettingsManager::searchResultHighlightingGradientChanged,
             this, [this]( const tHighlightingGradient& gradient )
    {
        mColors = generateColors(gradient);
    });

    mColors = generateColors(getSettingsManager()->getSearchResultHighlightingGradient());
}

tWorkerId CDLTRegexAnalyzerWorker::getWorkerId() const
{
    return mWorkerId;
}

void CDLTRegexAnalyzerWorker::analyzePortion(  const tAnalyzePortionData& analyzePortionData )
{
#ifdef DEBUG_BUILD
    SEND_MSG( QString( "[CDLTRegexAnalyzerWorker][%1] reqID - %2; procString.size() - %3; regex - %4 mWorkerId - %5" )
              .arg(__FUNCTION__)
              .arg(analyzePortionData.requestId)
              .arg(analyzePortionData.processingStrings.size())
              .arg(analyzePortionData.regex.pattern())
              .arg(mWorkerId));
#endif

    bool bAnalyzeUML = false;

    if(true == getSettingsManager()->getUML_FeatureActive() &&
       true == analyzePortionData.regexMetadata.doesContainAnyUMLGroup() &&
       true == analyzePortionData.regexMetadata.doesContainConsistentUMLData(false).first)
    {
        bAnalyzeUML = true;
    }

    bool bAnalyzePlotView = false;

    if(true == getSettingsManager()->getPlotViewFeatureActive() &&
        true == analyzePortionData.regexMetadata.doesContainAnyPlotViewGroup() &&
        true == analyzePortionData.regexMetadata.doesContainConsistentPlotViewData(false, true).first)
    {
        bAnalyzePlotView = true;
    }

    ePortionAnalysisState portionAnalysisState = ePortionAnalysisState::ePortionAnalysisState_SUCCESSFUL;
    tFoundMatchesPack foundMatchesPack;

    bool bUML_Req_Res_Ev_DuplicateFound = false;

    try
    {
#ifdef DEBUG_BUILD
        QElapsedTimer timer;
        timer.start();
#endif

        for(const auto& processingString : analyzePortionData.processingStrings)
        {
            QRegularExpressionMatch match = analyzePortionData.regex.match(*(processingString.second));

            if (true == match.hasMatch())
            {
                tFoundMatches foundMatches(processingString.first.msgSize,
                                           processingString.first.timeStamp,
                                           processingString.first.msgId);

                int foundMatchesVecCapacity = 0;

                for (int i = 0; i <= match.lastCapturedIndex(); ++i)
                {
                    if(i>0)
                    {
                        if(0 != match.capturedLength(i))
                        {
                            ++foundMatchesVecCapacity;
                        }
                    }
                }

                foundMatches.foundMatchesVec.reserve(foundMatchesVecCapacity);

                for (int i = 0; i <= match.lastCapturedIndex(); ++i)
                {
                    if(i>0)
                    {
                        if(0 != match.capturedLength(i))
                        {
                            const auto& matchItem = match.captured(i);
                            foundMatches.foundMatchesVec.emplace_back( tFoundMatch( matchItem,
                                                                 tIntRange( match.capturedStart(i), match.capturedEnd(i) - 1 ),
                                                                 i)  );
                        }
                    }
                }

                tItemMetadata itemMetadata = processingString.first;
                auto pTree = itemMetadata.updateHighlightingInfo(foundMatches,
                                                                 mColors,
                                                                 analyzePortionData.regexMetadata);

                if(true == bAnalyzeUML)
                {
                    auto updateUMLInfoResult = itemMetadata.updateUMLInfo(foundMatches,
                                               analyzePortionData.regexMetadata,
                                               pTree);

                    if(false == bUML_Req_Res_Ev_DuplicateFound)
                    {
                        bUML_Req_Res_Ev_DuplicateFound = updateUMLInfoResult.bUML_Req_Res_Ev_DuplicateFound;
                    }
                }

                if(true == bAnalyzePlotView)
                {
                    auto updatePlotViewInfoResult = itemMetadata.updatePlotViewInfo(foundMatches,
                                                                          analyzePortionData.regexMetadata,
                                                                          pTree);
                }

                foundMatchesPack.matchedItemVec.push_back( std::make_shared<tFoundMatchesPackItem>( std::move(itemMetadata), std::move(foundMatches) ) );
            }
        }

#ifdef DEBUG_BUILD
        SEND_MSG( QString("[CDLTRegexAnalyzerWorker][%1] Portion number - %2; found matches - %3; portion analysis took - %4 ms")
                  .arg(__FUNCTION__)
                  .arg(analyzePortionData.workerThreadCookie)
                  .arg(foundMatchesPack.matchedItemVec.size())
                  .arg(QLocale().toString(timer.elapsed()), 4) );
#endif
    }
    catch (const std::exception&)
    {
        portionAnalysisState = ePortionAnalysisState::ePortionAnalysisState_ERROR;
    }

    tPortionRegexAnalysisFinishedData portionRegexAnalysisFinishedData(
    analyzePortionData.requestId,
    static_cast<int>(analyzePortionData.processingStrings.size()),
    portionAnalysisState,
    foundMatchesPack,
    mWorkerId,
    analyzePortionData.workerThreadCookie,
    bUML_Req_Res_Ev_DuplicateFound);

    emit portionAnalysisFinished( portionRegexAnalysisFinishedData );
}

PUML_PACKAGE_BEGIN(DMA_Analyzer)
    PUML_CLASS_BEGIN_CHECKED(CDLTRegexAnalyzerWorker)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
