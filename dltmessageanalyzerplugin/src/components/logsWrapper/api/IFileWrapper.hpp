#pragma once

#include "memory"

#include "QObject"
#include "QString"
#include "QSet"
#include "QMap"
#include "QFile"

#include "common/Definitions.hpp"

/**
 * @brief The IFileWrapper class - wrapper on top of the file, with which the
 * plugin works.
 *
 * Implementation of this abstraction should allow to cache the messages from
 * the file to RAM in order to allow "fast as hell" analysis. Caching done in
 * lazy manner, during the "getMsg" call. Or during explicit cacheMsgXXX calls.
 */
class IFileWrapper : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief IFileWrapper - constructor
     */
    IFileWrapper();
    virtual ~IFileWrapper();

    /**
     * @brief setMessageDecoder - sets the message decoder, which is used to decode the messages
     * @param pMessageDecoder - the message decoder to be used for decoding
     */
    virtual void setMessageDecoder( const tMsgDecoderPtr& pMessageDecoder ) = 0;

    /**
     * @brief getNumberOfFiles - provides number of physical files behind the logical file instance
     * @return - number of actual files, which are opened in the trace application.
     */
    virtual int getNumberOfFiles() const = 0;

    /**
     * @brief size - gets number of messages within the file.
     * In case if file is filtered - provides a post-filered size, which includes only filtered messages.
     * If not filteres - provides overall size.
     * @return - the number of messages ( filtered, or non-filtered, depends on context ) within a file.
     */
    virtual int size() const = 0;

    /**
     * @brief sizeNonFiltered - gets OVERALL number of messages within the file
     * @return - non-filtered number of messages within the underlying file.
     */
    virtual int sizeNonFiltered() const = 0;

    /**
     * @brief getMsg - gets message wrapper by its id.
     * @param msgId - message id, to be searched
     * @return - pointer to a found message. Or a non-initialised message, in case if it was not found by id.
     * Still pointer would be initialised.
     */
    virtual tMsgWrapperPtr getMsg(const tMsgId& msgId) = 0;

    /**
     * @brief getFileName - get file name by file's index
     * @param num - index of the physical file. Can be from 0 up to "getNumberOfFiles"
     * @return - the name of the file with extension.
     */
    virtual QString getFileName(int num = 0) = 0;

    /**
     * @brief isFiltered - tells, whether file is filtered or not
     * @return - true, if file is filtered. False otherwise.
     */
    virtual bool isFiltered() const = 0;

    /**
     * @brief getMsgIdFromIndexInMainTable - takes index from main table and returnes the message id.
     * When to use? If your file is filtered, and you need a mapping from filtered to non-filtered id - you can use this method.
     * E.g. if filtered message has idx = 1000, and that message without filtering has id 101010, then this method will take 1000
     * and provide back 101010.
     * In case if file is not filtered, and provided idx is within the file's range - method will return input parameter.
     * In case if provided idx is outside of file's range - INVALID_MSG_ID value will be returned.
     * @param msgId - index of the message in the main table.
     * @return - in the basic case - the mapped "from filtered to non-filtered" id.
     * Other variants of returned value - referenced in above part of this comment.
     */
    virtual int getMsgIdFromIndexInMainTable(int msgId) const = 0;

    //////////////////////// CACHING_FUNCTIONALITY ////////////////////////

    /**
     * @brief setEnableCache - enables or disables caching functionality
     * @param isEnabled - if true provided - caching becomes enabled.
     * If false - cache is turned off and previously cached messages are discarded.
     */
    virtual void setEnableCache(bool isEnabled) = 0;

    /**
     * @brief setMaxCacheSize - sets max cache size.
     * @param cacheSize - maximum allowed size of cache in bytes.
     * All messages above this size will be dropped.
     */
    virtual void setMaxCacheSize(const tCacheSizeB& cacheSize /*in bytes*/ ) = 0;

    /**
     * @brief cacheMsgWrapper - caches theprovided message wrapper
     * @param msgId - non-filtered id of the message
     * @param pMsgWrapper - pointer to the message
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    virtual bool cacheMsgWrapper( const int& msgId, const tMsgWrapperPtr& pMsgWrapper ) = 0;

    /**
     * @brief cacheMsgByIndex - caches message by its id
     * @param msgId - non-filtered id of the message
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    virtual bool cacheMsgByIndex( const tMsgId& msgId ) = 0;

    /**
     * @brief cacheMsgByIndexes - caches messages by their id-s
     * @param msgIdSet - non-filtered set of id-s of the messages, which you want to be cached
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    virtual bool cacheMsgByIndexes( const QSet<tMsgId> msgIdSet ) = 0;

    /**
     * @brief cacheMsgByRange - caches messages within provided range
     * @param msgRange - the range of the messages to be cached
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    virtual bool cacheMsgByRange( const tIntRange& msgRange ) = 0;

    /**
     * @brief resetCache - resets the cache
     */
    virtual void resetCache() = 0;

    ////////////////////////////////////////////////////////////////////////

    /**
     * @brief normalizeSearchRange - gets input range in absolute indexes,
     * and returns back values in relative indexes ( in case if file is filtered )
     * @param inputRange - range in absolute indexes
     * @return - range in relative indexes
     */
    virtual tIntRangeProperty normalizeSearchRange( const tIntRangeProperty& inputRange) = 0;

    /**
     * @brief getCacheStatusAsString - the same as "formCacheStatusString", but using states of the class.
     * @return - a string, which describes status of the cache
     */
    virtual QString getCacheStatusAsString() const = 0;

    //////////////// SUB_FILES SECTION ////////////////

    /**
     * @brief setSubFilesHandlingStatus - sets sub-files handling status
     * @param val - if true, the number of messages in each sub-file would be monitored. False otherwise.
     */
    virtual void setSubFilesHandlingStatus(const bool& val) = 0;

    /**
     * @brief getSubFilesHandlingStatus - gets status of sub-files handling
     * @return - true, if the number of messages in each sub-file is be monitored. False otherwise.
     */
    virtual bool getSubFilesHandlingStatus() const = 0;

    /**
     * @brief getSubFilesSizeRanges - get's vector, which lists number of messages in each physical file
     * @return  - instance of the vector, which lists ranges of messages in each physical file
     * Note! This functionality works ONLY if subFilesHandlingStatus was switched to "ON".
     */
    virtual  tIntRangeList getSubFilesSizeRanges() const = 0;

    /**
     * @brief copyFileNameToClipboard - copies file name associated with provided msgId to clipboard
     * @param msgId - msg id to be checked
     * Note! This functionality works ONLY if subFilesHandlingStatus was switched to "ON".
     */
    virtual void copyFileNameToClipboard( const int& msgId ) const = 0;

    /**
     * @brief copyFileNamesToClipboard - copies file name associated with provided range of msg id-s to clipboard
     * @param msgsRange - range of messages to be checked
     * Note! This functionality works ONLY if subFilesHandlingStatus was switched to "ON".
     */
    virtual void copyFileNamesToClipboard( const tIntRange& msgsRange ) const = 0;

signals:
    /**
     * @brief isEnabledChanged - signal, which is fired whenver cache enabling status is changing
     * @param isEnabled - whether cache is enabled
     */
    void isEnabledChanged(bool isEnabled);

    /**
     * @brief loadChanged - signal, which is fired whenver cache load is being changed
     * @param percents - current cache load in percents
     */
    void loadChanged(unsigned int percents);

    /**
     * @brief currentSizeMbChanged - signal, which is fired whenver cache size in MB is being changed
     * @param MBytes - current cache size in Megabytes.
     */
    void currentSizeMbChanged(tCacheSizeMB MBytes);

    /**
     * @brief maxSizeMbChanged - signal, which is fired whenver max cache size in MB is being changed
     * @param MBytes - current MAX cache size in Megabytes.
     */
    void maxSizeMbChanged(tCacheSizeMB MBytes);

    /**
     * @brief fullChanged - signal, which is fired each time, when the "full" status of cache is changing
     * @param isFull - whether cache is full or not
     */
    void fullChanged(bool isFull);
};

/**
 * @brief formCacheStatusString - forms cache status string to be shown in UI
 * @param currentCacheSize - current cache size in bytes
 * @param maxCacheSize - max cache size in bytes
 * @param cacheLoadPercentage - cache load perentage
 * @param bCacheEnabled - is cache enabled
 * @param bIsFull - is cache full
 * @return - a string, which describes status of the cache
 */
QString formCacheStatusString( const tCacheSizeB& currentCacheSize,
                               const tCacheSizeB& maxCacheSize,
                               const unsigned int& cacheLoadPercentage,
                               bool bCacheEnabled,
                               bool bIsFull );
