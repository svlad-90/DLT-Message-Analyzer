/**
 * @file    IDLTMessageAnalyzerControllerConsumer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the IDLTMessageAnalyzerControllerConsumer class
 */

#include "../api/IDLTMessageAnalyzerControllerConsumer.hpp"
#include "../api/IDLTMessageAnalyzerController.hpp"
#include "qdlt.h"

#include "components/settings/api/ISettingsManager.hpp"
#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

//IDLTMessageAnalyzerControllerConsumer
IDLTMessageAnalyzerControllerConsumer::~IDLTMessageAnalyzerControllerConsumer()
{
}

IDLTMessageAnalyzerControllerConsumer::IDLTMessageAnalyzerControllerConsumer( const std::weak_ptr<IDLTMessageAnalyzerController>& pController ):
    std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>(),
    mpController(pController),
    mbGroupedViewFeatureActiveForCurrentAnalysis(false)
{

}

tRequestId IDLTMessageAnalyzerControllerConsumer::requestAnalyze( const tRequestParameters& requestParameters,
                                                                  bool bUMLFeatureActive,
                                                                  bool bPlotViewFeatureActive,
                                                                  bool bGroupedViewFeatureActive )
{
    tRequestId requestId = INVALID_REQUEST_ID;

    if(false == mpController.expired())
    {
        tRegexScriptingMetadata regexMetadata;

        bool bParseResult = regexMetadata.parse(requestParameters.regex,
                                                bUMLFeatureActive,
                                                bPlotViewFeatureActive,
                                                bGroupedViewFeatureActive);

        if(false == bParseResult)
        {
            SEND_ERR(QString("[IDLTMessageAnalyzerControllerConsumer][%1] Was not able to parse scripting metadata out of the regex!").arg(__FUNCTION__));
        }
        else
        {
            if(true == bUMLFeatureActive)
            {
                if(true == regexMetadata.doesContainAnyUMLGroup()) // if has at least one UML group
                {
                    auto checkUMLDataResult = regexMetadata.doesContainConsistentUMLData(true);

                    if(false == checkUMLDataResult.first) // let's check and trace the warnings
                    {
                        SEND_WRN(checkUMLDataResult.second);
                    }
                }
            }

            if(true == bPlotViewFeatureActive)
            {
                if(true == regexMetadata.doesContainAnyPlotViewGroup()) // if has at least one plot view group
                {
                    auto checkPlotViewDataResult = regexMetadata.doesContainConsistentPlotViewData(true, true);

                    if(false == checkPlotViewDataResult.first) // let's check and trace the warnings
                    {
                        SEND_WRN(checkPlotViewDataResult.second);
                    }
                }
            }
        }

        requestId = mpController.lock()->requestAnalyze(shared_from_this(), requestParameters, regexMetadata);
        mbGroupedViewFeatureActiveForCurrentAnalysis = bGroupedViewFeatureActive;
    }

    return requestId;
}

bool IDLTMessageAnalyzerControllerConsumer::isGroupedViewFeatureActiveForCurrentAnalysis() const
{
    return mbGroupedViewFeatureActiveForCurrentAnalysis;
}

void IDLTMessageAnalyzerControllerConsumer::cancelRequest( const tRequestId& requestId )
{
    if(false == mpController.expired())
    {
        mpController.lock()->cancelRequest( shared_from_this(), requestId );
    }
}

PUML_PACKAGE_BEGIN(DMA_Analyzer_API)
    PUML_CLASS_BEGIN_CHECKED(std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>)
    PUML_CLASS_END()

    PUML_ABSTRACT_CLASS_BEGIN_CHECKED(IDLTMessageAnalyzerControllerConsumer)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_INHERITANCE_CHECKED(std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>, extends)
        PUML_PURE_VIRTUAL_METHOD( +, slot void progressNotification( const tProgressNotificationData& progressNotificationData ) )
        PUML_METHOD( +, tRequestId requestAnalyze( const tRequestParameters& requestParameters ) )
        PUML_AGGREGATION_DEPENDENCY(IDLTMessageAnalyzerController, 1, 1, uses)
        PUML_USE_DEPENDENCY_CHECKED(IFileWrapper, 1, 1, uses)
    PUML_ABSTRACT_CLASS_END()
PUML_PACKAGE_END()
