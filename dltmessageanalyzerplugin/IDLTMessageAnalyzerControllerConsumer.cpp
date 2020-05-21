/**
 * @file    IDLTMessageAnalyzerControllerConsumer.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the IDLTMessageAnalyzerControllerConsumer class
 */

#include "IDLTMessageAnalyzerControllerConsumer.hpp"
#include "IDLTMessageAnalyzerController.hpp"
#include "qdlt.h"

#include "CConsoleCtrl.hpp"

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
        bool bParseResult = regexMetadata.parse(regex);

        if(false == bParseResult)
        {
            SEND_ERR(QString("[IDLTMessageAnalyzerControllerConsumer][%1] Was not able to parse scripting metadata out of the regex!").arg(__FUNCTION__));
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
