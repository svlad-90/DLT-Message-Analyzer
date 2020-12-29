#pragma once

#include "qdlt.h"

#include "common/Definitions.hpp"

#include "../api/IMsgDecoder.hpp"

class CDLTMsgDecoder : public IMsgDecoder
{
public:
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    CDLTMsgDecoder(QDltMessageDecoder* pMessageDecoder);
#else
    CDLTMsgDecoder(const tPluginPtrList& decoderPlugins);
#endif

    void decodeMsg(QDltMsg& msg, int triggeredByUser) override;

private:
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    QDltMessageDecoder* mpMessageDecoder;
#else
    tPluginPtrList mDecoderPlugins;
#endif
};
