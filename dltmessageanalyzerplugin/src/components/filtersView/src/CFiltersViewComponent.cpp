#include "../api/CFiltersViewComponent.hpp"

#include "CFiltersModel.hpp"

#include "../api/CFiltersView.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CFiltersViewComponent::CFiltersViewComponent( CFiltersView* pFiltersView,
                                              const tSettingsManagerPtr& pSettingsManager ):
CSettingsManagerClient(pSettingsManager),
mpFiltersModel(nullptr),
mpFiltersView(pFiltersView)
{
    DMA_FORCE_LINK_REFERENCE(IFiltersModel)
}

std::shared_ptr<IFiltersModel> CFiltersViewComponent::getFiltersModel()
{
    return mpFiltersModel;
}

const char* CFiltersViewComponent::getName() const
{
    return "CFiltersViewComponent";
}

DMA::tSyncInitOperationResult CFiltersViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpFiltersView)
        {
            auto pFiltersModel= std::make_shared<CFiltersModel>(getSettingsManager());

            if( nullptr != pFiltersModel )
            {
                mpFiltersView->setSettingsManager(getSettingsManager());
                mpFiltersModel = pFiltersModel;
                mpFiltersView->setSpecificModel(pFiltersModel.get());
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

DMA::tSyncInitOperationResult CFiltersViewComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpFiltersModel)
        {
            mpFiltersModel.reset();
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

CFiltersView* CFiltersViewComponent::getFiltersView() const
{
    return mpFiltersView;
}

PUML_PACKAGE_BEGIN(DMA_FiltersView_API)
    PUML_CLASS_BEGIN(CFiltersViewComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(IFiltersModel, 1, 1, contains)
        PUML_USE_DEPENDENCY_CHECKED(CFiltersModel, 1, 1, using to create IFiltersModel)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CFiltersView, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
