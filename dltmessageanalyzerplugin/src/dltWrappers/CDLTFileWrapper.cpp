/**
 * @file    CDLTFileWrapper.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTFileWrapper class
 */

#include "QDebug"
#include "QElapsedTimer"
#include "QClipboard"
#include "QApplication"

#include "CDLTFileWrapper.hpp"
#include "CDLTMsgWrapper.hpp"

#include "../log/CLog.hpp"

#include "qdlt.h"

#include "DMA_Plantuml.hpp"

CDLTFileWrapper::CDLTFileWrapper(QDltFile* pFile):
    mpFile(pFile),
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    mpMessageDecoder(nullptr),
#else
    mDecoderPlugins(),
#endif
    mMaxCacheSize(0),
    mCurrentCacheSize(0),
    mbCacheEnabled(false),
    mbIsFull(false),
    mCacheLoadPercentage(0),
    mpSubFilesHandler(std::make_shared<CSubFilesHandler>())
{
    mpSubFilesHandler->setFile(pFile);
}

int CDLTFileWrapper::getNumberOfFiles() const
{
    int result = 0;

    if(nullptr != mpFile)
    {
        result = mpFile->getNumberOfFiles();
    }

    return result;
}

int CDLTFileWrapper::size() const
{
    int result = 0;

    if(nullptr != mpFile)
    {
        if(true == mpFile->isFilter())
        {
            result = mpFile->sizeFilter();
        }
        else
        {
            result = mpFile->size();
        }
    }

    return result;
}

int CDLTFileWrapper::sizeNonFiltered() const
{
    int result = 0;

    if(nullptr != mpFile)
    {
        result = mpFile->size();
    }

    return result;
}

tDltMsgWrapperPtr CDLTFileWrapper::getMsg(const tMsgId& msgId)
{
    tDltMsgWrapperPtr pResult;

    auto getMessageFromFile = [this, &pResult, &msgId]()
    {
        QByteArray byteArray = mpFile->getMsg(msgId) ;

        QDltMsg msg;

        msg.setMsg(byteArray);

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        if(nullptr != mpMessageDecoder)
        {
            mpMessageDecoder->decodeMsg(msg,false);
        }
#else
        for(auto* pPlugin: mDecoderPlugins)
        {
            pPlugin->decodeMsg(msg,false);
        }
#endif

        pResult = std::make_shared<CDLTMsgWrapper>(msg);
    };

    if(true == mbCacheEnabled) // if caching functionality is enabled
    {
        auto findItem = [this, &pResult, &msgId]()->bool
        {
            bool bResult = false;

            auto foundCachedItem = mCache.cache.find( msgId );

            if(mCache.cache.end() != foundCachedItem)
            {
                pResult = foundCachedItem.value();
                bResult = true;
            }

            return bResult;
        };

        if(false == findItem()) // if no item found
        {
            bool bCached = cacheMsgByIndex(msgId); // cache it

            if(true == bCached) // if item cached successfully
            {
                if(false == findItem()) // try to find it again, and if no luck
                {
                    getMessageFromFile(); // get it from file
                }
            }
            else // otherwise
            {
                getMessageFromFile(); // get it from file
            }
        }
    }
    else
    {
        getMessageFromFile();
    }

    return pResult;
}

QString CDLTFileWrapper::getFileName(int num)
{
    QString result;

    if(nullptr != mpFile)
    {
        result = mpFile->getFileName(num);
    }

    return result;
}

int CDLTFileWrapper::getMsgRealPos(int msgId) const
{
    int result = INVALID_MSG_ID;

    if(nullptr != mpFile)
    {
        if(msgId >= 0 && msgId < size())
        {
            if(true == mpFile->isFilter())
            {
                result = mpFile->getMsgFilterPos(msgId);
            }
            else
            {
                result = msgId;
            }
        }
    }

    return result;
}

void CDLTFileWrapper::incrementCacheSize( const unsigned int& bytes )
{
    auto oldCacheSizeMB = BToMB( mCurrentCacheSize );

    mCurrentCacheSize += ( bytes );

    auto newCacheSizeMB = BToMB( mCurrentCacheSize );

    if(oldCacheSizeMB != newCacheSizeMB)
    {
        currentSizeMbChanged(newCacheSizeMB);

        auto maxCaxheSizeMb = BToMB( mMaxCacheSize );
        auto newPercentage = static_cast<unsigned int>(static_cast<double>(newCacheSizeMB) / maxCaxheSizeMb * 100);

        if(mCacheLoadPercentage != newPercentage)
        {
            mCacheLoadPercentage = newPercentage;
            loadChanged(mCacheLoadPercentage);
        }
    }
}

bool CDLTFileWrapper::cacheDecodedMsg( const int& msgId, const QDltMsg& msg )
{
    bool bResult = false;

    if( mCurrentCacheSize < mMaxCacheSize ) // if we have free cache size
    {
        auto foundMsg = mCache.cache.find(msgId);

        if(foundMsg == mCache.cache.end())
        {
            tDltMsgWrapperPtr pMsgWrapper = std::make_shared<CDLTMsgWrapper>(msg); // create wrapper

            mCache.cache.insert(msgId, pMsgWrapper); // cache wrapper

            auto size = pMsgWrapper->getSize();
            incrementCacheSize(size);

            //pMsgWrapper->dumpSize();
            //pMsgWrapper->dumpPayload();
            //qDebug() << "Msg by idx - " << msgId << " - added to cache. msg size - " << size << "; mCurrentCacheSize - " << mCurrentCacheSize << ". Line - " << __LINE__;

            bResult = true;
        }
    }
    else
    {
        handleCacheFull(true);
    }

    return bResult;
}

void CDLTFileWrapper::handleCacheFull(bool isFull)
{
    if(mbIsFull != isFull)
    {
        if(true == isFull && false == mbIsFull)
        {
            qDebug() << "Cache size limit was reached!";
        }

        mbIsFull = isFull;
        fullChanged(isFull);
    }
}

bool CDLTFileWrapper::decodeAndCacheMsg( const int& msgId, QDltMsg& msg )
{
    bool bResult = false;

    if( mCurrentCacheSize < mMaxCacheSize ) // if we have free cache size
    {
        auto foundMsg = mCache.cache.find(msgId);

        if(foundMsg == mCache.cache.end())
        {
#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
            if(nullptr != mpMessageDecoder)
            {
                mpMessageDecoder->decodeMsg(msg,false);
            }
#else
            for(auto* pPlugin: mDecoderPlugins) // decode message in all plugins
            {
                pPlugin->decodeMsg(msg,false);
            }
#endif

            tDltMsgWrapperPtr pMsgWrapper = std::make_shared<CDLTMsgWrapper>(msg); // create wrapper

            mCache.cache.insert(msgId, pMsgWrapper); // cache wrapper

            auto size = pMsgWrapper->getSize();
            incrementCacheSize(size);

            //pMsgWrapper->dumpSize();
            //pMsgWrapper->dumpPayload();
            //qDebug() << "Msg by idx - " << msgId << " - added to cache. msg size - " << size << "; mCurrentCacheSize - " << mCurrentCacheSize << ". Line - " << __LINE__;

            bResult = true;
        }

    }
    else
    {
        handleCacheFull(true);
    }

    return bResult;
}

bool CDLTFileWrapper::cacheMsgWrapper( const int& msgId, const tDltMsgWrapperPtr& pMsgWrapper )
{
    bool bResult = false;

    if(true == mbCacheEnabled) // if cache functionality is enabled
    {
        if( mCurrentCacheSize < mMaxCacheSize ) // if we have free cache size
        {
            auto foundMsg = mCache.cache.find(msgId);

            if(foundMsg == mCache.cache.end())
            {
                mCache.cache.insert(msgId, pMsgWrapper); // cache wrapper

                auto size = pMsgWrapper->getSize();
                incrementCacheSize(size);

                //pMsgWrapper->dumpSize();
                //pMsgWrapper->dumpPayload();
                //qDebug() << "Msg by idx - " << msgId << " - added to cache. msg size - " << size << "; mCurrentCacheSize - " << mCurrentCacheSize << ". Line - " << __LINE__;

                bResult = true;
            }
        }
        else
        {
            handleCacheFull(true);
        }
    }

    return bResult;
}

bool CDLTFileWrapper::cacheMsgByIndex( const tMsgId& msgId )
{
    bool bResult = false;

    if(true == mbCacheEnabled) // if cache functionality is enabled
    {
        if( mCurrentCacheSize < mMaxCacheSize ) // if we have free cache size
        {
            if(nullptr != mpFile) // if we have a file
            {
                auto foundCachedItem = mCache.cache.find( msgId );

                if(mCache.cache.end() == foundCachedItem) // if no such cached element found
                {
                    QByteArray byteArray = mpFile->getMsg(msgId) ;

                    if(0 != byteArray.size()) // if message is not empty
                    {
                        QDltMsg msg;

                        msg.setMsg(byteArray);

                        bResult = decodeAndCacheMsg(msgId, msg);
                    }
                }
            }
        }
        else
        {
            handleCacheFull(true);
        }
    }

    return bResult;
}

bool CDLTFileWrapper::cacheMsgByIndexes( const QSet<tMsgId> msgIdSet )
{
    bool bResult = true;

    for( const auto& msgId : msgIdSet )
    {
        bResult = cacheMsgByIndex(msgId);

        if(false == bResult)
        {
            break;
        }
    }

    return bResult;
}

bool CDLTFileWrapper::cacheMsgByRange( const tIntRange& msgRange )
{
    bool bResult = true;

    for( tMsgId msgId = msgRange.from; msgId < msgRange.to; ++msgId )
    {
        bResult = cacheMsgByIndex(msgId);

        if(false == bResult)
        {
            break;
        }
    }

    return bResult;
}

void CDLTFileWrapper::resetCache()
{
    mCache.cache.clear();
    mCurrentCacheSize = 0;
    mCacheLoadPercentage = 0;

    currentSizeMbChanged(0);
    loadChanged(0);

    handleCacheFull(false);
}

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
void CDLTFileWrapper::setMessageDecoder( QDltMessageDecoder* pMessageDecoder )
{
    mpMessageDecoder = pMessageDecoder;
}
#else
void CDLTFileWrapper::setDecoderPlugins( const tPluginPtrList& decoderPlugins )
{
    mDecoderPlugins = decoderPlugins;
}
#endif

void CDLTFileWrapper::setEnableCache(bool isEnabled)
{
    if(false == isEnabled)
    {
        resetCache();
    }

    mbCacheEnabled = isEnabled;

    isEnabledChanged(mbCacheEnabled);
}

bool CDLTFileWrapper::isFiltered() const
{
    bool bResult = false;

    if(nullptr != mpFile)
    {
        bResult = mpFile->isFilter();
    }

    return bResult;
}

void CDLTFileWrapper::setMaxCacheSize(const tCacheSizeB& cacheSize)
{
    if(mMaxCacheSize != cacheSize
    && mCurrentCacheSize > cacheSize)
    {
        resetCache();
    }

    mMaxCacheSize = cacheSize;

    maxSizeMbChanged( BToMB( mMaxCacheSize ) );
}

// use binary search to go from global to filtered indexes
int CDLTFileWrapper::binarySearch(bool isFrom, const int& fromIdx, const int& toIdx, const int& targetIdx) const
{
    if (toIdx >= fromIdx) {
        int mid = fromIdx + (toIdx - fromIdx) / 2;

        // If the element is present at the middle
        // itself
        if (getMsgRealPos(mid) == targetIdx)
            return mid;

        // If element is smaller than mid, then
        // it can only be present in left subarray
        if (getMsgRealPos(mid) > targetIdx)
            return binarySearch(isFrom, fromIdx, mid - 1, targetIdx);

        // Else the element can only be present
        // in right subarray
        return binarySearch(isFrom, mid + 1, toIdx, targetIdx);
    }

    // We reach here when element is not
    // present in array
    // still we want to get a closest available element here
    return (true == isFrom) ? std::min(fromIdx, toIdx) : std::max(fromIdx, toIdx);
}

tIntRangeProperty CDLTFileWrapper::normalizeSearchRange( const tIntRangeProperty& inputRange)
{
    tIntRangeProperty result;

    if(false == inputRange.isSet)
    {
        return inputRange;
    }

    if(true == inputRange.isFiltered)
    {
        if(true == mpFile->isFilter())
        {
            result = inputRange; // match. Do nothing.
        }
        else
        {
            // return back to global indexes
            result = inputRange;
            result.isFiltered = false;
            result.fromFiltered = 0;
            result.toFiltered = 0;
        }
    }
    else
    {
        if(true == mpFile->isFilter())
        {
            result = inputRange;
            result.isFiltered = true;
            result.fromFiltered = binarySearch( true, 0, mpFile->sizeFilter() - 1, result.from );
            result.toFiltered = binarySearch( false, 0, mpFile->sizeFilter() - 1, result.to );
        }
        else
        {
            result = inputRange; // match. Do nothing.
        }
    }

    return result;
}

QString CDLTFileWrapper::getCacheStatusAsString() const
{
    return formCacheStatusString( mCurrentCacheSize, mMaxCacheSize, mCacheLoadPercentage, mbCacheEnabled, mbIsFull );
}


tIntRangeList CDLTFileWrapper::getSubFilesSizeRanges() const
{
    tIntRangeList result;

    if(nullptr != mpSubFilesHandler && true == mpSubFilesHandler->getSubFilesHandlingStatus())
    {
        result = mpSubFilesHandler->getSubFilesSizeRanges();
    }

    return result;
}

void CDLTFileWrapper::setSubFilesHandlingStatus(const bool& val)
{
    if(nullptr != mpSubFilesHandler)
    {
        mpSubFilesHandler->setSubFilesHandlingStatus(val);
    }
}

bool CDLTFileWrapper::getSubFilesHandlingStatus() const
{
    bool bResult = false;

    if(nullptr != mpSubFilesHandler)
    {
        bResult = mpSubFilesHandler->getSubFilesHandlingStatus();
    }

    return bResult;
}

void CDLTFileWrapper::copyFileNameToClipboard( const int& msgId ) const
{
    if(nullptr != mpSubFilesHandler)
    {
        mpSubFilesHandler->copyFileNameToClipboard(msgId);
    }
}

void CDLTFileWrapper::copyFileNamesToClipboard( const tIntRange& msgsRange ) const
{
    if(nullptr != mpSubFilesHandler)
    {
        mpSubFilesHandler->copyFileNamesToClipboard(msgsRange);
    }
}

QString CDLTFileWrapper::formCacheStatusString( const tCacheSizeB& currentCacheSize,
                                                const tCacheSizeB& maxCacheSize,
                                                const unsigned int& cacheLoadPercentage,
                                                bool bCacheEnabled,
                                                bool bIsFull )
{
    QString result;

    if(false == bCacheEnabled)
    {
        result = "Disabled.";
    }
    else
    {
        if(true == bIsFull)
        {
            result = QString("Enabled. Load - %1% ( Full ). Size - %2 Mb / %3 Mb")
                    .arg(cacheLoadPercentage)
                    .arg(BToMB(currentCacheSize))
                    .arg(BToMB(maxCacheSize));
        }
        else
        {
            result = QString("Enabled. Load - %1%. Size - %2 Mb / %3 Mb")
                    .arg(cacheLoadPercentage)
                    .arg(BToMB(currentCacheSize))
                    .arg(BToMB(maxCacheSize));
        }
    }

    return result;
}

CDLTFileWrapper::CSubFilesHandler::CSubFilesHandler():
mbSubFilesInitialized(false),
mSubFilesMap(),
mpFile(nullptr)
{
}

void CDLTFileWrapper::CSubFilesHandler::setFile( QDltFile* pFile )
{
    mpFile = pFile;

    if(mpFile == nullptr)
    {
        clearSubFiles();
    }
    else
    {
        if(true == mbSubFilesInitialized)
        {
            updateSubFiles();
        }
        else
        {
            clearSubFiles();
        }
    }
}

void CDLTFileWrapper::CSubFilesHandler::setSubFilesHandlingStatus(const bool& value)
{
    if(false == mbSubFilesInitialized && true == value)
    {
        mbSubFilesInitialized = true;
        updateSubFiles();
    }
    else if(true == mbSubFilesInitialized && false == value)
    {
        mbSubFilesInitialized = false;
        clearSubFiles();
    }
}

tIntRangeList CDLTFileWrapper::CSubFilesHandler::getSubFilesSizeRanges() const
{
    tIntRangeList result;

    if(true == mbSubFilesInitialized)
    {
        const_cast<CSubFilesHandler*>(this)->updateSubFiles();

        int processedIndexes = -1;

        for(const auto& item : mSubFilesMap)
        {
            tIntRange analyzedRange;
            analyzedRange.from = processedIndexes + 1;
            analyzedRange.to += processedIndexes + 1 + item.second->size() - 1;
            result.push_back(analyzedRange);

            processedIndexes += item.second->size();

            //SEND_MSG(QString("%1-%2\n").arg(analyzedRange.from).arg(analyzedRange.to));
        }
    }

    return result;
}

void CDLTFileWrapper::CSubFilesHandler::copyFileNameToClipboard( const int& msgId ) const
{
    if( nullptr != mpFile && true == mbSubFilesInitialized )
    {
        auto subFilesSizeRanges = getSubFilesSizeRanges();

        if(false == subFilesSizeRanges.empty())
        {
            int counter = 0;

            for(const auto& subFilesSizeRange : subFilesSizeRanges)
            {
                if(msgId >= subFilesSizeRange.from && msgId <= subFilesSizeRange.to)
                {
                    QClipboard* pClipboard = QApplication::clipboard();

                    pClipboard->setText(mpFile->getFileName(counter));

                    break;
                }

                ++counter;
            }
        }
    }
}

void CDLTFileWrapper::CSubFilesHandler::copyFileNamesToClipboard( const tIntRange& msgsRange ) const
{
    if( nullptr != mpFile && true == mbSubFilesInitialized )
    {
        QVector<QString> fileNames;

        const int INVALID_IDX = -1;
        int startFileIdx = INVALID_IDX;
        int finishFileIdx = INVALID_IDX;

        auto subFilesSizeRanges = getSubFilesSizeRanges();

        int counter = 0;

        for(const auto& subFilesSizeRange : subFilesSizeRanges)
        {
            if(INVALID_IDX == startFileIdx)
            {
                if(msgsRange.from >= subFilesSizeRange.from && msgsRange.from <= subFilesSizeRange.to)
                {
                    startFileIdx = counter;
                }
            }

            if(INVALID_IDX != startFileIdx && INVALID_IDX == finishFileIdx) // we should already find start at this point
            {
                if(msgsRange.to >= subFilesSizeRange.from && msgsRange.to <= subFilesSizeRange.to)
                {
                    finishFileIdx = counter;
                    break;
                }
            }

            ++counter;
        }

        if(INVALID_IDX != startFileIdx && INVALID_IDX != finishFileIdx)
        {
            QClipboard* pClipboard = QApplication::clipboard();

            QString finalText;

            for(int i = startFileIdx; i <= finishFileIdx; ++i )
            {
                finalText.append(mpFile->getFileName(i).append("\n"));
            }

            pClipboard->setText(finalText);
        }
    }
}

bool CDLTFileWrapper::CSubFilesHandler::getSubFilesHandlingStatus() const
{
    return mbSubFilesInitialized;
}

void CDLTFileWrapper::CSubFilesHandler::updateSubFiles()
{
    if(nullptr != mpFile)
    {
        //QElapsedTimer timer;

        //timer.start();

        auto numberOfFiles = mpFile->getNumberOfFiles();

        for(int i = 0; i < numberOfFiles; ++i)
        {
            QString filePath = mpFile->getFileName(i);
            auto foundSubFile = mSubFilesMap.find(filePath);

            if(foundSubFile == mSubFilesMap.end())
            {
                auto pNewFile = std::make_shared<CDLTFileItem>(filePath);

                bool bIndexed = pNewFile->updateIndex();

                if(true == bIndexed)
                {
                    mSubFilesMap.insert(std::make_pair(filePath, pNewFile));
                }
                else
                {
                    QString str( QString("Was not able to index file \"%1\"").arg(filePath) );
                    SEND_WRN(str);
                }
            }
            else
            {
                foundSubFile->second->updateIndex();
            }
        }

        //auto elapsed = timer.elapsed();
        //SEND_MSG(QString("Update file data took %1 ms").arg(elapsed));
    }
}

void CDLTFileWrapper::CSubFilesHandler::clearSubFiles()
{
    mSubFilesMap.clear();
}

CDLTFileWrapper::CSubFilesHandler::CDLTFileItem::CDLTFileItem(const QString& path):
mPath(path)
{}

bool CDLTFileWrapper::CSubFilesHandler::CDLTFileItem::updateIndex()
{
    bool bResult = false;

    QByteArray buf;
    qint64 pos = 0;

    /* open file */
    if(false == mInfile.isOpen())
    {
        mInfile.setFileName(mPath);
        if(false == mInfile.open(QIODevice::ReadOnly))
        {
            /* open file failed */
            SEND_WRN( QString( "open of file" ).append( mPath ).append( "failed"  ) );
        }
        else
        {
            /* start at last found position */
            if(mIndexAll.size())
            {
                /* move behind last found position */
                const QVector<qint64>* const_indexAll = &(mIndexAll);
                pos = (*const_indexAll)[mIndexAll.size()-1] + 4;
                mInfile.seek(pos);
            }
            else {
                /* the file was empty the last call */
                mInfile.seek(0);
            }

            /* Align kbytes, 1MB read at a time */
            static const int READ_BUF_SZ = 1024 * 1024;

            /* walk through the whole file and find all DLT0x01 markers */
            /* store the found positions in the indexAll */
            char lastFound = 0;
            qint64 current_message_pos = 0;
            qint64 next_message_pos = 0;
            int counter_header = 0;
            quint16 message_length = 0;
            qint64 file_size = mInfile.size();
            qint64 errors_in_file  = 0;

            while(true)
            {
                /* read buffer from file */
                buf = mInfile.read(READ_BUF_SZ);
                if(buf.isEmpty())
                    break; // EOF

                /* Use primitive buffer for faster access */
                int cbuf_sz = buf.size();
                const char *cbuf = buf.constData();

                /* find marker in buffer */
                for(int num=0;num<cbuf_sz;num++) {
                    // search length of DLT message
                    if(counter_header>0)
                    {
                        counter_header++;
                        if (counter_header==16)
                        {
                            // Read low byte of message length
                            message_length = static_cast<unsigned char>(cbuf[num]);
                        }
                        else if (counter_header==17)
                        {
                            // Read high byte of message length
                            counter_header = 0;
                            message_length = static_cast<quint16>((message_length<<8 | (static_cast<unsigned char>(cbuf[num]))) + 16);
                            next_message_pos = current_message_pos + message_length;
                            if(next_message_pos==file_size)
                            {
                                // last message found in file
                                mIndexAll.append(current_message_pos);
                                break;
                            }
                            // speed up move directly to next message, if inside current buffer
                            if((message_length > 20))
                            {
                                if((num+message_length-20<cbuf_sz))
                                {
                                    num+=message_length-20;
                                }
                            }
                        }
                    }
                    else if(cbuf[num] == 'D')
                    {
                        lastFound = 'D';
                    }
                    else if(lastFound == 'D' && cbuf[num] == 'L')
                    {
                        lastFound = 'L';
                    }
                    else if(lastFound == 'L' && cbuf[num] == 'T')
                    {
                        lastFound = 'T';
                    }
                    else if(lastFound == 'T' && cbuf[num] == 0x01)
                    {
                        if(next_message_pos == 0)
                        {
                            // first message detected or first message after error
                            current_message_pos = pos+num-3;
                            counter_header = 1;
                            if(current_message_pos!=0)
                            {
                                // first messages not at beginning or error occured before
                                errors_in_file++;
                            }
                            // speed up move directly to message length, if inside current buffer
                            if(num+14<cbuf_sz)
                            {
                                num+=14;
                                counter_header+=14;
                            }
                        }
                        else if( next_message_pos == (pos+num-3) )
                        {
                            // Add message only when it is in the correct position in relationship to the last message
                            mIndexAll.append(current_message_pos);
                            current_message_pos = pos+num-3;
                            counter_header = 1;
                            // speed up move directly to message length, if inside current buffer
                            if(num+14<cbuf_sz)
                            {
                                num+=14;
                                counter_header+=14;
                            }
                        }
                        else if(next_message_pos > (pos+num-3))
                        {
                            // Header detected before end of message
                        }
                        else
                        {
                            // Header detected after end of message
                            // start search for new message back after last header found
                            mInfile.seek(current_message_pos+4);
                            pos = current_message_pos+4;
                            buf = mInfile.read(READ_BUF_SZ);
                            cbuf_sz = buf.size();
                            cbuf = buf.constData();
                            num=0;
                            next_message_pos = 0;
                        }
                        lastFound = 0;
                    }
                    else
                    {
                        lastFound = 0;
                    }
                }
                pos += cbuf_sz;
            }

            mInfile.close();

            bResult = true;
        }
    }

    /* success */
    return bResult;
}

int CDLTFileWrapper::CSubFilesHandler::CDLTFileItem::size()
{
    return mIndexAll.size();
}

PUML_PACKAGE_BEGIN(DMA_DLTWrappers)
    PUML_CLASS_BEGIN_CHECKED(CDLTFileWrapper)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QDltFile, 1, 1, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CDLTMsgWrapper, 1, *, cache)
    PUML_CLASS_END()
PUML_PACKAGE_END()
