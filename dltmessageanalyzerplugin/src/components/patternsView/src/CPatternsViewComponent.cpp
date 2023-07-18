#include "../api/CPatternsViewComponent.hpp"

#include "CPatternsModel.hpp"

#include "../api/CPatternsView.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CPatternsViewComponent::CPatternsViewComponent( CPatternsView* pPatternsView,
                                                const tSettingsManagerPtr& pSettingsManager ):
CSettingsManagerClient(pSettingsManager),
mpPatternsModel(nullptr),
mpPatternsView(pPatternsView)
{
    DMA_FORCE_LINK_REFERENCE(IPatternsModel)
}

std::shared_ptr<IPatternsModel> CPatternsViewComponent::getPatternsModel()
{
    return mpPatternsModel;
}

const char* CPatternsViewComponent::getName() const
{
    return "CPatternsViewComponent";
}

DMA::tSyncInitOperationResult CPatternsViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpPatternsView)
        {
            auto pPatternsModel= std::make_shared<CPatternsModel>(getSettingsManager());

            if( nullptr != pPatternsModel )
            {
                mpPatternsView->setSettingsManager(getSettingsManager());
                mpPatternsModel = pPatternsModel;
                mpPatternsView->setSpecificModel(pPatternsModel.get());
            }

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

DMA::tSyncInitOperationResult CPatternsViewComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpPatternsModel)
        {
            mpPatternsModel.reset();
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

CPatternsView* CPatternsViewComponent::getPatternsView() const
{
    return mpPatternsView;
}

PUML_PACKAGE_BEGIN(DMA_PatternsView_API)
    PUML_CLASS_BEGIN(CPatternsViewComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(IPatternsModel, 1, 1, contains)
        PUML_USE_DEPENDENCY_CHECKED(CPatternsModel, 1, 1, using to create IPatternsModel)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CPatternsView, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
