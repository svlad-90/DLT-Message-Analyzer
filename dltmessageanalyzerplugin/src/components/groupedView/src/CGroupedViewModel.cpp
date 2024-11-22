/**
 * @file    CGroupedViewModel.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CGroupedViewModel class
 */

#include "QDebug"
#include "QVector"
#include "QApplication"
#include "QDateTime"

#include "CGroupedViewModel.hpp"
#include "components/log/api/CLog.hpp"
#include "components/settings/api/ISettingsManager.hpp"

#include "DMA_Plantuml.hpp"

static const tQStringPtr sRootItemName = std::make_shared<QString>("Root");

CGroupedViewModel::CGroupedViewModel(const tSettingsManagerPtr& pSettingsManager,
                                     QObject *parent)
    : IGroupedViewModel(parent),
      CSettingsManagerClient(pSettingsManager),
      mRegex(),
      mSortingColumn(eGroupedViewColumn::Messages),
      mSortOrder(Qt::SortOrder::DescendingOrder),
      mDuplicatesHandler(),
      mSortingHandler(),
      mFindHandler(),
      mAnalyzedValues()
{   
    mSortingHandler = [this](QVector<tTreeItemPtr>& children,
                            const int& sortingColumn,
                            Qt::SortOrder sortingOrder)
    {
        switch(static_cast<eGroupedViewColumn>(sortingColumn))
        {
            case eGroupedViewColumn::SubString:
            {
                std::sort(children.begin(), children.end(),
                    [&sortingColumn](const tTreeItemPtr& lVal, const tTreeItemPtr& rVal)
                {
                    const tQStringPtrWrapper& lValSubString = lVal->data(sortingColumn).get<tQStringPtrWrapper>();
                    const tQStringPtrWrapper& rValSubString = rVal->data(sortingColumn).get<tQStringPtrWrapper>();
                    bool bResult = false;
                    if(nullptr != lValSubString.pString && nullptr != rValSubString.pString)
                    {
                        bResult = lValSubString.pString->compare( *rValSubString.pString ) < 0;
                    }
                    return bResult;
                });
            }
                break;
            case eGroupedViewColumn::Messages:
            case eGroupedViewColumn::MessagesPerSecondAverage:
            case eGroupedViewColumn::Payload:
            case eGroupedViewColumn::PayloadPerSecondAverage:
            {
                for(const auto& pChild : children)
                {
                    if(nullptr != pChild)
                    {
                        if(static_cast<eGroupedViewColumn>(sortingColumn) == eGroupedViewColumn::PayloadPerSecondAverage)
                        {
                            updateAverageValues(pChild, true, false);
                        }
                        else if(static_cast<eGroupedViewColumn>(sortingColumn) == eGroupedViewColumn::MessagesPerSecondAverage)
                        {
                            updateAverageValues(pChild, false, true);
                        }
                    }
                }

                std::sort(children.begin(), children.end(),
                [&sortingColumn](const tTreeItemPtr& lVal, const tTreeItemPtr& rVal)
                {
                    const auto& lValInt = lVal->data(sortingColumn).get<int>();
                    const auto& rValInt = rVal->data(sortingColumn).get<int>();
                    return lValInt < rValInt;
                });
            }
                break;
            case eGroupedViewColumn::PayloadPercantage:
            case eGroupedViewColumn::MessagesPercantage:
            {
                for(const auto& pChild : children)
                {
                    if(nullptr != pChild)
                    {
                        if(static_cast<eGroupedViewColumn>(sortingColumn) == eGroupedViewColumn::PayloadPercantage)
                        {
                            updatePercentageValues(pChild, true, false);
                        }
                        else if(static_cast<eGroupedViewColumn>(sortingColumn) == eGroupedViewColumn::MessagesPercantage)
                        {
                            updatePercentageValues(pChild, false, true);
                        }
                    }
                }

                std::sort(children.begin(), children.end(),
                [&sortingColumn](const tTreeItemPtr& lVal, const tTreeItemPtr& rVal)
                {
                    const auto& lValInt = lVal->data(sortingColumn).get<double>();
                    const auto& rValInt = rVal->data(sortingColumn).get<double>();
                    return lValInt < rValInt;
                });
            }
                break;
            case eGroupedViewColumn::AfterLastVisible:
            case eGroupedViewColumn::Metadata:
            case eGroupedViewColumn::Last:
                break;
        }

        if( sortingOrder == Qt::SortOrder::DescendingOrder)
        {
            std::reverse(children.begin(), children.end());
        }
    };

    mDuplicatesHandler =
    [this](CTreeItem* pItem, const CTreeItem::tData& dataItems)
    {
        if(static_cast<int>(eGroupedViewColumn::Messages) < dataItems.size())
        {    
            auto messagesColumn = static_cast<int>(eGroupedViewColumn::Messages);
            auto payloadColumn = static_cast<int>(eGroupedViewColumn::Payload);

            // update of message and payload columns
            {
                pItem->getWriteableData(messagesColumn) = pItem->getWriteableData(messagesColumn).get<int>() + 1;
                pItem->getWriteableData(payloadColumn) = pItem->getWriteableData(payloadColumn).get<int>() + dataItems[payloadColumn].get<int>();
            }

            const auto metadataColumn = static_cast<int>(eGroupedViewColumn::Metadata);

            //Important to update metadata in item before further usage.
            {
                auto& existingMetadata = pItem->getWriteableData(metadataColumn).get<tGroupedViewMetadata>();
                const auto& incomingMetadata = dataItems[metadataColumn].get<tGroupedViewMetadata>();
                existingMetadata.msgId = incomingMetadata.msgId; // it should be the same, actually, as they are duplicated
                existingMetadata.timeStamp = incomingMetadata.timeStamp;
            }

            const auto& metadataVariant = pItem->data(metadataColumn);

            const auto& metadata = metadataVariant.get<tGroupedViewMetadata>();
            const auto& timeStamp = metadata.timeStamp;
            const auto& msgId = metadata.msgId;

            if(mAnalyzedValues.prevMessageTimestamp == 0 || mAnalyzedValues.prevMessageId == msgId)
            {
                mAnalyzedValues.prevMessageTimestamp = timeStamp;
                mAnalyzedValues.prevMessageId = msgId;
            }
            else
            {
                if(false == mAnalyzedValues.perSecondStatisticsDiscarded)
                {
                    if(timeStamp < mAnalyzedValues.prevMessageTimestamp)
                    {
                        mAnalyzedValues.perSecondStatisticsDiscarded = true;

                        SEND_WRN(QString("Timestamps inconsistency between messages : previous message - %1 (%2); current message - %3 (%4). "
                                         "Average statistics will be discarded. "
                                         "Please, filter out specific ECU-s, app-s & search range to preserve consistency of analyzed data.")
                       .arg(mAnalyzedValues.prevMessageId)
                       .arg(mAnalyzedValues.prevMessageTimestamp)
                       .arg(msgId)
                       .arg(timeStamp));
                    }
                    else
                    {
                        mAnalyzedValues.prevMessageTimestamp = timeStamp;
                        mAnalyzedValues.prevMessageId = msgId;
                    }
                }
            }

            if(false == mAnalyzedValues.perSecondStatisticsDiscarded)
            {
                {
                    if(mAnalyzedValues.maxTime == 0u)
                    {
                        mAnalyzedValues.maxTime = timeStamp;
                    }
                    else
                    {
                        if(timeStamp > mAnalyzedValues.maxTime)
                        {
                            mAnalyzedValues.maxTime = timeStamp;
                        }
                    }
                }

                {
                    if(mAnalyzedValues.minTime == 0u)
                    {
                        mAnalyzedValues.minTime = timeStamp;
                    }
                    else
                    {
                        if(timeStamp < mAnalyzedValues.minTime)
                        {
                            mAnalyzedValues.minTime = timeStamp;
                        }
                    }
                }
            }
        }
    };

    mFindHandler = [](const CTreeItem* pItem, const CTreeItem::tData& key) -> CTreeItem::tFindItemResult
    {
        CTreeItem::tFindItemResult result;
        result.bFound = false;

        auto keyColumn = static_cast<int>(eGroupedViewColumn::SubString);

        if(keyColumn < key.size())
        {
            const auto& children = pItem->getChildren();
            const auto& targetKey = key[keyColumn];

            auto foundChild = children.find(targetKey);

            if(foundChild != children.end())
            {
                result.bFound = true;
                result.pItem = *foundChild;
            }
            else
            {
                result.key = targetKey;
            }
        }

        return result;
    };

    mpRootItem = new tTreeItem(nullptr, static_cast<int>(mSortingColumn),
                               mSortingHandler,
                               mDuplicatesHandler, mFindHandler);
    mpRootItem->appendColumn( getName(eGroupedViewColumn::SubString) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Messages) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::MessagesPercantage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::MessagesPerSecondAverage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Payload) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::PayloadPercantage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::PayloadPerSecondAverage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Metadata) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Last) );
}

void CGroupedViewModel::updateAverageValues(CTreeItem* pItem, bool updatePayload, bool updateMessages)
{
    if(false == mAnalyzedValues.perSecondStatisticsDiscarded)
    {
        auto delimiter = static_cast<int>(( mAnalyzedValues.maxTime - mAnalyzedValues.minTime ) / 10000);

        if(delimiter > 0)
        {
            if(true == updatePayload)
            {
                pItem->getWriteableData(static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage)) =
                        pItem->data(static_cast<int>(eGroupedViewColumn::Payload)).get<int>() / delimiter;
            }

            if(true == updateMessages)
            {
                pItem->getWriteableData(static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage)) =
                        pItem->data(static_cast<int>(eGroupedViewColumn::Messages)).get<int>() / delimiter;
            }
        }
    }
    else
    {
        if(true == updatePayload)
        {
            pItem->getWriteableData(static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage)) = 0;
        }

        if(true == updateMessages)
        {
            pItem->getWriteableData(static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage)) = 0;
        }
    }
}

void CGroupedViewModel::updatePercentageValues(CTreeItem* pItem, bool updatePayload, bool updateMessages)
{
    if(true == updatePayload)
    {
        pItem->getWriteableData(static_cast<int>(eGroupedViewColumn::PayloadPercantage)) =
                ( pItem->data(static_cast<int>(eGroupedViewColumn::Payload)).get<int>()
                / static_cast<double>(mAnalyzedValues.analyzedPayload) ) * 100;
    }

    if(true == updateMessages)
    {
        auto msgs = pItem->data(static_cast<int>(eGroupedViewColumn::Messages)).get<int>();
        auto msgsDelimiter = static_cast<double>(mAnalyzedValues.analyzedMessages);
        pItem->getWriteableData(static_cast<int>(eGroupedViewColumn::MessagesPercantage)) =
                msgs / msgsDelimiter * 100;
    }
}

CGroupedViewModel::~CGroupedViewModel()
{
    if(nullptr != mpRootItem)
    {
        delete mpRootItem;
        mpRootItem = nullptr;
    }
}

QModelIndex CGroupedViewModel::rootIndex() const
{
    return QModelIndex();
}

void CGroupedViewModel::updateView()
{
    emit dataChanged( index(0,0), index ( rowCount(), columnCount()) );
    emit layoutChanged();
}

QModelIndex CGroupedViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    tTreeItemPtr pParentItem;

    if (parent == rootIndex())
        pParentItem = mpRootItem;
    else
        pParentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    if(nullptr != pParentItem)
    {
        tTreeItem *childItem = pParentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex CGroupedViewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    tTreeItem *childItem = static_cast<tTreeItemPtr>(index.internalPointer());
    tTreeItem *parentItem = childItem->getParent();

    if (parentItem == mpRootItem)
        return rootIndex();
    else
        return createIndex(parentItem->row(), 0, parentItem);
}

int CGroupedViewModel::rowCount(const QModelIndex &parent) const
{
    int result = 0;

    tTreeItem *parentItem;

    if (parent == rootIndex())
        parentItem = mpRootItem;
    else
        parentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    if(nullptr != parentItem)
    {
        result = parentItem->childCount();
    }

    return result;
}

int CGroupedViewModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(eGroupedViewColumn::Last);
}

QVariant CGroupedViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QVariant result;

    if (role == Qt::DisplayRole)
    {
        tTreeItem *pItem = static_cast<tTreeItemPtr>(index.internalPointer());

        if(index.column() == static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage) ||
           index.column() == static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage))
        {
            if(index.column() == static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage))
            {
                const_cast<CGroupedViewModel*>(this)->updateAverageValues(pItem, false, true);
            }
            else if(index.column() == static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage))
            {
                const_cast<CGroupedViewModel*>(this)->updateAverageValues(pItem, true, false);
            }
        }

        if(index.column() == static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage) ||
           index.column() == static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage))
        {
            if(index.column() == static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage))
            {
                const_cast<CGroupedViewModel*>(this)->updatePercentageValues(pItem, false, true);
            }
            else if(index.column() == static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage))
            {
                const_cast<CGroupedViewModel*>(this)->updatePercentageValues(pItem, true, false);
            }
        }

        result = toQVariant(pItem->data(index.column()));
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if( static_cast<int>(eGroupedViewColumn::Messages) == index.column() ||
            static_cast<int>(eGroupedViewColumn::MessagesPercantage) == index.column() ||
            static_cast<int>(eGroupedViewColumn::MessagesPerSecondAverage) == index.column() ||
            static_cast<int>(eGroupedViewColumn::Payload) == index.column() ||
            static_cast<int>(eGroupedViewColumn::PayloadPercantage) == index.column() ||
            static_cast<int>(eGroupedViewColumn::PayloadPerSecondAverage) == index.column())
        {
            result = Qt::AlignCenter;
        }
    }

    return result;
}

Qt::ItemFlags CGroupedViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant CGroupedViewModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return toQVariant(mpRootItem->data(section));

    return QVariant();
}

void CGroupedViewModel::addMatches( const tGroupedViewIndices& groupedViewIndices,
                                    const tFoundMatches& matches,
                                    bool update )
{
    if(!groupedViewIndices.empty())
    {
        if(mpRootItem)
        {
            if(false == matches.foundMatchesVec.empty())
            {
                tTreeItem::tDataVec dataVec;
                dataVec.reserve(matches.foundMatchesVec.size() + 1);

                {
                    CTreeItem::tData data;
                    data.reserve(9);
                    data.push_back( tQStringPtrWrapper(sRootItemName) ); /*SubString*/
                    data.push_back(tDataItem(1)); /*Messages*/
                    data.push_back(tDataItem(0)); /*MessagesPercantage*/
                    data.push_back(tDataItem(0)); /*MessagesPerSecond*/
                    data.push_back(tDataItem(static_cast<int>(matches.msgSizeBytes))); /*Payload*/
                    data.push_back(tDataItem(0)); /*PayloadPercantage*/
                    data.push_back(tDataItem(0)); /*PayloadPerSecondAverage*/
                    data.push_back(tDataItem()); /*AfterLastVisible*/
                    data.push_back(tDataItem(tGroupedViewMetadata(matches.timeStamp, matches.msgId))); /*Metadata*/
                    dataVec.push_back(data);

                    mAnalyzedValues.analyzedMessages += 1;
                    mAnalyzedValues.analyzedPayload += matches.msgSizeBytes;
                }

                auto createData = [&matches](const tFoundMatch& match) -> CTreeItem::tData
                {
                    CTreeItem::tData data;
                    data.reserve(9);
                    data.push_back(match.matchStr); /*SubString*/
                    data.push_back(tDataItem(1)); /*Messages*/
                    data.push_back(tDataItem(0)); /*MessagesPercantage*/
                    data.push_back(tDataItem(0)); /*MessagesPerSecond*/
                    data.push_back(tDataItem(static_cast<int>(matches.msgSizeBytes))); /*Payload*/
                    data.push_back(tDataItem(0)); /*PayloadPercantage*/
                    data.push_back(tDataItem(0)); /*PayloadPerSecondAverage*/
                    data.push_back(tDataItem()); /*AfterLastVisible*/
                    data.push_back(tDataItem(tGroupedViewMetadata(matches.timeStamp, matches.msgId))); /*Metadata*/
                    return data;
                };

                std::map<tGroupedViewIdx, CTreeItem::tData> sortingMap;

                for(const auto& match : matches.foundMatchesVec)
                {
                    auto data = createData(match);

                    auto foundIndex = groupedViewIndices.find(match.idx);

                    if(foundIndex != groupedViewIndices.end())
                    {
                        sortingMap.insert(std::make_pair(foundIndex->second, std::move(data)));
                    }
                }

                for(auto& pair : sortingMap)
                {
                    dataVec.emplace_back(std::move(pair.second));
                }

                auto afterAppendFunction = [](CTreeItem* pItem)
                {
                    // if it is a leaf node
                    if(nullptr != pItem && 0 == pItem->childCount())
                    {
                        // we should overtake its msgId into the relatedMsgIds.
                        const auto metadataColumn = static_cast<int>(eGroupedViewColumn::Metadata);
                        auto& existingMetadata = pItem->getWriteableData(metadataColumn).get<tGroupedViewMetadata>();

                        if(existingMetadata.relatedMsgIds.index() == existingMetadata.relatedMsgIds.index_of<tMsgIdSet>())
                        {
                            existingMetadata.relatedMsgIds.get<tMsgIdSet>().insert(existingMetadata.msgId);
                        }
                        else
                        {
                            tMsgIdSet msgIdSet;
                            msgIdSet.insert(existingMetadata.msgId);
                            existingMetadata.relatedMsgIds = msgIdSet;
                        }
                    }
                };

                mpRootItem->addData( dataVec, afterAppendFunction );

                if(true == update)
                {
                    updateView();
                }
            }
        }
    }
}

void CGroupedViewModel::resetData()
{
    beginResetModel();
    if(mpRootItem)
        delete mpRootItem;
    mpRootItem = new tTreeItem(nullptr,
                               static_cast<int>(mSortingColumn),
                               mSortingHandler,
                               mDuplicatesHandler,
                               mFindHandler);
    mpRootItem->appendColumn( getName(eGroupedViewColumn::SubString) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Messages) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::MessagesPercantage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::MessagesPerSecondAverage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Payload) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::PayloadPercantage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::PayloadPerSecondAverage) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Metadata) );
    mpRootItem->appendColumn( getName(eGroupedViewColumn::Last) );
    mAnalyzedValues = tAnalyzedValues();
    endResetModel();
    updateView();
}

std::pair<bool /*result*/, QString /*error*/> CGroupedViewModel::exportToHTML(QString& resultHTML)
{
    std::pair<bool /*result*/, QString /*error*/> result;
    result.first = false;

    if(nullptr != mpRootItem)
    {
        const auto& visibleColumns = getSettingsManager()->getGroupedViewColumnsVisibilityMap();

        bool bIsAnythingToShow = false;

        for(const auto& columnVisibility : visibleColumns)
        {
            if(true == columnVisibility)
            {
                bIsAnythingToShow = true;
                break;
            }
        }

        if(true == bIsAnythingToShow)
        {
            mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);

            QString& finalText = resultHTML;

            QString currentTime = QString().append("[").append(QDateTime::currentDateTime().toString()).append("]");

            finalText.append(QString("<!DOCTYPE html>\n"
                             "<html lang=\"en\">\n"
                             "<head>\n"
                             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                             "<title>Trace spam analysis report %1</title>\n").arg(currentTime));
            finalText.append("<style>\n"
                             "ul, #myUL {\n"
                             "  list-style-type: none;\n"
                             "}\n"
                             "\n"
                             "#myUL {\n"
                             "  margin: 0;\n"
                             "  padding: 0;\n"
                             "}\n"
                             "\n"
                             ".box {\n"
                             "  cursor: pointer;\n"
                             "  -webkit-user-select: none; /* Safari 3.1+ */\n"
                             "  -moz-user-select: none; /* Firefox 2+ */\n"
                             "  -ms-user-select: none; /* IE 10+ */\n"
                             "  user-select: none;\n"
                             "}\n"
                             "\n"
                             ".box::before {\n"
                             "  content: \"\\2610\";\n"
                             "  color: black;\n"
                             "  display: inline-block;\n"
                             "  margin-right: 6px;\n"
                             "}\n"
                             "\n"
                             ".check-box::before {\n"
                             "color: dodgerblue;\n"
                             "}\n"
                             "\n"
                             ".nested {\n"
                             "  display: none;\n"
                             "}\n"
                             "\n"
                             ".active {\n"
                             "  display: block;\n"
                             "}\n"
                             "</style>\n"
                             "</head>\n"
                             "<body>\n");

            finalText.append( QString("\n<h2>Trace spam analysis report %1</h2>").arg(currentTime) );
            finalText.append( QString("\n<h3>Analysis based on regex: \"").append(mRegex.toHtmlEscaped()).append("\"</h3>") );
            finalText.append("<ul id=\"myUL\">");

            auto preVisitFunction = [this, &finalText, &visibleColumns](tTreeItem* pItem)
            {
                if(nullptr != pItem->getParent())
                {
                    if(0 != pItem->childCount())
                    {
                        finalText.append("<li><span class=\"box\">");
                    }
                    else
                    {
                        finalText.append("<li>");
                    }

                    for( int i = static_cast<int>(eGroupedViewColumn::SubString);
                         i < static_cast<int>(eGroupedViewColumn::AfterLastVisible);
                         ++i )
                    {
                        // lets consider ONLY the visible columns
                        auto foundVisibleColumn = visibleColumns.find(static_cast<eGroupedViewColumn>(i));
                        if(foundVisibleColumn != visibleColumns.end() && true == foundVisibleColumn.value())
                        {
                            if(i > 0)
                            {
                                finalText.append("\t|\t");
                            }

                            switch(static_cast<eGroupedViewColumn>(i))
                            {
                                case eGroupedViewColumn::SubString:
                                {
                                    auto variantVal = pItem->data(i);
                                    finalText.append(variantVal
                                                     .get<tQStringPtrWrapper>().pString->toHtmlEscaped());
                                }
                                    break;
                                case eGroupedViewColumn::Messages:
                                case eGroupedViewColumn::MessagesPerSecondAverage:
                                case eGroupedViewColumn::Payload:
                                case eGroupedViewColumn::PayloadPerSecondAverage:
                                {

                                    if(static_cast<eGroupedViewColumn>(i) == eGroupedViewColumn::PayloadPerSecondAverage)
                                    {
                                        updateAverageValues(pItem, true, false);
                                    }
                                    else if(static_cast<eGroupedViewColumn>(i) == eGroupedViewColumn::MessagesPerSecondAverage)
                                    {
                                        updateAverageValues(pItem, false, true);
                                    }

                                    auto variantVal = pItem->data(i);

                                    finalText.append(getName(static_cast<eGroupedViewColumn>(i)))
                                    .append(" : ")
                                    .append(QString::number(variantVal
                                    .get<int>()));
                                }
                                    break;
                                case eGroupedViewColumn::PayloadPercantage:
                                case eGroupedViewColumn::MessagesPercantage:
                                {
                                    if(static_cast<eGroupedViewColumn>(i) == eGroupedViewColumn::PayloadPercantage)
                                    {
                                        updatePercentageValues(pItem, true, false);
                                    }
                                    else if(static_cast<eGroupedViewColumn>(i) == eGroupedViewColumn::MessagesPercantage)
                                    {
                                        updatePercentageValues(pItem, false, true);
                                    }

                                    auto variantVal = pItem->data(i);

                                    finalText.append(getName(static_cast<eGroupedViewColumn>(i)))
                                    .append(" : ")
                                    .append(QString::number(variantVal
                                    .get<double>(), 'f', 3));
                                }
                                    break;
                                case eGroupedViewColumn::AfterLastVisible:
                                case eGroupedViewColumn::Metadata:
                                case eGroupedViewColumn::Last:
                                    break;
                            }
                        }
                    }

                    finalText.append("\n");

                    if(0 != pItem->childCount())
                    {
                        finalText.append("</span><ul class=\"nested\">");
                    }
                }

                return true;
            };

            auto postVisitFunction = [&finalText](const tTreeItem* pItem)
            {
                if(nullptr != pItem->getParent())
                {
                    if(0 != pItem->childCount())
                    {
                        finalText.append("</ul>"
                                         "</li>");
                    }
                    else
                    {
                        finalText.append("</li>");
                    }
                }

                return true;
            };

            mpRootItem->visit( preVisitFunction, postVisitFunction );

            finalText.append("</ul>"
            ""
            "<script>\n"
            "var toggler = document.getElementsByClassName(\"box\");\n"
            "var i;\n"
            "\n"
            "for (i = 0; i < toggler.length; i++) {\n"
            "toggler[i].addEventListener(\"click\", function() {\n"
            "this.parentElement.querySelector(\".nested\").classList.toggle(\"active\");\n"
            "this.classList.toggle(\"check-box\");\n"
            "});\n"
            "}\n"
            "</script>\n"
            "\n"
            "</body>\n"
            "</html>\n");

            result.first = true;
        }
        else
        {
            result.second = "There are 0 visible columns. Nothing to export.";
        }
    }

    return result;
}

void CGroupedViewModel::setUsedRegex(const QString& regex)
{
    mRegex = regex;
}

void CGroupedViewModel::sort(int column, Qt::SortOrder order)
{
    mSortingColumn = toGroupedViewColumn(column);
    mSortOrder = order;

    if(nullptr != mpRootItem)
    {
        mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);
    }

    updateView();
}

tMsgIdSet CGroupedViewModel::getAllMessageIds(const QModelIndex& index)
{
    tMsgIdSet result;

    if(false == index.isValid())
        return result;

    tTreeItem *pItem = static_cast<tTreeItemPtr>(index.internalPointer());

    if(nullptr != pItem)
    {
        pItem->visit([&result](tTreeItemPtr pItem)
        {
            if(nullptr != pItem)
            {
                const auto metadataColumn = static_cast<int>(eGroupedViewColumn::Metadata);
                const auto& itemMetadata = pItem->getWriteableData(metadataColumn).get<tGroupedViewMetadata>();

                if(itemMetadata.relatedMsgIds.index() == itemMetadata.relatedMsgIds.index_of<tMsgIdSet>())
                {
                    auto& insertSet = itemMetadata.relatedMsgIds.get<tMsgIdSet>();
                    result.insert(insertSet.begin(), insertSet.end());
                }
            }

            return true;
        },
        [](const tTreeItemPtr)
        {
            return true;
        });
    }

    return result;
}

void CGroupedViewModel::sortByCurrentSortingColumn()
{
    sort(static_cast<int>(mSortingColumn), mSortOrder);
}

PUML_PACKAGE_BEGIN(DMA_GroupedView)
    PUML_CLASS_BEGIN_CHECKED(CGroupedViewModel)
        PUML_INHERITANCE_CHECKED(QAbstractItemModel, implements)
        PUML_INHERITANCE_CHECKED(IGroupedViewModel, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTreeItem, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
