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

Q_DECLARE_METATYPE(CDLTRegexAnalyzerWorker::ePortionAnalysisState)

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

    connect( getSettingsManager().get(), &ISettingsManager::searchResultHighlightingGradientChanged,
             [this]( const tHighlightingGradient& gradient )
    {
        mColors = generateColors(gradient);
    });

    mColors = generateColors(getSettingsManager()->getSearchResultHighlightingGradient());
}

tWorkerId CDLTRegexAnalyzerWorker::getWorkerId() const
{
    return mWorkerId;
}

void CDLTRegexAnalyzerWorker::analyzePortion( const tRequestId& requestId,
                                              const tProcessingStrings& processingStrings,
                                              const QRegularExpression& regex,
                                              const tRegexScriptingMetadata& regexMetadata,
                                              const tWorkerThreadCookie& workerThreadCookie)
{
#ifdef DEBUG_BUILD
    SEND_MSG( QString( "[CDLTRegexAnalyzerWorker][%1] reqID - %2; procString.size() - %3; regex - %4 mWorkerId - %5" )
              .arg(__FUNCTION__)
              .arg(requestId)
              .arg(processingStrings.size())
              .arg(regex.pattern())
              .arg(mWorkerId));
#endif

    bool bAnalyzeUML = false;

    if(true == getSettingsManager()->getUML_FeatureActive() &&
       true == regexMetadata.doesContainAnyUMLGroup() &&
       true == regexMetadata.doesContainConsistentUMLData(false).first)
    {
        bAnalyzeUML = true;
    }

    CDLTRegexAnalyzerWorker::ePortionAnalysisState portionAnalysisState = CDLTRegexAnalyzerWorker::ePortionAnalysisState::ePortionAnalysisState_SUCCESSFUL;
    tFoundMatchesPack foundMatchesPack;

    try
    {
#ifdef DEBUG_BUILD
        QElapsedTimer timer;
        timer.start();
#endif

        for(const auto& processingString : processingStrings)
        {
            QRegularExpressionMatch match = regex.match(*(processingString.second));

            if (true == match.hasMatch())
            {
                tFoundMatches foundMatches;

                for (int i = 0; i <= match.lastCapturedIndex(); ++i)
                {
                    const auto& matchItem = match.captured(i);

                    if(i>0)
                    {
                        if(0 != matchItem.size())
                        {
                            foundMatches.push_back( tFoundMatch( std::make_shared<QString>(matchItem),
                                                                 tIntRange( match.capturedStart(i), match.capturedEnd(i) - 1 ),
                                                                 i,
                                                                 processingString.first.msgSize,
                                                                 processingString.first.timeStamp,
                                                                 processingString.first.msgId)  );
                        }
                    }
                }

                tItemMetadata itemMetadata = processingString.first;
                auto pTree = itemMetadata.updateHighlightingInfo(foundMatches, mColors, regexMetadata);

                if(true == bAnalyzeUML)
                {
                    itemMetadata.updateUMLInfo(foundMatches, regexMetadata, pTree);
                }

                foundMatchesPack.matchedItemVec.push_back( tFoundMatchesPackItem( std::move(itemMetadata), std::move(foundMatches) ) );
            }
        }

#ifdef DEBUG_BUILD
        SEND_MSG( QString("[CDLTRegexAnalyzerWorker][%1] Portion number - %2; found matches - %3; portion analysis took - %4 ms")
                  .arg(__FUNCTION__)
                  .arg(workerThreadCookie)
                  .arg(foundMatchesPack.matchedItemVec.size())
                  .arg(QLocale().toString(timer.elapsed()), 4) );
#endif
    }
    catch (const std::exception&)
    {
        portionAnalysisState = CDLTRegexAnalyzerWorker::ePortionAnalysisState::ePortionAnalysisState_ERROR;
    }

    emit portionAnalysisFinished( requestId, static_cast<int>(processingStrings.size()), portionAnalysisState, foundMatchesPack, mWorkerId, workerThreadCookie );
}

PUML_PACKAGE_BEGIN(DMA_Analyzer)
    PUML_CLASS_BEGIN_CHECKED(CDLTRegexAnalyzerWorker)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
