#include "../api/CLogsWrapperComponent.hpp"

#include "CDLTFileWrapper.hpp"
#include "CDLTMsgWrapper.hpp"
#include "CDLTMsgDecoder.hpp"

#include "qdlt.h"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

CLogsWrapperComponent::CLogsWrapperComponent()
{
    DMA_FORCE_LINK_REFERENCE(IFileWrapper)
    DMA_FORCE_LINK_REFERENCE(IMsgDecoder)
    DMA_FORCE_LINK_REFERENCE(IMsgWrapper)
    DMA_FORCE_LINK_REFERENCE(IDLTLogsWrapperCreator)
}

const char* CLogsWrapperComponent::getName() const
{
    return "CLogsWrapperComponent";
}

tFileWrapperPtr CLogsWrapperComponent::createDLTFileWrapper( QDltFile* pFile ) const
{
    return std::make_shared<CDLTFileWrapper>(pFile);
}

tMsgWrapperPtr CLogsWrapperComponent::createDLTMsgWrapper( QDltMsg& msg ) const
{
    return std::make_shared<CDLTMsgWrapper>(msg);
}

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
tMsgDecoderPtr CLogsWrapperComponent::createMsgDecoder(QDltMessageDecoder* pMessageDecoder) const
{
    return std::make_shared<CDLTMsgDecoder>(pMessageDecoder);
}
#else
tMsgDecoderPtr CLogsWrapperComponent::createMsgDecoder(const tPluginPtrList& decoderPlugins) const
{
    return std::make_shared<CDLTMsgDecoder>(decoderPlugins);
}
#endif

DMA::tSyncInitOperationResult CLogsWrapperComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
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

DMA::tSyncInitOperationResult CLogsWrapperComponent::shutdown()
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

PUML_PACKAGE_BEGIN(DMA_LogsWrapper_API)
    PUML_CLASS_BEGIN(CLogsWrapperComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_INHERITANCE_CHECKED(IDLTLogsWrapperCreator, implements)
        PUML_USE_DEPENDENCY_CHECKED(CDLTFileWrapper, 1, *, creates instances)
        PUML_USE_DEPENDENCY_CHECKED(CDLTMsgWrapper, 1, *, creates instances)
        PUML_USE_DEPENDENCY_CHECKED(CDLTMsgDecoder, 1, *, creates instances)
    PUML_CLASS_END()
PUML_PACKAGE_END()
