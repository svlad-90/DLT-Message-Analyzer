#include "../api/CUMLViewComponent.hpp"

#include "../api/CUMLView.hpp"

#include "DMA_Plantuml.hpp"

CUMLViewComponent::CUMLViewComponent( CUMLView* pUMLView,
                                      const tSettingsManagerPtr& pSettingsManager,
                                      QPushButton* pUMLCreateDiagramFromTextButton,
                                      QPlainTextEdit* pUMLTextEditor ):
CSettingsManagerClient(pSettingsManager),
mpUMLView(pUMLView),
mpUMLCreateDiagramFromTextButton(pUMLCreateDiagramFromTextButton),
mpUMLTextEditor(pUMLTextEditor)
{
}

const char* CUMLViewComponent::getName() const
{
    return "CUMLViewComponent";
}

DMA::tSyncInitOperationResult CUMLViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpUMLView)
        {
            mpUMLView->setSettingsManager(getSettingsManager());

            if(nullptr != mpUMLCreateDiagramFromTextButton)
            {
                mpUMLView->setUMLCreateDiagramFromTextButton(mpUMLCreateDiagramFromTextButton);
            }

            if(nullptr != mpUMLTextEditor)
            {
                mpUMLView->setUMLTextEditor(mpUMLTextEditor);
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

DMA::tSyncInitOperationResult CUMLViewComponent::shutdown()
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

CUMLView* CUMLViewComponent::getUMLView() const
{
    return mpUMLView;
}

PUML_PACKAGE_BEGIN(DMA_PlantumlView_API)
    PUML_CLASS_BEGIN(CUMLViewComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CUMLView, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
