#include "../api/CSettingsManagerClient.hpp"

#include "DMA_Plantuml.hpp"

CSettingsManagerClient::CSettingsManagerClient():
mpSettingsManager(nullptr)
{

}

CSettingsManagerClient::CSettingsManagerClient(const tSettingsManagerPtr& pSettingsManager):
mpSettingsManager(pSettingsManager)
{

}

const tSettingsManagerPtr& CSettingsManagerClient::getSettingsManager() const
{
    return mpSettingsManager;
}

void CSettingsManagerClient::setSettingsManager(const tSettingsManagerPtr& pSettingsManager)
{
    mpSettingsManager = pSettingsManager;
    handleSettingsManagerChange();
}

PUML_PACKAGE_BEGIN(DMA_Settings_API)
    PUML_CLASS_BEGIN(CSettingsManagerClient)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
