#include "../api/CAnalyzerComponent.hpp"

#include "CContinuousAnalyzer.hpp"
#include "CMTAnalyzer.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CAnalyzerComponent::CAnalyzerComponent():
mpMessageAnalyzerController(nullptr)
{
    // force linkage references in order to have consistent diagrams
    DMA_FORCE_LINK_REFERENCE(INamedObject)
}

const char* CAnalyzerComponent::getName() const
{
    return "CAnalyzerComponent";
}

DMA::tSyncInitOperationResult CAnalyzerComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        auto pMTController = IDLTMessageAnalyzerController::createInstance<CMTAnalyzer>();
        mpMessageAnalyzerController = IDLTMessageAnalyzerController::createInstance<CContinuousAnalyzer>(pMTController);

        result.bIsOperationSuccessful = true;
        result.returnCode = 0;
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

DMA::tSyncInitOperationResult CAnalyzerComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpMessageAnalyzerController)
        {
            mpMessageAnalyzerController.reset();
        }

        result.bIsOperationSuccessful = true;
        result.returnCode = 0;
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

std::shared_ptr<IDLTMessageAnalyzerController>
CAnalyzerComponent::getAnalyzerController() const
{
    return mpMessageAnalyzerController;
}

PUML_PACKAGE_BEGIN(DMA_Analyzer_API)
    PUML_CLASS_BEGIN(CAnalyzerComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CContinuousAnalyzer, 1, 1, contains)
        PUML_USE_DEPENDENCY_CHECKED(CMTAnalyzer, 1, 1, creates and feeds into CContinuousAnalyzer)
    PUML_CLASS_END()
PUML_PACKAGE_END()
