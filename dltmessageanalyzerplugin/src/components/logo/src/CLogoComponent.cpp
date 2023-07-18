#include "../api/CLogoComponent.hpp"

#include "../api/CLogo.hpp"

#include "DMA_Plantuml.hpp"

CLogoComponent::CLogoComponent( CLogo* pLogoView ):
mpLogoView(pLogoView)
{
}

const char* CLogoComponent::getName() const
{
    return "CLogoComponent";
}

DMA::tSyncInitOperationResult CLogoComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpLogoView)
        {
            // add logic here, when it will be needed
            result.bIsOperationSuccessful = true;
            result.returnCode = 0;
        }
        else
        {
            result.bIsOperationSuccessful = false;
            result.returnCode = -1;
        }
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

DMA::tSyncInitOperationResult CLogoComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        // add logic here, when it will be needed
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

PUML_PACKAGE_BEGIN(DMA_Logo_API)
    PUML_CLASS_BEGIN(CLogoComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CLogo, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
