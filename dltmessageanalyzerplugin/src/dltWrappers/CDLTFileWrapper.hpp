/**
 * @file    CDLTFileWrapper.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CDLTFileWrapper class
 */
#ifndef CDLTFILEWRAPPER_HPP
#define CDLTFILEWRAPPER_HPP

#include "memory"

#include "QString"
#include "QSet"
#include "QMap"
#include "QObject"
#include "QFile"

#include "../common/Definitions.hpp"

#include "qdlt.h"

class QDltFile;
class QDltMsg;
typedef std::shared_ptr<CDLTMsgWrapper> tDltMsgWrapperPtr;

/**
 * @brief The CDLTFileWrapper class - wrapper on top of QDltFile.
 * Allows to cache the messages from the file to RAM in order to allow "fast as hell" analysis.
 * Caching done in lazy manner, during the "getMsg" call.
 * Or during explicit cacheMsgXXX calls.
 */
class CDLTFileWrapper : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief CDLTFileWrapper - constructor
     * @param pFile - takes pointer to DLT file
     */
    CDLTFileWrapper(QDltFile* pFile);

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    /**
     * @brief setMessageDecoder - sets the message decoder, which is used to decode the messages
     * @param pMessageDecoder - the message decoder to be used for decoding
     */
    void setMessageDecoder( QDltMessageDecoder* pMessageDecoder );
#else
    /**
     * @brief setDecoderPlugins - sets the collection of decoder plugins, which are used then to
     * decode the file messages
     * @param decoderPlugins - list of plugins, which should be used during fetch of the file messages.
     */
    void setDecoderPlugins( const tPluginPtrList& decoderPlugins );
#endif

    /**
     * @brief getNumberOfFiles - provides number of files within the nested QDltFile
     * @return - number of actual DLT files, which are opened in dlt-viewer.
     */
    int getNumberOfFiles() const;

    /**
     * @brief size - gets number of messages within the file.
     * In case if file is filtered - provides a post-filered size, which includes only filtered messages.
     * If not filteres - provides overall size.
     * @return - the number of messages ( filtered, or non-filtered, depends on context ) within a DLT-file.
     */
    int size() const;

    /**
     * @brief sizeNonFiltered - gets OVERALL number of messages within the DLT file
     * @return - non-filtered number of messages within the underlying DLT file.
     */
    int sizeNonFiltered() const;

    /**
     * @brief getMsg - gets DLT message wrapper by its id.
     * @param msgId - message id, to be searched
     * @return - pointer to a found message. Or a non-initialised message, in case if it was not found by id.
     * Still pointer would be initialised.
     */
    tDltMsgWrapperPtr getMsg(const tMsgId& msgId);

    /**
     * @brief getFileName - get file name by file's index
     * @param num - index of the physical file. Can be from 0 up to "getNumberOfFiles"
     * @return - the name of the file with extension.
     */
    QString getFileName(int num = 0);

    /**
     * @brief isFiltered - tells, whether file is filtered or not
     * @return - true, if file is filtered. False otherwise.
     */
    bool isFiltered() const;

    /**
     * @brief getMsgRealPos - takes logical index to the file and returnes the real one.
     * When to use? If your file is filtered, and you need a mapping from filtered to non-filtered id - you can use this method.
     * E.g. if filtered message has idx = 1000, and that message without filtering has idx 101010, then this method will take 1000
     * and provide back 101010.
     * In case if file is not filtered, and provided idx is within the file's range - method will return input parameter.
     * In case if provided idx is outside of file's range - INVALID_MSG_ID value will be returned.
     * @param msgId - logical index of the message.
     * @return - in bast case - the mapped "from filtered to non-filtered" id. Other variants of returned value - referenced in above part of this comment.
     */
    int getMsgRealPos(int msgId) const;

    //////////////////////// CACHING_FUNCTIONALITY ////////////////////////

    /**
     * @brief setEnableCache - enables or disables caching functionality
     * @param isEnabled - if true provided - caching becomes enabled.
     * If false - cache is turned off and previously cached messages are discarded.
     */
    void setEnableCache(bool isEnabled);

    /**
     * @brief setMaxCacheSize - sets max cache size.
     * @param cacheSize - maximum allowed size of cache in bytes.
     * All messages above this size will be dropped.
     */
    void setMaxCacheSize(const tCacheSizeB& cacheSize) /*in bytes*/;

    /**
     * @brief cacheMsgWrapper - caches theprovided DLT message wrapper
     * @param msgId - non-filtered id of the message
     * @param pMsgWrapper - pointer to the message
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    bool cacheMsgWrapper( const int& msgId, const tDltMsgWrapperPtr& pMsgWrapper );

    /**
     * @brief cacheMsgByIndex - caches message by its id
     * @param msgId - non-filtered id of the message
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    bool cacheMsgByIndex( const tMsgId& msgId );

    /**
     * @brief cacheMsgByIndexes - caches messages by their id-s
     * @param msgIdSet - non-filtered set of id-s of the messages, which you want to be cached
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    bool cacheMsgByIndexes( const QSet<tMsgId> msgIdSet );

    /**
     * @brief cacheMsgByRange - caches messages within provided range
     * @param msgRange - the range of the messages to be cached
     * @return - true, if caching was successfully done.
     * False otherwise - if cache is disables or overflowed.
     */
    bool cacheMsgByRange( const tIntRange& msgRange );

    /**
     * @brief resetCache - resets the cache
     */
    void resetCache();

    ////////////////////////////////////////////////////////////////////////

    /**
     * @brief normalizeSearchRange - gets input range in absolute indexes,
     * and returns back values in relative indexes ( in case if file is filtered )
     * @param inputRange - range in absolute indexes
     * @return - range in relative indexes
     */
    tIntRangeProperty normalizeSearchRange( const tIntRangeProperty& inputRange);

    /**
     * @brief formCacheStatusString - forms cache status string to be shown in UI
     * @param currentCacheSize - current cache size in bytes
     * @param maxCacheSize - max cache size in bytes
     * @param cacheLoadPercentage - cache load perentage
     * @param bCacheEnabled - is cache enabled
     * @param bIsFull - is cache full
     * @return - a string, which describes status of the cache
     */
    static QString formCacheStatusString( const tCacheSizeB& currentCacheSize, const tCacheSizeB& maxCacheSize, const unsigned int& cacheLoadPercentage, bool bCacheEnabled, bool bIsFull );

    /**
     * @brief getCacheStatusAsString - the same as "formCacheStatusString", but using states of the class.
     * @return - a string, which describes status of the cache
     */
    QString getCacheStatusAsString() const;

    //////////////// SUB_FILES SECTION ////////////////

    /**
     * @brief setSubFilesHandlingStatus - sets sub-files handling status
     * @param val - if true, the number of messages in each sub-file would be monitored. False otherwise.
     */
    void setSubFilesHandlingStatus(const bool& val);

    /**
     * @brief getSubFilesHandlingStatus - gets status of sub-files handling
     * @return - true, if the number of messages in each sub-file is be monitored. False otherwise.
     */
    bool getSubFilesHandlingStatus() const;

    /**
     * @brief getSubFilesSizeRanges - get's vector, which lists number of messages in each physical file
     * @return  - instance of the vector, which lists ranges of messages in each physical file
     * Note! This functionality works ONLY if subFilesHandlingStatus was switched to "ON".
     */
    tIntRangeList getSubFilesSizeRanges() const;

    /**
     * @brief copyFileNameToClipboard - copies file name associated with provided msgId to clipboard
     * @param msgId - msg id to be checked
     * Note! This functionality works ONLY if subFilesHandlingStatus was switched to "ON".
     */
    void copyFileNameToClipboard( const int& msgId ) const;

    /**
     * @brief copyFileNamesToClipboard - copies file name associated with provided range of msg id-s to clipboard
     * @param msgsRange - range of messages to be checked
     * Note! This functionality works ONLY if subFilesHandlingStatus was switched to "ON".
     */
    void copyFileNamesToClipboard( const tIntRange& msgsRange ) const;

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

private:
    bool cacheDecodedMsg( const int& msgId, const QDltMsg& msg );
    bool decodeAndCacheMsg( const int& msgId, QDltMsg& msg ); // will decode incoming msg
    void incrementCacheSize( const unsigned int& bytes );
    void handleCacheFull(bool isFull);
    int binarySearch(bool isFrom, const int& fromIdx, const int& toIdx, const int& targetIdx) const;

private:
    QDltFile* mpFile;

    typedef QMap<tMsgId, tDltMsgWrapperPtr> tCache;

    struct tCacheData
    {
        tCache cache;
    };

    tCacheData mCache;
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    QDltMessageDecoder* mpMessageDecoder;
#else
    tPluginPtrList mDecoderPlugins;
#endif
    tCacheSizeB mMaxCacheSize;
    tCacheSizeB mCurrentCacheSize;
    bool mbCacheEnabled;
    bool mbIsFull;
    unsigned int mCacheLoadPercentage;

    ///////////////////////////////////////////////////////

    class CSubFilesHandler
    {
        public:
            CSubFilesHandler();
            void setFile( QDltFile* pFile );
            void setSubFilesHandlingStatus(const bool& value);
            bool getSubFilesHandlingStatus() const;
            tIntRangeList getSubFilesSizeRanges() const;
            void copyFileNameToClipboard( const int& msgId ) const;
            void copyFileNamesToClipboard( const tIntRange& msgsRange ) const;

        private:
            void updateSubFiles();
            void clearSubFiles();

        private:
            bool mbSubFilesInitialized;

            class CDLTFileItem
            {
                public:
                    CDLTFileItem(const QString& path);
                    bool updateIndex();
                    int size();

                private:
                    // file path
                    QString mPath;

                    // DLT log file.
                    QFile mInfile;

                    // index
                    QVector<qint64> mIndexAll;
            };

            typedef std::map<QString, std::shared_ptr<CDLTFileItem> > tSubFilesMap;
            tSubFilesMap mSubFilesMap;
            QDltFile* mpFile;
    };

    typedef std::shared_ptr<CSubFilesHandler> tSubFilesHandlerPtr;
    tSubFilesHandlerPtr mpSubFilesHandler;
};

typedef std::shared_ptr<CDLTFileWrapper> tDLTFileWrapperPtr;

#endif // CDLTFILEWRAPPER_HPP
