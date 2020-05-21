/**
 * @file    CDLTFileWrapper.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CDLTFileWrapper class
 */

#include "QDebug"

#include "CDLTFileWrapper.hpp"
#include "CDLTMsgWrapper.hpp"

#include "qdlt.h"

CDLTFileWrapper::CDLTFileWrapper(QDltFile* pFile):
    mpFile(pFile),
    mMaxCacheSize(0),
    mCurrentCacheSize(0),
    mbCacheEnabled(false),
    mbIsFull(false),
    mCacheLoadPercentage(0)
{
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

        for(auto* pPlugin: mDecoderPlugins)
        {
            pPlugin->decodeMsg(msg,false);
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
            for(auto* pPlugin: mDecoderPlugins) // decode message in all plugins
            {
                pPlugin->decodeMsg(msg,false);
            }

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

    for( const auto msgId : msgIdSet )
    {
        bResult = cacheMsgByIndex(msgId);

        if(false == bResult)
        {
            break;
        }
    }

    return bResult;
}

bool CDLTFileWrapper::cacheMsgByRange( const tRange& msgRange )
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

void CDLTFileWrapper::setDecoderPlugins( const tPluginPtrList& decoderPlugins )
{
    mDecoderPlugins = decoderPlugins;
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

tRangeProperty CDLTFileWrapper::normalizeSearchRange( const tRangeProperty& inputRange)
{
    tRangeProperty result;

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
