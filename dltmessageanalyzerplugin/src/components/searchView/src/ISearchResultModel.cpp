#include "../api/ISearchResultModel.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

ISearchResultModel::ISearchResultModel()
{

}

ISearchResultModel::~ISearchResultModel()
{

}

void ISearchResultModel::tPlotGraphDataItem::setX(const tMsgId& msgIdVal, tPlotData xVal)
{
    if(false == msgId.isSet())
    {
        msgId.setValue(msgIdVal);
    }

    if(false == x.isSet())
    {
        x.setValue(xVal);
    }
}

const tPlotData& ISearchResultModel::tPlotGraphDataItem::getX() const
{
    return x.getValue();
}

void ISearchResultModel::tPlotGraphDataItem::setY(const tMsgId& msgIdVal, tPlotData xVal)
{
    if(false == msgId.isSet())
    {
        msgId.setValue(msgIdVal);
    }

    if(false == y.isSet())
    {
        y.setValue(xVal);
    }
}

const tPlotData& ISearchResultModel::tPlotGraphDataItem::getY() const
{
    return y.getValue();
}

void ISearchResultModel::tPlotGraphDataItem::setEventId(const tMsgId& msgIdVal, tEventId eventIdVal)
{
    if(false == msgId.isSet())
    {
        msgId.setValue(msgIdVal);
    }

    if(false == eventId.isSet())
    {
        eventId.setValue(eventIdVal);
    }
}

const TOptional<tEventId>& ISearchResultModel::tPlotGraphDataItem::getEventId() const
{
    return eventId;
}

void ISearchResultModel::tPlotGraphDataItem::setGanttDataItemType(const tMsgId& msgIdVal, eGanttDataItemType ganttDataItemTypeVal)
{
    if(false == msgId.isSet())
    {
        msgId.setValue(msgIdVal);
    }

    if(false == ganttDataItemType.isSet())
    {
        ganttDataItemType.setValue(ganttDataItemTypeVal);
    }
}

ISearchResultModel::eGanttDataItemType ISearchResultModel::tPlotGraphDataItem::getGanttDataItemType() const
{
    return ganttDataItemType.getValue();
}

const tMsgId& ISearchResultModel::tPlotGraphDataItem::getMsgId() const
{
    return msgId.getValue();
}

void ISearchResultModel::tPlotGraphDataItem::appendMetadata(const tMsgId& msgIdVal,
                                                            const QString& key,
                                                            const QString& value)
{
    if(false == msgId.isSet())
    {
        msgId.setValue(msgIdVal);
    }

    auto keyWrapper = tQStringPtrWrapper();
    keyWrapper.pString = std::make_shared<QString>(key);

    auto& plotGraphMetadataValue = plotGraphMetadataMap[keyWrapper];

    auto valueWrapper = tQStringPtrWrapper();
    valueWrapper.pString = std::make_shared<QString>(value);

    if(nullptr == plotGraphMetadataValue.pString)
    {
        plotGraphMetadataValue = valueWrapper;
    }
    else
    {
        plotGraphMetadataValue.pString->append(*valueWrapper.pString);
    }
}

const tPlotGraphMetadataMap& ISearchResultModel::tPlotGraphDataItem::getPlotGraphMetadataMap() const
{
    return plotGraphMetadataMap;
}

DMA_FORCE_LINK_ANCHOR_CPP(ISearchResultModel)

PUML_PACKAGE_BEGIN(DMA_SearchView_API)
    PUML_CLASS_BEGIN(ISearchResultModel)
        PUML_PURE_VIRTUAL_METHOD( +, void updateView(const int& fromRow = 0) )
        PUML_PURE_VIRTUAL_METHOD( +,  std::pair<int__QString > getUMLDiagramContent() const )
        PUML_PURE_VIRTUAL_METHOD( +,  void resetData() )
        PUML_PURE_VIRTUAL_METHOD( +,  void setFile(const tFileWrapperPtr& pFile) )
        PUML_PURE_VIRTUAL_METHOD( +,  std::pair<bool__tIntRange> addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack) )
        PUML_PURE_VIRTUAL_METHOD( +,  int getFileIdx( const QModelIndex& idx ) const )
        PUML_PURE_VIRTUAL_METHOD( +,  tPlotContent createPlotContent() const )
    PUML_CLASS_END()
PUML_PACKAGE_END()
