/**
 * @file    CDLTMsgWrapper.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CDLTMsgWrapper class
 */
#pragma once

#include "memory"

#include "QString"

#include "qdlt.h"

#include "../api/IMsgWrapper.hpp"

/**
 * @brief The CDLTMsgWrapper class - wrapper on top of QDltMsg.
 * Used to extend the QDltMsg with required additional functionality.
 */
class CDLTMsgWrapper : public IMsgWrapper
{
public:
    CDLTMsgWrapper();
    CDLTMsgWrapper(const QDltMsg& msg);
    ~CDLTMsgWrapper();

    QString getTimeString() const override;
    const unsigned int& getMicroseconds() const override;
    const unsigned int& getTimestamp() const override;
    const unsigned char& getMessageCounter() const override;
    QString getEcuid() const override;
    QString getApid() const override;
    QString getCtid() const override;
    const unsigned int& getSessionid() const override;
    QString getTypeString() const override;
    QString getSubtypeString() const override;
    QString getModeString() const override;
    const unsigned int& getNumberOfArguments() const override;
    QString getPayload() const override;
    QString getCtrlServiceIdString() const override;
    QString getCtrlReturnTypeString() const override;

    /**
     * @brief getInitialMessageSize - gets size of initial message
     * @return - size of initial dlt msg, which is calculated as sum of its header & payload.
     */
    unsigned int getInitialMessageSize() const override;

    //returnes approximal size of structure instance in bytes
    unsigned int getSize() const override;
    void dumpSize() const override;
    void dumpPayload() const override;

    CDLTMsgWrapper( const CDLTMsgWrapper& ) = delete;
    CDLTMsgWrapper( CDLTMsgWrapper&& ) = delete;
    CDLTMsgWrapper& operator=( const CDLTMsgWrapper& ) = delete;
    CDLTMsgWrapper& operator=( CDLTMsgWrapper&& ) = delete;

private:

    unsigned int mMicroseconds;
    unsigned int mTimestamp;
    unsigned int mMessageId;
    unsigned int mCtrlServiceId;
    unsigned int mSessionid;
    int mSubtype;
    unsigned int mNumberOfArguments;
    time_t mTime;

    QByteArray mEcuidUTF8;
    QByteArray mApidUTF8;
    QByteArray mCtidUTF8;
    QByteArray mPayloadUTF8;

    QDltMsg::DltTypeDef mType;
    QDltMsg::DltModeDef mMode;
    unsigned char mCtrlReturnType;
    unsigned char mMessageCounter;
    QDlt::DltEndiannessDef mEndianess;

    unsigned int mInitialMsgSize;
};
