#include "../api/IMsgWrapper.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IMsgWrapper::IMsgWrapper()
{

}

IMsgWrapper::~IMsgWrapper()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(IMsgWrapper)

PUML_PACKAGE_BEGIN(DMA_LogsWrapper_API)
    PUML_CLASS_BEGIN(IMsgWrapper)
        PUML_PURE_VIRTUAL_METHOD(+, QString getTimeString() const )
        PUML_PURE_VIRTUAL_METHOD(+, const unsigned int& getMicroseconds() const )
        PUML_PURE_VIRTUAL_METHOD(+, const unsigned int& getTimestamp() const )
        PUML_PURE_VIRTUAL_METHOD(+, const unsigned char& getMessageCounter() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getEcuid() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getApid() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getCtid() const )
        PUML_PURE_VIRTUAL_METHOD(+, const unsigned int& getSessionid() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getTypeString() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getSubtypeString() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getModeString() const )
        PUML_PURE_VIRTUAL_METHOD(+, const unsigned int& getNumberOfArguments() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getPayload() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getCtrlServiceIdString() const )
        PUML_PURE_VIRTUAL_METHOD(+, QString getCtrlReturnTypeString() const )
        PUML_PURE_VIRTUAL_METHOD(+, unsigned int getInitialMessageSize() const )
        PUML_PURE_VIRTUAL_METHOD(+, unsigned int getSize() const )
        PUML_PURE_VIRTUAL_METHOD(+, void dumpSize() const )
        PUML_PURE_VIRTUAL_METHOD(+, void dumpPayload() const )
    PUML_CLASS_END()
PUML_PACKAGE_END()
