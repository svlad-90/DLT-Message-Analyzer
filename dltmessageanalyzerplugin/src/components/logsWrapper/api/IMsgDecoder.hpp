#pragma once

class QDltMsg;

class IMsgDecoder
{
public:
    IMsgDecoder();
    virtual ~IMsgDecoder();

    /**
     * Decode message
     * param msg The message to be decoded.
     * param triggeredByUser Whether decode operation was triggered by the user or not
     */
    virtual void decodeMsg(QDltMsg& /*msg*/, int /*triggeredByUser*/) = 0;

private:
    IMsgDecoder(const IMsgDecoder&) = delete;
    IMsgDecoder& operator= (const IMsgDecoder&) = delete;
    IMsgDecoder(IMsgDecoder&&) = delete;
    IMsgDecoder& operator= (IMsgDecoder&&) = delete;
};
