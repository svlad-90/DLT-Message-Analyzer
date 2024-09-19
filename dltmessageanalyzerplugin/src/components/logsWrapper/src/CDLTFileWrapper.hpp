/**
 * @file    CDLTFileWrapper.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CDLTFileWrapper class
 */
#pragma once

#include "memory"

#include "QString"
#include "QSet"
#include "QMap"
#include "QObject"
#include "QFile"

#include "qdlt.h"

#include "common/Definitions.hpp"
#include "../api/IFileWrapper.hpp"

class QDltFile;
class QDltMsg;
class IMsgDecoder;

/**
 * @brief The CDLTFileWrapper class - wrapper on top of QDltFile.
 * Allows to cache the messages from the file to RAM in order to allow "fast as hell" analysis.
 * Caching done in lazy manner, during the "getMsg" call.
 * Or during explicit cacheMsgXXX calls.
 */
class CDLTFileWrapper : public IFileWrapper
{
    Q_OBJECT

public:

    /**
     * @brief CDLTFileWrapper - constructor
     * @param pFile - takes pointer to DLT file
     */
    CDLTFileWrapper(QDltFile* pFile);

    virtual void setMessageDecoder( const tMsgDecoderPtr& pMessageDecoder ) override;
    int getNumberOfFiles() const override;
    int size() const override;
    int sizeNonFiltered() const override;
    tMsgWrapperPtr getMsg(const tMsgId& msgId) override;
    QString getFileName(int num = 0) override;
    bool isFiltered() const override;
    int getMsgIdFromIndexInMainTable(int msgIdxInMainTable) const override;

    //////////////////////// CACHING_FUNCTIONALITY ////////////////////////

    void setEnableCache(bool isEnabled) override;
    void setMaxCacheSize(const tCacheSizeB& cacheSize) override;
    bool cacheMsgWrapper( const int& msgId, const tMsgWrapperPtr& pMsgWrapper ) override;
    bool cacheMsgByIndex( const tMsgId& msgId ) override;
    bool cacheMsgByIndexes( const QSet<tMsgId> msgIdSet ) override;
    bool cacheMsgByRange( const tIntRange& msgRange ) override;
    void resetCache() override;

    ////////////////////////////////////////////////////////////////////////

    tIntRangeProperty normalizeSearchRange( const tIntRangeProperty& inputRange) override;
    QString getCacheStatusAsString() const override;

    //////////////// SUB_FILES SECTION ////////////////

    void setSubFilesHandlingStatus(const bool& val) override;
    bool getSubFilesHandlingStatus() const override;
    tIntRangeList getSubFilesSizeRanges() const override;
    void copyFileNameToClipboard( const int& msgId ) const override;
    void copyFileNamesToClipboard( const tIntRange& msgsRange ) const override;

private:
    bool cacheDecodedMsg( const int& msgId, const QDltMsg& msg );
    bool decodeAndCacheMsg( const int& msgId, QDltMsg& msg ); // will decode incoming msg
    void incrementCacheSize( const unsigned int& bytes );
    void handleCacheFull(bool isFull);
    int binarySearch(bool isFrom, const int& fromIdx, const int& toIdx, const int& targetIdx) const;

private:
    QDltFile* mpFile;

    typedef QMap<tMsgId, tMsgWrapperPtr> tCache;

    struct tCacheData
    {
        tCache cache;
    };

    tCacheData mCache;
    tMsgDecoderPtr mpMessageDecoder;
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
