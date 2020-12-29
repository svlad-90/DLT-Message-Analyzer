#include "CDLTMsgDecoder.hpp"

#include "DMA_Plantuml.hpp"

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    CDLTMsgDecoder::CDLTMsgDecoder(QDltMessageDecoder* pMessageDecoder):
    mpMessageDecoder(pMessageDecoder)
#else
    CDLTMsgDecoder::CDLTMsgDecoder(const tPluginPtrList& decoderPlugins):
    mDecoderPlugins(decoderPlugins)
#endif
{
}

void CDLTMsgDecoder::decodeMsg(QDltMsg& msg, int triggeredByUser)
{
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    if(nullptr != mpMessageDecoder)
    {
        mpMessageDecoder->decodeMsg(msg,triggeredByUser);
    }
#else
    for(auto* pPlugin: mDecoderPlugins)
    {
        pPlugin->decodeMsg(msg,triggeredByUser);
    }
#endif
}

PUML_PACKAGE_BEGIN(DMA_LogsWrapper)
    PUML_CLASS_BEGIN_CHECKED(CDLTMsgDecoder)
        PUML_INHERITANCE_CHECKED(IMsgDecoder, implements)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QDltMsg, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
