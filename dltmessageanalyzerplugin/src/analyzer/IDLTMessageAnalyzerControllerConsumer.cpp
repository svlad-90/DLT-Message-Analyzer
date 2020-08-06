/**
 * @file    IDLTMessageAnalyzerControllerConsumer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the IDLTMessageAnalyzerControllerConsumer class
 */

#include "IDLTMessageAnalyzerControllerConsumer.hpp"
#include "IDLTMessageAnalyzerController.hpp"
#include "qdlt.h"

#include "../settings/CSettingsManager.hpp"
#include "../log/CConsoleCtrl.hpp"

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
