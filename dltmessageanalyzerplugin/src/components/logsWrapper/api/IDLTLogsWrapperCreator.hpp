#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

class QDltFile;
class QDltMsg;
class QDltMessageDecoder;

class IDLTLogsWrapperCreator
{
public:

    IDLTLogsWrapperCreator();
    virtual ~IDLTLogsWrapperCreator();

    virtual tFileWrapperPtr createDLTFileWrapper( QDltFile* pFile ) const = 0;
    virtual tMsgWrapperPtr createDLTMsgWrapper( QDltMsg& msg ) const = 0;

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        virtual tMsgDecoderPtr createMsgDecoder(QDltMessageDecoder* pMessageDecoder) const = 0;
#else
        virtual tMsgDecoderPtr createMsgDecoder(const tPluginPtrList& decoderPlugins) const = 0;
#endif
};
