/**
 * @file    CDLTMsgWrapper.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTMsgWrapper class
 */

//#include <atomic>

#include "QDebug"

#include "dlt_protocol.h"
#include "dlt_common.h"

#include "components/log/api/CLog.hpp"

#include "CDLTMsgWrapper.hpp"

#include "DMA_Plantuml.hpp"

//static std::atomic<int> sInstanceCounter(0);

CDLTMsgWrapper::CDLTMsgWrapper():
mMicroseconds(0),
mTimestamp(0),
mMessageId(0),
mCtrlServiceId(0),
mSessionid(0),
mSubtype(),
mNumberOfArguments(0),
mTime(0),
mEcuidUTF8(),
mApidUTF8(),
mCtidUTF8(),
mPayloadUTF8(),
mType(),
mMode(),
mCtrlReturnType(),
mMessageCounter(0),
mEndianess(),
mInitialMsgSize(0u)
{
    mEcuidUTF8.squeeze();
    mApidUTF8.squeeze();
    mCtidUTF8.squeeze();
    mPayloadUTF8.squeeze();

    //++sInstanceCounter;
    //qDebug() << "CDLTMsgWrapper::CDLTMsgWrapper: number of instances - " << sInstanceCounter;
}

static const QRegularExpression sReplaceRegex("\n|\\x{0000}|\\x{0001}|\\x{0002}|\\x{0003}|\\x{0004}|\\x{0005}"
                                 "|\\x{0006}|\\x{0007}|\\x{0008}|\\x{0009}|\\x{000A}|\\x{000B}"
                                 "|\\x{000C}|\\x{000D}|\\x{000E}|\\x{000F}|\\x{0010}|\\x{0011}"
                                 "|\\x{0012}|\\x{0013}|\\x{0014}|\\x{0015}|\\x{0016}|\\x{0017}"
                                 "|\\x{0018}|\\x{0019}|\\x{001A}|\\x{001B}|\\x{001C}|\\x{001D}"
                                 "|\\x{001E}|\\x{001F}");

CDLTMsgWrapper::CDLTMsgWrapper(const QDltMsg& msg):
mMicroseconds(msg.getMicroseconds()),
mTimestamp(msg.getTimestamp()),
mMessageId(msg.getMessageId()),
mCtrlServiceId(msg.getCtrlServiceId()),
mSessionid(msg.getSessionid()),
mSubtype(msg.getSubtype()),
mNumberOfArguments(msg.getNumberOfArguments()),
mTime(msg.getTime()),
mEcuidUTF8(msg.getEcuid().toUtf8()),
mApidUTF8(msg.getApid().toUtf8()),
mCtidUTF8(msg.getCtid().toUtf8()),
mPayloadUTF8(msg.toStringPayload().replace(sReplaceRegex, "").toUtf8()),
mType(msg.getType()),
mMode(msg.getMode()),
mCtrlReturnType(msg.getCtrlReturnType()),
mMessageCounter(msg.getMessageCounter()),
mEndianess(msg.getEndianness()),
mInitialMsgSize(static_cast<unsigned int>(msg.getPayloadSize() + msg.getHeaderSize()))
{
    mEcuidUTF8.squeeze();
    mApidUTF8.squeeze();
    mCtidUTF8.squeeze();
    mPayloadUTF8.squeeze();

    //qDebug() << "mEcuidUTF8.capacity - " << mEcuidUTF8.capacity() << "; mEcuidUTF8.size - " << mEcuidUTF8.size();
    //qDebug() << "mApidUTF8.capacity - " << mApidUTF8.capacity() << "; mApidUTF8.size - " << mApidUTF8.size();
    //qDebug() << "mCtidUTF8.capacity - " << mCtidUTF8.capacity() << "; mCtidUTF8.size - " << mCtidUTF8.size();
    //qDebug() << "mPayloadUTF8.capacity - " << mPayloadUTF8.capacity() << "; mPayloadUTF8.size - " << mPayloadUTF8.size();

    //++sInstanceCounter;
    //qDebug() << "CDLTMsgWrapper::CDLTMsgWrapper: number of instances - " << sInstanceCounter;

    //static int counter = 0;

    //if(!counter%100000)
    //{
    //    dumpSize();
    //}

    //++counter;
}

CDLTMsgWrapper::~CDLTMsgWrapper()
{
    //--sInstanceCounter;
    //qDebug() << "CDLTMsgWrapper::~CDLTMsgWrapper: number of instances - " << sInstanceCounter;
}

QString CDLTMsgWrapper::getTimeString() const
{
    char strtime[256];
    struct tm *time_tm;
    time_tm = localtime(&mTime);
    if(time_tm)
        strftime(strtime, 256, "%Y/%m/%d %H:%M:%S", time_tm);
    return QString(strtime);
}

const unsigned int& CDLTMsgWrapper::getMicroseconds() const
{
    return mMicroseconds;
}

const unsigned int& CDLTMsgWrapper::getTimestamp() const
{
    return mTimestamp;
}

const unsigned char& CDLTMsgWrapper::getMessageCounter() const
{
    return mMessageCounter;
}

QString CDLTMsgWrapper::getEcuid() const
{
    return QString::fromUtf8( mEcuidUTF8 );
}

QString CDLTMsgWrapper::getApid() const
{
    return QString::fromUtf8( mApidUTF8 );
}

QString CDLTMsgWrapper::getCtid() const
{
    return QString::fromUtf8( mCtidUTF8 );
}

const unsigned int& CDLTMsgWrapper::getSessionid() const
{
    return mSessionid;
}

QString CDLTMsgWrapper::getTypeString() const
{
    return QString((mType>=static_cast<QDltMsg::DltTypeDef>(0) && mType<=static_cast<QDltMsg::DltTypeDef>(7))?qDltMessageType[mType]:"");
}

QString CDLTMsgWrapper::getSubtypeString() const
{
    switch(mType)
    {
    case QDltMsg::DltTypeLog:
        return QString((mSubtype>=0 && mSubtype<=7)?qDltLogInfo[mSubtype]:"");
        break;
    case QDltMsg::DltTypeAppTrace:
        return QString((mSubtype>=0 && mSubtype<=7)?qDltTraceType[mSubtype]:"");
        break;
    case QDltMsg::DltTypeNwTrace:
        return QString((mSubtype>=0 && mSubtype<=7)?qDltNwTraceType[mSubtype]:"");
        break;
    case QDltMsg::DltTypeControl:
        return QString((mSubtype>=0 && mSubtype<=7)?qDltControlType[mSubtype]:"");
        break;
    default:
        return QString("");
    }
}

QString CDLTMsgWrapper::getModeString() const
{
    return QString((mMode>=static_cast<QDltMsg::DltModeDef>(0) && mMode<=static_cast<QDltMsg::DltModeDef>(1))?qDltMode[mMode]:"");
}

const unsigned int& CDLTMsgWrapper::getNumberOfArguments() const
{
    return mNumberOfArguments;
}

QString CDLTMsgWrapper::getPayload() const
{
    return QString::fromUtf8(mPayloadUTF8);
}

QString CDLTMsgWrapper::getCtrlServiceIdString() const
{
    if(mCtrlServiceId == DLT_SERVICE_ID_UNREGISTER_CONTEXT)
        return QString("unregister_context");
    else if(mCtrlServiceId == DLT_SERVICE_ID_CONNECTION_INFO)
        return QString("connection_info");
    else if(mCtrlServiceId == DLT_SERVICE_ID_TIMEZONE)
        return QString("timezone");
    else if(mCtrlServiceId == DLT_SERVICE_ID_MARKER)
        return QString("marker");
    else
        return QString(( mCtrlServiceId<=20 )?qDltCtrlServiceId[mCtrlServiceId]:"");
}

QString CDLTMsgWrapper::getCtrlReturnTypeString() const
{
    return QString(( mCtrlReturnType<=8 )?qDltCtrlReturnType[mCtrlReturnType]:"");
}

unsigned int CDLTMsgWrapper::getInitialMessageSize() const
{
    return mInitialMsgSize;
}

unsigned int CDLTMsgWrapper::getSize() const
{
    return sizeof(CDLTMsgWrapper) +
           sizeof( char ) * static_cast<unsigned int>(mEcuidUTF8.capacity()) +
           sizeof( char ) * static_cast<unsigned int>(mApidUTF8.capacity()) +
           sizeof( char ) * static_cast<unsigned int>(mCtidUTF8.capacity()) +
           sizeof( char ) * static_cast<unsigned int>(mPayloadUTF8.capacity());
}

void CDLTMsgWrapper::dumpSize() const
{
    SEND_MSG( QString( "sizeof(CDLTMsgWrapper) - %1;").arg(sizeof(CDLTMsgWrapper)));
    SEND_MSG( QString( "sizeof( char ) * static_cast<unsigned int>(mEcuidUTF8.capacity()) - %1;").arg(sizeof( char ) * static_cast<unsigned int>(mEcuidUTF8.capacity())));
    SEND_MSG( QString( "sizeof( char ) * static_cast<unsigned int>(mApidUTF8.capacity()) - %1;").arg(sizeof( char ) * static_cast<unsigned int>(mApidUTF8.capacity())));
    SEND_MSG( QString( "sizeof( char ) * static_cast<unsigned int>(mCtidUTF8.capacity()) - %1;").arg(sizeof( char ) * static_cast<unsigned int>(mCtidUTF8.capacity())));
    SEND_MSG( QString( "sizeof( char ) * static_cast<unsigned int>(mPayloadUTF8.capacity() - %1;").arg(sizeof( char ) * static_cast<unsigned int>(mPayloadUTF8.capacity())));
}

void CDLTMsgWrapper::dumpPayload() const
{
    qDebug() << "payload - " << QString::fromUtf8(mPayloadUTF8);
}

PUML_PACKAGE_BEGIN(DMA_LogsWrapper)
    PUML_CLASS_BEGIN_CHECKED(CDLTMsgWrapper)
        PUML_INHERITANCE_CHECKED(IMsgWrapper, implements)
        PUML_USE_DEPENDENCY_CHECKED(QDltMsg, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
