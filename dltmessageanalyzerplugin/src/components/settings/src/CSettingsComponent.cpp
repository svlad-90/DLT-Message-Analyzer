#include "../api/CSettingsComponent.hpp"

#include "CSettingsManager.hpp"

#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

CSettingsComponent::CSettingsComponent():
mpSettingsManager(nullptr)
{
}

const char* CSettingsComponent::getName() const
{
    return "CSettingsComponent";
}

DMA::tSyncInitOperationResult CSettingsComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        mpSettingsManager = std::make_shared<CSettingsManager>();

        if(nullptr != mpSettingsManager)
        {
            CSettingsManager::tOperationResult setupResult = mpSettingsManager->setUp();

            if(true == setupResult.bResult)
            {
                SEND_MSG(QString("[CSettingsComponent][setUp] set up is successful"));
                result.bIsOperationSuccessful = true;
                result.returnCode = 0;
            }
            else
            {
                SEND_ERR(QString("[CSettingsComponent][setUp] Error - %1").arg(setupResult.err));
                result.bIsOperationSuccessful = false;
                result.returnCode = -1;
            }
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

DMA::tSyncInitOperationResult CSettingsComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpSettingsManager)
        {
            auto result = mpSettingsManager->storeConfigs();

            if(false == result.bResult)
            {
                SEND_ERR(QString("Was not able to store configs due to the following error: %1").arg(result.err));
            }

            mpSettingsManager.reset();
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

const tSettingsManagerPtr& CSettingsComponent::getSettingsManager() const
{
    return mpSettingsManager;
}

PUML_PACKAGE_BEGIN(DMA_Settings_API)
    PUML_CLASS_BEGIN(CSettingsComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, provides)
        PUML_USE_DEPENDENCY_CHECKED(CSettingsManager, 1, 1, creates)
    PUML_CLASS_END()
PUML_PACKAGE_END()
