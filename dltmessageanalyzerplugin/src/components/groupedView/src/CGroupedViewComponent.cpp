#include "../api/CGroupedViewComponent.hpp"

#include "CGroupedViewModel.hpp"

#include "../api/CGroupedView.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CGroupedViewComponent::CGroupedViewComponent( CGroupedView* pGroupedView ):
mpGroupedViewModel(nullptr),
mpGroupedView(pGroupedView)
{
    DMA_FORCE_LINK_REFERENCE(IGroupedViewModel)
}

std::shared_ptr<IGroupedViewModel> CGroupedViewComponent::getGroupedViewModel()
{
    return mpGroupedViewModel;
}

const char* CGroupedViewComponent::getName() const
{
    return "CGroupedViewComponent";
}

DMA::tSyncInitOperationResult CGroupedViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpGroupedView)
        {
            auto pGroupedViewModel= std::make_shared<CGroupedViewModel>();

            if( nullptr != pGroupedViewModel )
            {
                mpGroupedViewModel = pGroupedViewModel;
                mpGroupedView->setModel(pGroupedViewModel.get());
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

DMA::tSyncInitOperationResult CGroupedViewComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpGroupedViewModel)
        {
            mpGroupedViewModel.reset();
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

CGroupedView* CGroupedViewComponent::getGroupedView() const
{
    return mpGroupedView;
}

PUML_PACKAGE_BEGIN(DMA_GroupedView_API)
    PUML_CLASS_BEGIN(CGroupedViewComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(IGroupedViewModel, 1, 1, contains)
        PUML_USE_DEPENDENCY_CHECKED(CGroupedViewModel, 1, 1, using to create IGroupedViewModel)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CGroupedView, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
