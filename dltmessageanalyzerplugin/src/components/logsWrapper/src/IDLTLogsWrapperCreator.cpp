#include "../api/IDLTLogsWrapperCreator.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IDLTLogsWrapperCreator::IDLTLogsWrapperCreator()
{

}

IDLTLogsWrapperCreator::~IDLTLogsWrapperCreator()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(IDLTLogsWrapperCreator)

PUML_PACKAGE_BEGIN(DMA_LogsWrapper_API)
    PUML_CLASS_BEGIN(IDLTLogsWrapperCreator)

        PUML_PURE_VIRTUAL_METHOD(+,  tFileWrapperPtr createDLTFileWrapper( QDltFile* pFile ) const )
        PUML_PURE_VIRTUAL_METHOD(+,  tMsgWrapperPtr createDLTMsgWrapper( QDltMsg& msg ) const )

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        PUML_PURE_VIRTUAL_METHOD(+,  tMsgDecoderPtr createMsgDecoder(QDltMessageDecoder* pMessageDecoder) const )
#else
        PUML_PURE_VIRTUAL_METHOD(+,  tMsgDecoderPtr createMsgDecoder(const tPluginPtrList& decoderPlugins) const )
#endif

        PUML_USE_DEPENDENCY_CHECKED(QDltFile, 1, *, uses)
        PUML_USE_DEPENDENCY_CHECKED(QDltMsg, 1, *, uses)

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        PUML_USE_DEPENDENCY_CHECKED(QDltMessageDecoder, 1, 1, uses)
#else
        PUML_USE_DEPENDENCY_CHECKED(QDltPlugin, 1, *, uses)
#endif

        PUML_USE_DEPENDENCY_CHECKED(IMsgWrapper, 1, *, creates)
        PUML_USE_DEPENDENCY_CHECKED(IMsgDecoder, 1, *, creates)
        PUML_USE_DEPENDENCY_CHECKED(IFileWrapper, 1, *, creates)

    PUML_CLASS_END()
PUML_PACKAGE_END()
