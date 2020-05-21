/**
 * @file    CDLTMsgWrapper.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CDLTMsgWrapper class
 */
#ifndef CDLTMSGWRAPPER_HPP
#define CDLTMSGWRAPPER_HPP

#include "memory"

#include "QString"

#include "qdlt.h"

/**
 * @brief The CDLTMsgWrapper class - wrapper on top of QDltMsg.
 * Used to extend the QDltMsg with required additional functionality.
 */
class CDLTMsgWrapper
{
public:
    CDLTMsgWrapper();
    CDLTMsgWrapper(const QDltMsg& msg);
    ~CDLTMsgWrapper();

    QString getTimeString() const;
    const unsigned int& getMicroseconds() const;
    const unsigned int& getTimestamp() const;
    const unsigned char& getMessageCounter() const;
    QString getEcuid() const;
    QString getApid() const;
    QString getCtid() const;
    const unsigned int& getSessionid() const;
    QString getTypeString() const;
    QString getSubtypeString() const;
    QString getModeString() const;
    const unsigned int& getNumberOfArguments() const;
    QString getPayload() const;
    QString getCtrlServiceIdString() const;
    QString getCtrlReturnTypeString() const;

    /**
     * @brief getInitialMessageSize - gets size of initial message
     * @return - size of initial dlt msg, which is calculated as sum of its header & payload.
     */
    unsigned int getInitialMessageSize() const;

    //returnes approximal size of structure instance in bytes
    unsigned int getSize() const;
    void dumpSize() const;
    void dumpPayload() const;

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

typedef std::shared_ptr<CDLTMsgWrapper> tDLTMsgWrapperPtr;

#endif // CDLTMSGWRAPPER_HPP
