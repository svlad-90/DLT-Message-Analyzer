#include "../api/IFileWrapper.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IFileWrapper::IFileWrapper()
{

}

IFileWrapper::~IFileWrapper()
{

}

QString formCacheStatusString( const tCacheSizeB& currentCacheSize,
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

DMA_FORCE_LINK_ANCHOR_CPP(IFileWrapper)

PUML_PACKAGE_BEGIN(DMA_LogsWrapper_API)
    PUML_CLASS_BEGIN(IFileWrapper)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_USE_DEPENDENCY_CHECKED(IMsgDecoder, 1, 1, uses)
        PUML_USE_DEPENDENCY_CHECKED(IMsgWrapper, 1, 1, provides)
        PUML_PURE_VIRTUAL_METHOD(+,  void setMessageDecoder( IMsgDecoder* pMessageDecoder ) )
        PUML_PURE_VIRTUAL_METHOD(+, int getNumberOfFiles() const )
        PUML_PURE_VIRTUAL_METHOD(+, int size() const )
        PUML_PURE_VIRTUAL_METHOD(+, int sizeNonFiltered() const )
        PUML_PURE_VIRTUAL_METHOD(+, tMsgWrapperPtr getMsg(const tMsgId& msgId) )
        PUML_PURE_VIRTUAL_METHOD(+, QString getFileName(int num = 0) )
        PUML_PURE_VIRTUAL_METHOD(+, bool isFiltered() const )
        PUML_PURE_VIRTUAL_METHOD(+, int getMsgIdFromIndexInMainTable(int msgIdxInMainTable) const )
        PUML_PURE_VIRTUAL_METHOD(+, void setEnableCache(bool isEnabled) )
        PUML_PURE_VIRTUAL_METHOD(+, void setMaxCacheSize(const tCacheSizeB& cacheSize ) )
        PUML_PURE_VIRTUAL_METHOD(+, bool cacheMsgWrapper( const int& msgId, const tMsgWrapperPtr& pMsgWrapper ) )
        PUML_PURE_VIRTUAL_METHOD(+, bool cacheMsgByIndex( const tMsgId& msgId ) )
        PUML_PURE_VIRTUAL_METHOD(+, bool cacheMsgByIndexes( const QSet<tMsgId> msgIdSet ) )
        PUML_PURE_VIRTUAL_METHOD(+, bool cacheMsgByRange( const tIntRange& msgRange ) )
        PUML_PURE_VIRTUAL_METHOD(+, void resetCache() )
        PUML_PURE_VIRTUAL_METHOD(+, tIntRangeProperty normalizeSearchRange( const tIntRangeProperty& inputRange) )
        PUML_PURE_VIRTUAL_METHOD(+, QString getCacheStatusAsString() const )
        PUML_PURE_VIRTUAL_METHOD(+, void setSubFilesHandlingStatus(const bool& val) )
        PUML_PURE_VIRTUAL_METHOD(+, bool getSubFilesHandlingStatus() const )
        PUML_PURE_VIRTUAL_METHOD(+, virtual  tIntRangeList getSubFilesSizeRanges() const )
        PUML_PURE_VIRTUAL_METHOD(+, void copyFileNameToClipboard( const int& msgId ) const )
        PUML_PURE_VIRTUAL_METHOD(+, void copyFileNamesToClipboard( const tIntRange& msgsRange ) const )
        PUML_METHOD(+, signal void isEnabledChanged(bool isEnabled) )
        PUML_METHOD(+, signal void loadChanged(unsigned int percents) )
        PUML_METHOD(+, signal void currentSizeMbChanged(tCacheSizeMB MBytes) )
        PUML_METHOD(+, signal void maxSizeMbChanged(tCacheSizeMB MBytes) )
        PUML_METHOD(+, signal void fullChanged(bool isFull) )
    PUML_CLASS_END()
PUML_PACKAGE_END()
