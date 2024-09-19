#pragma once

#include <unordered_map>

#include "QModelIndex"
#include "QRegularExpression"

#include "common/Definitions.hpp"

class ISearchResultModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    ISearchResultModel();
    virtual ~ISearchResultModel();

    virtual void updateView(const int& fromRow = 0) = 0;
    virtual std::pair<int /*rowNumber*/, QString /*diagramContent*/> getUMLDiagramContent() const = 0;
    virtual void resetData() = 0;
    virtual void setFile(const tFileWrapperPtr& pFile) = 0;
    virtual std::pair<bool, tIntRange> addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack) = 0;
    virtual int getFileIdx( const QModelIndex& idx ) const = 0;
    virtual int getRowByMsgId( const tMsgId& id ) const = 0;
    virtual void setHighlightedRows( const tMsgIdSet& msgs ) = 0;
    virtual const tMsgIdSet& getHighlightedRows() const = 0;
    virtual const tFoundMatchesPackItem& getFoundMatchesItemPack( const QModelIndex& modelIndex ) const = 0;

    enum eGanttDataItemType
    {
        START = 0,
        END
    };

    struct tPlotGraphDataItem
    {
    public:
        void setX(const tMsgId& msgIdVal, tPlotData xVal);
        const tPlotData& getX() const;
        void setY(const tMsgId& msgIdVal, tPlotData xVal);
        const tPlotData& getY() const;
        void setGanttDataItemType(const tMsgId& msgIdVal, eGanttDataItemType ganttDataItemTypeVal);
        eGanttDataItemType getGanttDataItemType() const;
        void appendMetadata(const tMsgId& msgIdVal,
                            const QString& key,
                            const QString& value);
        const tPlotGraphMetadataMap& getPlotGraphMetadataMap() const;
        const tMsgId& getMsgId() const;
        void setEventId(const tMsgId& msgIdVal, tEventId eventIdVal);
        const TOptional<tEventId>& getEventId() const;

    private:
        TOptional<tPlotData> x;
        TOptional<tPlotData> y;
        TOptional<tMsgId> msgId;
        tPlotGraphMetadataMap plotGraphMetadataMap;
        TOptional<eGanttDataItemType> ganttDataItemType;
        TOptional<tEventId> eventId;
    };

    typedef std::vector<tPlotGraphDataItem> tPlotGraphDataItemVec;

    struct tPlotGraphSubItem
    {
        tPlotGraphDataItemVec dataItems;
        QOptionalColor xOptColor;
        QOptionalColor yOptColor;
    };

    typedef QString tPlotGraphSubItemName;
    typedef std::map<tPlotGraphSubItemName, tPlotGraphSubItem> tPlotGraphSubItemMap;

    struct tPlotGraphItem
    {
        tPlotGraphSubItemMap plotGraphSubItemMap;
    };

    typedef uint32_t tPlotGraphId;
    typedef std::map<tPlotGraphId, tPlotGraphItem> tPlotGraphItemMap;

    struct tPlotAxisItem
    {
        TOptional<ePlotViewAxisType> axisType;
        TOptional<QString> axisLabel;
        TOptional<QString> xName;
        TOptional<QString> yName;
        TOptional<QString> xUnit;
        TOptional<QString> yUnit;
        TOptional<tPlotData> xMax;
        TOptional<tPlotData> xMin;
        TOptional<tPlotData> yMax;
        TOptional<tPlotData> yMin;
        tPlotGraphItemMap plotGraphItemMap;
    };

    typedef QString tPlotAxisName;
    typedef std::map<tPlotAxisName, tPlotAxisItem> tPlotAxisMap;

    struct tPlotContent
    {
        tPlotAxisMap plotAxisMap;
    };

    virtual tPlotContent createPlotContent() const = 0;
};
