#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "IDLTLogsWrapperCreator.hpp"

class QDltFile;
class QDltMsg;
class QDltMessageDecoder;

class CLogsWrapperComponent : public DMA::IComponent, public IDLTLogsWrapperCreator
{
public:

    CLogsWrapperComponent();

    virtual const char* getName() const override;

    tFileWrapperPtr createDLTFileWrapper( QDltFile* pFile ) const override;
    tMsgWrapperPtr createDLTMsgWrapper( QDltMsg& msg ) const override;

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        tMsgDecoderPtr createMsgDecoder(QDltMessageDecoder* pMessageDecoder) const override;
#else
        tMsgDecoderPtr createMsgDecoder(const tPluginPtrList& decoderPlugins) const override;
#endif

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;
};
