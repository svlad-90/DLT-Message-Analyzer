#pragma once

#include "memory"

#include "QString"

/**
 * @brief The IMsgWrapper class - interface which represents a message, with which plugin does its work.
 */
class IMsgWrapper
{
public:
    IMsgWrapper();
    virtual ~IMsgWrapper();

    virtual QString getTimeString() const = 0;
    virtual const unsigned int& getMicroseconds() const = 0;
    virtual const unsigned int& getTimestamp() const = 0;
    virtual const unsigned char& getMessageCounter() const = 0;
    virtual QString getEcuid() const = 0;
    virtual QString getApid() const = 0;
    virtual QString getCtid() const = 0;
    virtual const unsigned int& getSessionid() const = 0;
    virtual QString getTypeString() const = 0;
    virtual QString getSubtypeString() const = 0;
    virtual QString getModeString() const = 0;
    virtual const unsigned int& getNumberOfArguments() const = 0;
    virtual QString getPayload() const = 0;
    virtual QString getCtrlServiceIdString() const = 0;
    virtual QString getCtrlReturnTypeString() const = 0;

    virtual unsigned int getInitialMessageSize() const = 0;
    virtual unsigned int getSize() const = 0;
    virtual void dumpSize() const = 0;
    virtual void dumpPayload() const = 0;

    IMsgWrapper( const IMsgWrapper& ) = delete;
    IMsgWrapper( IMsgWrapper&& ) = delete;
    IMsgWrapper& operator=( const IMsgWrapper& ) = delete;
    IMsgWrapper& operator=( IMsgWrapper&& ) = delete;
};

typedef std::shared_ptr<IMsgWrapper> tMsgWrapperPtr;
