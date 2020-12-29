#include "../api/IMsgDecoder.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IMsgDecoder::IMsgDecoder()
{

}

IMsgDecoder::~IMsgDecoder()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(IMsgDecoder)

PUML_PACKAGE_BEGIN(DMA_LogsWrapper_API)
    PUML_CLASS_BEGIN(IMsgDecoder)
        PUML_PURE_VIRTUAL_METHOD(+,  void decodeMsg(QDltMsg&, int) )
    PUML_CLASS_END()
PUML_PACKAGE_END()
