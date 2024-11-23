/**
 * @file    CDLTFileWrapper.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTFileWrapper class
 */

#include "QDebug"
#include "QElapsedTimer"
#include "QClipboard"
#include "QApplication"

#include "components/log/api/CLog.hpp"

#include "CDLTFileWrapper.hpp"
#include "CDLTMsgWrapper.hpp"
#include "../api/IMsgDecoder.hpp"

#include "qdlt.h"

#include "DMA_Plantuml.hpp"

CDLTFileWrapper::CDLTFileWrapper(QDltFile* pFile):
    mpFile(pFile),
    mpMessageDecoder(nullptr),
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

tMsgWrapperPtr CDLTFileWrapper::getMsg(const tMsgId& msgId)
{
    tMsgWrapperPtr pResult;

    auto getMessageFromFile = [this, &pResult, &msgId]()
    {
        QByteArray byteArray = mpFile->getMsg(msgId) ;

        QDltMsg msg;

        msg.setMsg(byteArray);

        if(nullptr != mpMessageDecoder)
        {
            mpMessageDecoder->decodeMsg(msg,false);
        }

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

int CDLTFileWrapper::getMsgIdFromIndexInMainTable(int msgIdxInMainTable) const
{
    int result = INVALID_MSG_ID;

    if(nullptr != mpFile)
    {
        if(msgIdxInMainTable >= 0 && msgIdxInMainTable < size())
        {
            if(true == mpFile->isFilter())
            {
                result = mpFile->getMsgFilterPos(msgIdxInMainTable);
            }
            else
            {
                result = msgIdxInMainTable;
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
            tMsgWrapperPtr pMsgWrapper = std::make_shared<CDLTMsgWrapper>(msg); // create wrapper

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
            if(nullptr != mpMessageDecoder)
            {
                mpMessageDecoder->decodeMsg(msg,false);
            }

            tMsgWrapperPtr pMsgWrapper = std::make_shared<CDLTMsgWrapper>(msg); // create wrapper

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

bool CDLTFileWrapper::cacheMsgWrapper( const int& msgId, const tMsgWrapperPtr& pMsgWrapper )
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

    releaseMemoryToOS();
}

void CDLTFileWrapper::setMessageDecoder( const tMsgDecoderPtr& pMessageDecoder )
{
    mpMessageDecoder = pMessageDecoder;
}

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
        if (getMsgIdFromIndexInMainTable(mid) == targetIdx)
            return mid;

        // If element is smaller than mid, then
        // it can only be present in left subarray
        if (getMsgIdFromIndexInMainTable(mid) > targetIdx)
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
    quint16 last_message_length = 0;
    quint8 version=1;
    qint64 lengthOffset=2;
    qint64 storageLength=0;

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
                pos = (*const_indexAll)[mIndexAll.size()-1];

                // first move to beginnng of last found message
                mInfile.seek(pos);

                // read and get file storage length
                buf = mInfile.read(14);
                if(((unsigned char)buf.at(3))==2)
                {
                    storageLength = 14 + ((unsigned char)buf.at(13));
                }
                else
                {
                    storageLength = 16;
                }

                // read and  get last message length
                mInfile.seek(pos + storageLength);
                buf = mInfile.read(7);
                version = (((unsigned char)buf.at(0))&0xe0)>>5;
                if(version==2)
                {
                    lengthOffset = 5;
                }
                else
                {
                    lengthOffset = 2;  // default
                }
                last_message_length = (unsigned char)buf.at(lengthOffset); // was 0
                last_message_length = (last_message_length<<8 | ((unsigned char)buf.at(lengthOffset+1))) + storageLength; // was 1

                // move just behind the next expected message
                pos += (last_message_length - 1);
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
                        if(storageLength==13 && counter_header==13)
                        {
                            storageLength += ((unsigned char)cbuf[num]) + 1;
                        }
                        else if (counter_header==storageLength)
                        {
                            // Read DLT protocol version
                            version = (((unsigned char)cbuf[num])&0xe0)>>5;
                            if(version==1)
                            {
                                lengthOffset = 2;
                            }
                            else if(version==2)
                            {
                                lengthOffset = 5;
                            }
                            else
                            {
                                lengthOffset = 2;  // default
                            }
                        }
                        else if (counter_header==(storageLength+lengthOffset)) // was 16
                        {
                            // Read low byte of message length
                            message_length = (unsigned char)cbuf[num];
                        }
                        else if (counter_header==(storageLength+1+lengthOffset)) // was 17
                        {
                            // Read high byte of message length
                            counter_header = 0;
                            message_length = (message_length<<8 | ((unsigned char)cbuf[num])) + storageLength;
                            next_message_pos = current_message_pos + message_length;
                            if(next_message_pos==file_size)
                            {
                                // last message found in file
                                mIndexAll.append(current_message_pos);
                                break;
                            }
                            // speed up move directly to next message, if inside current buffer
                            if((message_length > storageLength+2+lengthOffset)) // was 20
                            {
                                if((num+message_length-(storageLength+2+lengthOffset)<cbuf_sz))  // was 20
                                {
                                    num+=message_length-(storageLength+2+lengthOffset);  // was 20
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
                    else if(lastFound == 'T' && (cbuf[num] == 0x01 || cbuf[num] == 0x02))
                    {
                        if(next_message_pos == 0)
                        {
                            // first message detected or first message after error
                            current_message_pos = pos+num-3;
                            counter_header = 3;
                            if(cbuf[num] == 0x01)
                                storageLength = 16;
                            else
                                storageLength = 13;
                            if(current_message_pos!=0)
                            {
                                // first messages not at beginning or error occured before
                                errors_in_file++;
                            }
                            // speed up move directly to message length, if inside current buffer
                            if(num+9<cbuf_sz)
                            {
                                num+=9;
                                counter_header+=9;
                            }
                        }
                        else if( next_message_pos == (pos+num-3) )
                        {
                            // Add message only when it is in the correct position in relationship to the last message
                            mIndexAll.append(current_message_pos);
                            current_message_pos = pos+num-3;
                            counter_header = 3;
                            if(cbuf[num] == 0x01)
                                storageLength = 16;
                            else
                                storageLength = 13;
                            // speed up move directly to message length, if inside current buffer
                            if(num+9<cbuf_sz)
                            {
                                num+=9;
                                counter_header+=9;
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

PUML_PACKAGE_BEGIN(DMA_LogsWrapper)
    PUML_CLASS_BEGIN_CHECKED(CDLTFileWrapper)
        PUML_INHERITANCE_CHECKED(IFileWrapper, implements)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QDltFile, 1, 1, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(IMsgWrapper, 1, *, cache)
        PUML_USE_DEPENDENCY_CHECKED(CDLTMsgWrapper, 1, *, creates)
    PUML_CLASS_END()
PUML_PACKAGE_END()
