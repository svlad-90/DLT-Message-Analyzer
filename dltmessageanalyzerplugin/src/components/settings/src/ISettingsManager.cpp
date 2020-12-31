#include "../api/ISettingsManager.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

ISettingsManager::ISettingsManager()
{

}

ISettingsManager::~ISettingsManager()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(ISettingsManager)

PUML_PACKAGE_BEGIN(DMA_Settings_API)
    PUML_CLASS_BEGIN(ISettingsManager)
        PUML_INHERITANCE_CHECKED(QObject, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
