/**
 * @file    IDLTMessageAnalyzerControllerConsumer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the IDLTMessageAnalyzerControllerConsumer class
 */

#include "IDLTMessageAnalyzerControllerConsumer.hpp"
#include "IDLTMessageAnalyzerController.hpp"
#include "qdlt.h"

#include "../settings/CSettingsManager.hpp"
#include "../log/CLog.hpp"

#include "DMA_Plantuml.hpp"

//IDLTMessageAnalyzerControllerConsumer
IDLTMessageAnalyzerControllerConsumer::~IDLTMessageAnalyzerControllerConsumer()
{
}

IDLTMessageAnalyzerControllerConsumer::IDLTMessageAnalyzerControllerConsumer( const std::weak_ptr<IDLTMessageAnalyzerController>& pController ):
    std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>(),
    mpController(pController)
{

}

tRequestId IDLTMessageAnalyzerControllerConsumer::requestAnalyze( const tDLTFileWrapperPtr& pFile,
                                                                  const int& fromMessage,
                                                                  const int& numberOfMessages,
                                                                  const QRegularExpression& regex,
                                                                  const int& numberOfThreads,
                                                                  bool isContinuous)
{
    tRequestId requestId = INVALID_REQUEST_ID;

    if(false == mpController.expired())
    {
        tRegexScriptingMetadata regexMetadata;

        bool bUMLFeatureActive = CSettingsManager::getInstance()->getUML_FeatureActive();

        bool bParseResult = regexMetadata.parse(regex, bUMLFeatureActive);

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
        }

        requestId = mpController.lock()->requestAnalyze(shared_from_this(), pFile, fromMessage, numberOfMessages, regex, numberOfThreads, regexMetadata, isContinuous);
    }

    return requestId;
}

void IDLTMessageAnalyzerControllerConsumer::cancelRequest( const tRequestId& requestId )
{
    if(false == mpController.expired())
    {
        mpController.lock()->cancelRequest( shared_from_this(), requestId );
    }
}

PUML_PACKAGE_BEGIN(DMA_Analyzer)
    PUML_CLASS_BEGIN_CHECKED(std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>)
    PUML_CLASS_END()

    PUML_ABSTRACT_CLASS_BEGIN_CHECKED(IDLTMessageAnalyzerControllerConsumer)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_INHERITANCE_CHECKED(std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>, extends)
        PUML_PURE_VIRTUAL_METHOD( +, void progressNotification( const tRequestId& requestId,
                                                                const eRequestState& requestState,
                                                                const int8_t& progress,
                                                                const tFoundMatchesPack& processedMatches) )
        PUML_METHOD( +, tRequestId requestAnalyze( const tDLTFileWrapperPtr& pFile,
                                                   const int& fromMessage,
                                                   const int& numberOfMessages,
                                                   const QRegularExpression& regex,
                                                   const int& numberOfThreads,
                                                   bool isContinuous ) )
        PUML_AGGREGATION_DEPENDENCY(IDLTMessageAnalyzerController, 1, 1, uses)
    PUML_ABSTRACT_CLASS_END()
PUML_PACKAGE_END()
