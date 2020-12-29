/**
 * @file    IDLTMessageAnalyzerController.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the IDLTMessageAnalyzerController class
 */

#include "../api/IDLTMessageAnalyzerController.hpp"

#include "DMA_Plantuml.hpp"

//IDLTMessageAnalyzerController
IDLTMessageAnalyzerController::~IDLTMessageAnalyzerController(){}
IDLTMessageAnalyzerController::IDLTMessageAnalyzerController(){}

PUML_PACKAGE_BEGIN(DMA_Analyzer_API)
    PUML_ABSTRACT_CLASS_BEGIN_CHECKED(IDLTMessageAnalyzerController)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_PURE_VIRTUAL_METHOD( +, void cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient, const tRequestId& requestId ) )
        PUML_PURE_VIRTUAL_METHOD( +, tRequestId requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                                                               const tFileWrapperPtr& pFile,
                                                               const int& fromMessage,
                                                               const int& numberOfMessages,
                                                               const QRegularExpression& regex,
                                                               const int& numberOfThreads,
                                                               const tRegexScriptingMetadata& regexScriptingMetadata,
                                                               bool isContinuous) )
        PUML_PURE_VIRTUAL_METHOD( +, void cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient, const tRequestId& requestId ) )
        PUML_PURE_VIRTUAL_METHOD( +, int getMaximumNumberOfThreads() const )
        PUML_USE_DEPENDENCY_CHECKED(IFileWrapper, 1, 1, uses)
    PUML_ABSTRACT_CLASS_END()
PUML_PACKAGE_END()
