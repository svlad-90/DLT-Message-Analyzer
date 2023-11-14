/**
 * @file    CSearchResultModel.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSearchResultModel class
 */

#include <QDateTime>

#include "CSearchResultModel.hpp"
#include "components/log/api/CLog.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/logsWrapper/api/IMsgWrapper.hpp"
#include "components/logsWrapper/api/IFileWrapper.hpp"

#include "DMA_Plantuml.hpp"

CSearchResultModel::CSearchResultModel(const tSettingsManagerPtr& pSettingsManager, QObject *):
CSettingsManagerClient(pSettingsManager),
mFoundMatchesPack(),
mpFile(nullptr)
{

}

void CSearchResultModel::setFile(const tFileWrapperPtr& pFile)
{
    mpFile = pFile;
}

void CSearchResultModel::updateView(const int& fromRow)
{
    emit dataChanged( index(fromRow,0), index(static_cast<int>(mFoundMatchesPack.matchedItemVec.size() - 1), columnCount(QModelIndex()) - 1) );
    emit layoutChanged();
}

void CSearchResultModel::resetData()
{
    beginResetModel();
    mFoundMatchesPack.matchedItemVec.clear();
    endResetModel();
    updateView();
}

int CSearchResultModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(mFoundMatchesPack.matchedItemVec.size());
}

int CSearchResultModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(eSearchResultColumn::Last);
}

const tFoundMatchesPackItem& CSearchResultModel::getFoundMatchesItemPack( const QModelIndex& modelIndex ) const
{
    if ( ( modelIndex.row() < 0 || modelIndex.row() >= static_cast<int>(mFoundMatchesPack.matchedItemVec.size()) ) ||
         ( modelIndex.column() < 0 || modelIndex.column() >= static_cast<int>(eSearchResultColumn::Last) ) )
    {
        static const tFoundMatchesPackItem sDummyValue;
        return sDummyValue;
    }

    return mFoundMatchesPack.matchedItemVec[ static_cast<std::size_t>(modelIndex.row()) ];
}

QVariant CSearchResultModel::data(const QModelIndex &index, int role) const
{
    if ( ( index.row() < 0 || index.row() >= static_cast<int>(mFoundMatchesPack.matchedItemVec.size()) ) ||
         ( index.column() < 0 || index.column() >= static_cast<int>(eSearchResultColumn::Last) ) )
        return QVariant();

    QVariant result;

    auto column = static_cast<eSearchResultColumn>(index.column());

    if (role == Qt::TextAlignmentRole)
    {
        Qt::AlignmentFlag alignment = Qt::AlignCenter;

        switch( static_cast<eSearchResultColumn>(index.column()) )
        {
            case eSearchResultColumn::UML_Applicability: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::PlotView_Applicability: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Index: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Time: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Timestamp: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Count: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Ecuid: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Apid: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Ctid: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::SessionId: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Type: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Subtype: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Mode: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Args: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Payload: { alignment = static_cast<Qt::AlignmentFlag>((Qt::AlignLeft | Qt::AlignVCenter).operator QFlags<Qt::AlignmentFlag>::Int()); } break;
            case eSearchResultColumn::Last: { alignment = Qt::AlignCenter; } break;
        }

        result = alignment;
    }
    else if ( role == Qt::DisplayRole &&
              ( column != eSearchResultColumn::UML_Applicability &&
                column != eSearchResultColumn::PlotView_Applicability ) )
    {
        auto msgId = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadata().msgId;
        const auto& pMsg = mpFile->getMsg(msgId);
        auto pStrValue = getDataStrFromMsg(msgId, pMsg, column);

        if(nullptr != pStrValue)
        {
            result = std::move(*pStrValue);
        }
    }
    else if( role == Qt::CheckStateRole &&( index.column() == static_cast<int>(eSearchResultColumn::UML_Applicability) ) )
    {
        const auto& UMLInfo = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadata().UMLInfo;

        if(true == UMLInfo.bUMLConstraintsFulfilled)
        {
            if(true == UMLInfo.bApplyForUMLCreation)
            {
                result = Qt::CheckState::Checked;
            }
            else
            {
                result = Qt::CheckState::Unchecked;
            }
        }
    }
    else if( role == Qt::CheckStateRole &&( index.column() == static_cast<int>(eSearchResultColumn::PlotView_Applicability) ) )
    {
        const auto& plotViewInfo = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadata().plotViewInfo;

        if(true == plotViewInfo.bPlotViewConstraintsFulfilled)
        {
            if(true == plotViewInfo.bApplyForPlotCreation)
            {
                result = Qt::CheckState::Checked;
            }
            else
            {
                result = Qt::CheckState::Unchecked;
            }
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if(index.column() == static_cast<int>(eSearchResultColumn::UML_Applicability) ||
           index.column() == static_cast<int>(eSearchResultColumn::PlotView_Applicability))
        {
            result = Qt::AlignLeft;
        }
    }

    return result;
}

QVariant CSearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        result = getName(static_cast<eSearchResultColumn>(section));
    }

    return result;
}

Qt::ItemFlags CSearchResultModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result;

    if (false == index.isValid())
    {
        result = Qt::NoItemFlags;
    }
    else
    {
        if(static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::UML_Applicability)
        {
            const auto& UMLInfo = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadata().UMLInfo;

            if(true == UMLInfo.bUMLConstraintsFulfilled)
            {
                result = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
            }
            else
            {
                result = QAbstractItemModel::flags(index) & (~Qt::ItemIsEditable);
            }
        }
        else if(static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::PlotView_Applicability)
        {
            const auto& plotViewInfo = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadata().plotViewInfo;

            if(true == plotViewInfo.bPlotViewConstraintsFulfilled)
            {
                result = QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
            }
            else
            {
                result = QAbstractItemModel::flags(index) & (~Qt::ItemIsEditable);
            }
        }
        else
        {
            result = QAbstractItemModel::flags(index);
        }
    }

    return result;
}

std::pair<bool, tIntRange> CSearchResultModel::addNextMessageIdxVec(const tFoundMatchesPack &foundMatchesPack)
{
    std::pair<bool, tIntRange> result;

    if(false == foundMatchesPack.matchedItemVec.empty())
    {
        beginInsertRows(QModelIndex(), static_cast<int>(mFoundMatchesPack.matchedItemVec.size()),
                        static_cast<int>(mFoundMatchesPack.matchedItemVec.size() + foundMatchesPack.matchedItemVec.size() - 1));
        result.second.from = static_cast<int>(mFoundMatchesPack.matchedItemVec.size());
        mFoundMatchesPack.matchedItemVec.insert(mFoundMatchesPack.matchedItemVec.end(),
                                                foundMatchesPack.matchedItemVec.begin(),
                                                foundMatchesPack.matchedItemVec.end());
        result.second.to = static_cast<int>(mFoundMatchesPack.matchedItemVec.size() - 1);

        endInsertRows();

        updateView(result.second.from);

        result.first = true;
    }

    return result;
}

int CSearchResultModel::getFileIdx( const QModelIndex& idx ) const
{
    int result = -1;

    auto row = idx.row();

    if(row >= 0 && static_cast<size_t>(row) < mFoundMatchesPack.matchedItemVec.size())
    {
        result = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(row)].getItemMetadata().msgIdFiltered;
    }

    return result;
}

QString CSearchResultModel::getStrValue(const int& row, const eSearchResultColumn& column) const
{
    QString result;

    if(column != eSearchResultColumn::UML_Applicability &&
       column != eSearchResultColumn::PlotView_Applicability)
    {
        auto index = createIndex(row, static_cast<int>(column));

        if(true == index.isValid())
        {
            result = index.data().value<QString>();
        }
    }

    return result;
}

std::pair<int /*rowNumber*/, QString /*diagramContent*/> CSearchResultModel::getUMLDiagramContent() const
{
    std::pair<int, QString> result;

    int& numberOfRows = result.first;
    QString& outputString = result.second;

    outputString.append("@startuml\n");

    if(true == getSettingsManager()->getUML_Autonumber())
    {
        outputString.append("autonumber\n");
    }

    outputString.append("skinparam backgroundColor white\n");

#ifdef __linux__
    outputString.append("skinparam defaultFontName Ubuntu Mono\n");
#elif _WIN32
    outputString.append("skinparam defaultFontName monospaced\n");
#else
    outputString.append("skinparam defaultFontName monospaced\n");
#endif

    //let's represent whether UML data is properly filled in
    int row = 0;

    for(const auto& foundMatchPack : mFoundMatchesPack.matchedItemVec)
    {
        QString subStr;

        const auto& itemMetadata = foundMatchPack.getItemMetadata();

        if(true == itemMetadata.UMLInfo.bUMLConstraintsFulfilled
           && true == itemMetadata.UMLInfo.bApplyForUMLCreation)
        {
            // Result string - <UCL> <URT|URS|UE> <US> : [timestamp] <USID><UM>(<UA>)

            tIntRange insertMethodFormattingRange;
            tIntRange insertMethodFormattingOffset;

            auto getUMLItemRepresentation = [this, &itemMetadata, &row](const eUML_ID& UML_ID)->std::pair<bool, QString>
            {
                std::pair<bool, QString> UMLRepresentationResult;
                UMLRepresentationResult.first = false;

                auto foundUMLDataItem = itemMetadata.UMLInfo.UMLDataMap.find(UML_ID);

                if(foundUMLDataItem != itemMetadata.UMLInfo.UMLDataMap.end())
                {
                    for(const auto& item : foundUMLDataItem->second)
                    {
                        if(item.pUML_Custom_Value == nullptr ||
                           true == item.pUML_Custom_Value->isEmpty()) // if there is no client-defined value
                        {
                            // let's use value from the corresponding group
                            for(const auto& stringCoverageMapItem : item.stringCoverageMap)
                            {
                                auto column = stringCoverageMapItem.first;

                                QString message = getStrValue(row, column);

                                auto messageSize = message.size();
                                const auto& range = stringCoverageMapItem.second.range;

                                if(range.from >= 0 && range.from < messageSize &&
                                   range.to >= 0 && range.to < messageSize )
                                {
                                    switch(UML_ID)
                                    {
                                        case eUML_ID::UML_REQUEST:
                                            UMLRepresentationResult.second.append("->");
                                            break;
                                        case eUML_ID::UML_RESPONSE:
                                        case eUML_ID::UML_EVENT:
                                            UMLRepresentationResult.second.append("<-");
                                            break;
                                        case eUML_ID::UML_ARGUMENTS:
                                        {
                                            int numberOfCharacters = range.to - range.from + 1;
                                            QString argString = message.mid(range.from, numberOfCharacters);
                                            argString.replace("[[", "[ [");
                                            argString.replace("]]", "] ]");
                                            UMLRepresentationResult.second.append(argString);

                                            if(true == stringCoverageMapItem.second.bAddSeparator)
                                            {
                                                UMLRepresentationResult.second.append(" ");
                                            }
                                        }
                                            break;
                                        case eUML_ID::UML_CLIENT:
                                        case eUML_ID::UML_SERVICE:
                                        {
                                            QString str;
                                            str.append("\"");
                                            str.append(message.mid(range.from, range.to - range.from + 1));

                                            if(true == stringCoverageMapItem.second.bAddSeparator)
                                            {
                                                str.append(" ");
                                            }

                                            str.append("\"");

                                            UMLRepresentationResult.second.append(str);
                                        }
                                            break;
                                        case eUML_ID::UML_TIMESTAMP:
                                        {
                                            QString str;
                                            str.append("[");
                                            str.append( message.mid(range.from, range.to - range.from + 1) );
                                            str.append("] ");

                                            UMLRepresentationResult.second.append(str);
                                        }
                                        break;
                                        default:
                                        {
                                            UMLRepresentationResult.second.append(message.mid(range.from, range.to - range.from + 1));

                                            if(true == stringCoverageMapItem.second.bAddSeparator)
                                            {
                                                UMLRepresentationResult.second.append(" ");
                                            }
                                        }
                                            break;
                                    }
                                }

                                UMLRepresentationResult.first = true;
                            }
                        }
                        else // otherwise
                        {
                            switch(UML_ID)
                            {
                                case eUML_ID::UML_CLIENT:
                                case eUML_ID::UML_SERVICE:
                                {
                                    QString str;
                                    str.reserve(item.pUML_Custom_Value->size() + 2);
                                    str.append("\"");
                                    str.append(*item.pUML_Custom_Value);
                                    str.append("\"");

                                    // let's directly use client-defined value, ignoring value from the group
                                    UMLRepresentationResult.second.append(str);
                                }
                                    break;
                                case eUML_ID::UML_TIMESTAMP:
                                {
                                    const auto column = eSearchResultColumn::Timestamp;
                                    QString timestampVal = getStrValue(row, column);
                                    QString str;
                                    str.reserve(timestampVal.size() + 3);
                                    str.append("[");
                                    str.append( timestampVal );
                                    str.append("] ");

                                    // let's use dlt's native timestamp
                                    UMLRepresentationResult.second.append(str);
                                }
                                    break;
                                default:
                                {
                                    // let's directly use client-defined value, ignoring value from the group
                                    UMLRepresentationResult.second.append(*item.pUML_Custom_Value);
                                }
                                    break;
                            }

                            UMLRepresentationResult.first = true;
                        }
                    }
                }
                else
                {
                    if(UML_ID == eUML_ID::UML_TIMESTAMP)
                    {
                        const auto column = eSearchResultColumn::Timestamp;
                        QString timestampVal = getStrValue(row, column);
                        QString str;
                        str.reserve(timestampVal.size() + 3);
                        str.append("[");
                        str.append( timestampVal );
                        str.append("] ");

                        // let's use dlt's native timestamp
                        UMLRepresentationResult.second.append(str);
                        UMLRepresentationResult.first = true;
                    }
                }

                return UMLRepresentationResult;
            };

            auto appendUMLData = [&getUMLItemRepresentation, &subStr](const eUML_ID& UML_ID)
            {
                auto umlRepresentationResult = getUMLItemRepresentation(UML_ID);

                if(true == umlRepresentationResult.first)
                {
                    subStr.append(umlRepresentationResult.second);
                }
                else
                {
                    //SEND_ERR(QString("Was not able to find \"%1\" field!").arg(getUMLIDAsString(UML_ID)));
                }
            };

            appendUMLData(eUML_ID::UML_CLIENT);
            subStr.append(" ");
            appendUMLData(eUML_ID::UML_REQUEST);
            appendUMLData(eUML_ID::UML_RESPONSE);
            appendUMLData(eUML_ID::UML_EVENT);
            subStr.append(" ");
            appendUMLData(eUML_ID::UML_SERVICE);
            subStr.append(" : ");
            int wrappingStartingPoint = subStr.size();
            appendUMLData(eUML_ID::UML_TIMESTAMP);
            appendUMLData(eUML_ID::UML_SEQUENCE_ID);
            subStr.append(" ");
            insertMethodFormattingRange.from = subStr.size();
            appendUMLData(eUML_ID::UML_METHOD);
            insertMethodFormattingRange.to = subStr.size();
            subStr.append("(");

            if(true == getSettingsManager()->getUML_ShowArguments())
            {
                appendUMLData(eUML_ID::UML_ARGUMENTS);
            }
            else
            {
                subStr.append(" ... ");
            }

            subStr.append(")");

            subStr.append("\n");

            // wrapping logic
            {
                if(true == getSettingsManager()->getUML_WrapOutput())
                {
                    const int insertNewLineRange = 100;

                    if(subStr.size() > insertNewLineRange)
                    {
                        const QString insertStr = "\\n";

                        subStr.reserve(subStr.size() + ( insertStr.size() * ( (subStr.size() / insertNewLineRange) + 1 ) ) );

                        int currentIndex = wrappingStartingPoint + insertNewLineRange;
                        int currentOffset = 0;

                        while( (currentIndex + currentOffset) < subStr.size())
                        {
                            if(currentIndex + currentOffset < insertMethodFormattingRange.from)
                            {
                                insertMethodFormattingOffset.from += insertStr.size();
                            }

                            if(currentIndex + currentOffset < insertMethodFormattingRange.to)
                            {
                                insertMethodFormattingOffset.to += insertStr.size();
                            }

                            subStr.insert( currentIndex + currentOffset, insertStr );
                            currentIndex += insertNewLineRange;
                            currentOffset += insertStr.size();
                        }
                    }
                }
            }

            subStr.insert(insertMethodFormattingRange.from + insertMethodFormattingOffset.from, "__**");
            subStr.insert(insertMethodFormattingRange.to + insertMethodFormattingOffset.to + 4, "**__");

            outputString.append(subStr);

            ++numberOfRows;

            const auto& maxRowsNumber = getSettingsManager()->getUML_MaxNumberOfRowsInDiagram();

            // if we've reached the limit
            if(numberOfRows >= maxRowsNumber)
            {
                // stop addition of new rows
                SEND_WRN(QString("Not all UML content will be rendered. Number of rows in diagram was limited to %1 rows due to specified settings").arg(maxRowsNumber));
                break;
            }
        }

        ++row;
    }

    outputString.append("@enduml");

    return result;
}

namespace detail
{
    struct tGraphMetadataItem
    {
        TOptional<QString> graphName;
    };

    typedef std::map<tGraphId, tGraphMetadataItem> tGraphIdMetadataMap;
    typedef std::map<QString, tGraphIdMetadataMap> tAxisNameMetadataMap;

    struct tMetadataItem
    {
        TOptional<tTimestampParserBase::tParsedData> xTimestamp;
        TOptional<tTimestampParserBase::tParsedData> yTimestamp;
    };

    static QString splitCamelCase(const QString val)
    {
        QString result;
        result.reserve(val.size() + 5);

        for (int i = 0; i < val.size(); ++i)
        {
            QChar ch = val[i];
            bool bIsLastCharacter = i == val.size() - 1;

            if(true == bIsLastCharacter)
            {
                result.append(ch);
            }
            else
            {
                QChar nextCh = val[i+1];

                bool bIsFirstCharacter = i == 0;

                if(true == ch.isUpper() &&
                   true == nextCh.isLower() &&
                   false == bIsFirstCharacter)
                {
                    result.append(" ");
                    result.append(ch.toLower());
                }
                else
                {
                    result.append(ch);
                }
            }
        }

        return result;
    }

    static bool processPlotViewDataItemVec(const CSearchResultModel& searchResultModel,
                                           ePlotViewID plotViewID,
                                           const tPlotViewDataItemVec& plotViewDataItemVec,
                                           ISearchResultModel::tPlotContent& plotContent,
                                           const tMsgId& msgId,
                                           tAxisNameMetadataMap& axisNameMetadataMap,
                                           const uint32_t& rowId,
                                           bool bXDataPresented,
                                           tMetadataItem& metadataItem)
    {
        bool bResult = true;

        for(auto it = plotViewDataItemVec.begin();
             it != plotViewDataItemVec.end() && true == bResult;
             ++it)
        {
            const auto& plotViewDataItem = *it;
            const auto& pGroupName = plotViewDataItem.pPlotViewGroupName;
            const auto& splitParameters = plotViewDataItem.plotViewSplitParameters;
            const auto& optColor = plotViewDataItem.optColor;
            const auto& stringCoverageMap = plotViewDataItem.stringCoverageMap;

            auto getValueFromStringCoverageMap = [&]()
            {
                QString result;

                for(const auto& stringCoverageMapItem : stringCoverageMap)
                {
                    auto column = stringCoverageMapItem.first;

                    QString message = searchResultModel.getStrValue(rowId, column);

                    auto messageSize = message.size();
                    const auto& range = stringCoverageMapItem.second.range;

                    if(range.from >= 0 && range.from < messageSize &&
                        range.to >= 0 && range.to < messageSize )
                    {
                        result.append(message.mid(range.from, range.to - range.from + 1));

                        if(true == stringCoverageMapItem.second.bAddSeparator)
                        {
                            result.append(" ");
                        }
                    }
                }

                return result;
            };

            auto resolveTimestamp = [](const tMsgId& msgId,
                                       const TOptional<tTimestampParserBase::tParsedData>& timestamp,
                                       const QString& valueStr,
                                       tPlotData& valueDouble
                                       ) -> bool
            {
                bool bValueResolved = false;

                if(true == timestamp.isSet())
                {
                    const auto& xTimestamp = timestamp.getValue();
                    if(valueStr.size() >= xTimestamp.minValueLength)
                    {
                        typedef tTimestampParserBase::eTimestampItemType eTimestampItemType;

                        QString fetchedValue;
                        int32_t charCounter = 0;

                        for(const auto& timestampDataPair : timestamp.getValue().timestampDataVec)
                        {
                            if(timestampDataPair.first == eTimestampItemType::separator)
                            {
                                fetchedValue.append(":");
                            }
                            else
                            {
                                fetchedValue.append(valueStr.mid(charCounter, timestampDataPair.second));
                            }

                            charCounter += timestampDataPair.second;
                        }

                        auto dateTime = QDateTime::fromString(fetchedValue, xTimestamp.dateTimeFormat);

                        if(true == dateTime.isValid())
                        {
                            auto valueMsecDouble = static_cast<double>(dateTime.toMSecsSinceEpoch()) / 1000;
                            valueDouble = valueMsecDouble;
                            bValueResolved = true;
                        }
                        else
                        {
                            SEND_WRN(QString("Skip line #%1 due to the following error: <Was not able to get valid "
                                             "data out of value '%2' and data format '%3'. Please, try to specify "
                                             "the date and time format, which will work with QDateTime::fromString"
                                             "(const QString &string, const QString &format) method.>")
                                         .arg(msgId).arg(fetchedValue).arg(xTimestamp.dateTimeFormat));
                            bValueResolved = false;
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: <According to the timestamp definition the "
                                         "expected min data size is '%2', while captured data size of ;ine '%3' is '%4'.>")
                                     .arg(msgId).arg(xTimestamp.minValueLength).arg(valueStr).arg(valueStr.size()));
                        bValueResolved = false;
                    }
                }
                else
                {
                    valueDouble = valueStr.toDouble(&bValueResolved);

                    if(false == bValueResolved)
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '<Was not able to convert data '%2' to double>'")
                                     .arg(msgId).arg(valueStr));
                    }
                }

                return bValueResolved;
            };

            assert(nullptr != plotViewDataItem.pPlotViewGroupName);

            switch(plotViewID)
            {
            case ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].axisType.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].axisType.setValue(parsingResult.axisRectType);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_X_MAX:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_X_MAX>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].xMax.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].xMax.setValue(parsingResult.value);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_X_MIN:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_X_MIN>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].xMin.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].xMin.setValue(parsingResult.value);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_Y_MAX:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_Y_MAX>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].yMax.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].yMax.setValue(parsingResult.value);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_Y_MIN:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_Y_MIN>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].yMin.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].yMin.setValue(parsingResult.value);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_X_NAME:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_X_NAME>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].xName.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].xName.setValue(splitCamelCase(parsingResult.value));
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                    .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_Y_NAME:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_Y_NAME>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].yName.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].yName.setValue(splitCamelCase(parsingResult.value));
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_X_UNIT:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_X_UNIT>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].xUnit.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].xUnit.setValue(splitCamelCase(parsingResult.value));
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_Y_UNIT:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_Y_UNIT>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == plotContent.plotAxisMap[parsingResult.axisRectName].yUnit.isSet())
                        {
                            plotContent.plotAxisMap[parsingResult.axisRectName].yUnit.setValue(splitCamelCase(parsingResult.value));
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_X_DATA:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_X_DATA>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        QString valueStr;
                        bool bValueResolved = false;
                        auto valueDouble = 0.0;

                        if(true == parsingResult.value.isSet())
                        {
                            valueDouble = parsingResult.value.getValue();
                            bValueResolved = true;
                        }
                        else
                        {
                            valueStr = getValueFromStringCoverageMap();
                            bValueResolved = resolveTimestamp(msgId,
                                                              metadataItem.xTimestamp,
                                                              valueStr,
                                                              valueDouble);
                        }

                        if(true == bValueResolved)
                        {
                            auto foundAxisNameMetadata = axisNameMetadataMap.find(parsingResult.axisRectName);

                            if(foundAxisNameMetadata != axisNameMetadataMap.end())
                            {
                                auto foundGraphIdMetadata = foundAxisNameMetadata->second.find(parsingResult.graphId);

                                if(foundGraphIdMetadata != foundAxisNameMetadata->second.end())
                                {
                                    assert(true == foundGraphIdMetadata->second.graphName.isSet());
                                    auto& graphSubItem = plotContent.plotAxisMap[parsingResult.axisRectName].plotGraphItemMap[parsingResult.graphId]
                                                               .plotGraphSubItemMap[foundGraphIdMetadata->second.graphName.getValue()];

                                    if(true == graphSubItem.dataItems.empty() || msgId != graphSubItem.dataItems.back().getMsgId())
                                    {
                                        graphSubItem.dataItems.push_back(ISearchResultModel::tPlotGraphDataItem());
                                    }

                                    auto& dataItem = graphSubItem.dataItems.back();

                                    dataItem.setX(msgId, valueDouble);

                                    if(false == graphSubItem.xOptColor.isSet)
                                    {
                                        if(true == optColor.isSet)
                                        {
                                            graphSubItem.xOptColor = optColor;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            bResult = false;
                            continue;
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_X_TIMESTAMP:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_X_TIMESTAMP>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == metadataItem.xTimestamp.isSet())
                        {
                            metadataItem.xTimestamp.setValue(parsingResult.parsedData);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_Y_DATA:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_Y_DATA>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        QString valueStr;
                        bool bValueResolved = false;
                        auto valueDouble = 0.0;

                        if(true == parsingResult.value.isSet())
                        {
                            valueDouble = parsingResult.value.getValue();
                            bValueResolved = true;
                        }
                        else
                        {
                            valueStr = getValueFromStringCoverageMap();
                            bValueResolved = resolveTimestamp(msgId,
                                                                   metadataItem.yTimestamp,
                                                                   valueStr,
                                                                   valueDouble);
                        }

                        if(true == bValueResolved)
                        {
                            auto foundAxisNameMetadata = axisNameMetadataMap.find(parsingResult.axisRectName);

                            if(foundAxisNameMetadata != axisNameMetadataMap.end())
                            {
                                auto foundGraphIdMetadata = foundAxisNameMetadata->second.find(parsingResult.graphId);

                                if(foundGraphIdMetadata != foundAxisNameMetadata->second.end())
                                {
                                    assert(true == foundGraphIdMetadata->second.graphName.isSet());
                                    auto& graphSubItem = plotContent.plotAxisMap[parsingResult.axisRectName].plotGraphItemMap[parsingResult.graphId]
                                                             .plotGraphSubItemMap[foundGraphIdMetadata->second.graphName.getValue()];

                                    if(true == graphSubItem.dataItems.empty() || msgId != graphSubItem.dataItems.back().getMsgId())
                                    {
                                        graphSubItem.dataItems.push_back(ISearchResultModel::tPlotGraphDataItem());
                                    }

                                    auto& dataItem = graphSubItem.dataItems.back();

                                    dataItem.setY(msgId, valueDouble);

                                    if(false == bXDataPresented)
                                    {
                                        QString timestampStr = searchResultModel.getStrValue(rowId, eSearchResultColumn::Timestamp);
                                        bool conversionOk = false;
                                        auto timestampDouble = timestampStr.toDouble(&conversionOk);

                                        if(true == conversionOk)
                                        {
                                            dataItem.setX(msgId, timestampDouble);
                                        }
                                        else
                                        {
                                            SEND_ERR(QString("Was not able to assign timestamp '%1' for string #%2")
                                                         .arg(timestampStr, msgId));
                                        }
                                    }

                                    if(false == graphSubItem.xOptColor.isSet)
                                    {
                                        if(true == optColor.isSet)
                                        {
                                            graphSubItem.xOptColor = optColor;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            bResult = false;
                            continue;
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_Y_TIMESTAMP:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_Y_TIMESTAMP>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        if(false == metadataItem.yTimestamp.isSet())
                        {
                            metadataItem.yTimestamp.setValue(parsingResult.parsedData);
                        }
                        else
                        {
                            //SEND_WRN(QString("Duplicated '%1' flag with content '%2' was parsed. Ignoring.")
                            //             .arg(getPlotIDAsString(plotViewID), *plotViewDataItem.pPlotViewGroupName));
                        }
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_GRAPH_NAME:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_GRAPH_NAME>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        auto& axisRectNameMap = axisNameMetadataMap[parsingResult.axisRectName];
                        auto& metadataItem = axisRectNameMap[parsingResult.graphId];

                        metadataItem.graphName.setValue("");

                        if(true == parsingResult.graphName.isSet() &&
                           false == parsingResult.graphName.getValue().isEmpty())
                        {
                            metadataItem.graphName.getWriteableValue().append(parsingResult.graphName.getValue());
                        }
                        else
                        {
                            auto value = getValueFromStringCoverageMap();
                            metadataItem.graphName.getWriteableValue().append(value);
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_GRAPH_METADATA:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_GRAPH_METADATA>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        auto foundAxisNameMetadata = axisNameMetadataMap.find(parsingResult.axisRectName);

                        if(foundAxisNameMetadata != axisNameMetadataMap.end())
                        {
                            auto foundGraphIdMetadata = foundAxisNameMetadata->second.find(parsingResult.graphId);

                            if(foundGraphIdMetadata != foundAxisNameMetadata->second.end())
                            {
                                assert(true == foundGraphIdMetadata->second.graphName.isSet());
                                auto& graphSubItem = plotContent.plotAxisMap[parsingResult.axisRectName].plotGraphItemMap[parsingResult.graphId]
                                                         .plotGraphSubItemMap[foundGraphIdMetadata->second.graphName.getValue()];

                                if(true == graphSubItem.dataItems.empty() || msgId != graphSubItem.dataItems.back().getMsgId())
                                {
                                    graphSubItem.dataItems.push_back(ISearchResultModel::tPlotGraphDataItem());
                                }

                                auto& dataItem = graphSubItem.dataItems.back();

                                QString targetValue;

                                if(true == parsingResult.value.isSet() &&
                                   false == parsingResult.value.getValue().isEmpty())
                                {
                                    targetValue.append(parsingResult.value.getValue());
                                }
                                else
                                {
                                    auto value = getValueFromStringCoverageMap();
                                    targetValue.append(value);
                                }

                                dataItem.appendMetadata(msgId, splitCamelCase(parsingResult.key), targetValue);
                            }
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            case ePlotViewID::PLOT_GANTT_EVENT:
            {
                if(false == splitParameters.empty())
                {
                    auto parser = tPlotParametersParser<ePlotViewID::PLOT_GANTT_EVENT>();
                    auto parsingResult = parser.parse(true, pGroupName, splitParameters);

                    if(true == parsingResult.bParsingSuccessful)
                    {
                        auto foundAxisNameMetadata = axisNameMetadataMap.find(parsingResult.axisRectName);

                        if(foundAxisNameMetadata != axisNameMetadataMap.end())
                        {
                            auto foundGraphIdMetadata = foundAxisNameMetadata->second.find(parsingResult.graphId);

                            if(foundGraphIdMetadata != foundAxisNameMetadata->second.end())
                            {
                                assert(true == foundGraphIdMetadata->second.graphName.isSet());
                                auto& graphSubItem = plotContent.plotAxisMap[parsingResult.axisRectName].plotGraphItemMap[parsingResult.graphId]
                                                         .plotGraphSubItemMap[foundGraphIdMetadata->second.graphName.getValue()];

                                if(true == graphSubItem.dataItems.empty() || msgId != graphSubItem.dataItems.back().getMsgId())
                                {
                                    graphSubItem.dataItems.push_back(ISearchResultModel::tPlotGraphDataItem());
                                }

                                auto& dataItem = graphSubItem.dataItems.back();

                                ISearchResultModel::eGanttDataItemType eventType;

                                switch(parsingResult.eventType)
                                {
                                    case tPlotParametersParser<ePlotViewID::PLOT_GANTT_EVENT>::eGanttEventType::START:
                                    {
                                        eventType = ISearchResultModel::eGanttDataItemType::START;
                                    }
                                    break;
                                    case tPlotParametersParser<ePlotViewID::PLOT_GANTT_EVENT>::eGanttEventType::END:
                                    {
                                        eventType = ISearchResultModel::eGanttDataItemType::END;
                                    }
                                    break;
                                }

                                dataItem.setGanttDataItemType(msgId, eventType);
                            }
                        }
                    }
                    else
                    {
                        SEND_WRN(QString("Skip line #%1 due to the following error: '%2'")
                                     .arg(msgId).arg(parsingResult.errors));
                        bResult = false;
                        continue;
                    }
                }
            }
            break;
            }
        }

        return bResult;
    }
}

ISearchResultModel::tPlotContent CSearchResultModel::createPlotContent() const
{
    tPlotContent result;

    detail::tAxisNameMetadataMap axisNameMetadataMap;

    uint32_t row = 0;

    for(const auto& foundMatchPack : mFoundMatchesPack.matchedItemVec)
    {
        const auto& itemMetadata = foundMatchPack.getItemMetadata();

        if(true == itemMetadata.plotViewInfo.bPlotViewConstraintsFulfilled
            && true == itemMetadata.plotViewInfo.bApplyForPlotCreation)
        {
            detail::tMetadataItem metadataItem;

            bool bXDataPresented = itemMetadata.plotViewInfo.plotViewDataMap.find(ePlotViewID::PLOT_X_DATA) !=
                    itemMetadata.plotViewInfo.plotViewDataMap.end();
            bool skipRowFlag = false;

            for(auto it = itemMetadata.plotViewInfo.plotViewDataMap.begin();
                it != itemMetadata.plotViewInfo.plotViewDataMap.end() && false == skipRowFlag;
                ++it)
            {
                const auto& plotViewDataMap = *it;
                const auto& plotViewID = plotViewDataMap.first;
                const auto& plotViewDataItemVec = plotViewDataMap.second;

                skipRowFlag = !processPlotViewDataItemVec(*this,
                                           plotViewID,
                                           plotViewDataItemVec,
                                           result,
                                           itemMetadata.msgIdFiltered,
                                           axisNameMetadataMap,
                                           row,
                                           bXDataPresented,
                                           metadataItem);
            }
        }
        ++row;
    }

    return result;
}

void CSearchResultModel::setUML_Applicability( const QModelIndex& index, bool checked )
{
    if(true == index.isValid())
    {
        auto& UML_Info = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadataWriteable().UMLInfo;

        if(true == UML_Info.bUMLConstraintsFulfilled)
        {
            UML_Info.bApplyForUMLCreation = checked;
            dataChanged(index, index);
        }
    }
}

void CSearchResultModel::setPlotView_Applicability( const QModelIndex& index, bool checked )
{
    if(true == index.isValid())
    {
        auto& plotView_Info = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadataWriteable().plotViewInfo;

        if(true == plotView_Info.bPlotViewConstraintsFulfilled)
        {
            plotView_Info.bApplyForPlotCreation = checked;
            dataChanged(index, index);
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_SearchView)
    PUML_CLASS_BEGIN_CHECKED(CSearchResultModel)
        PUML_INHERITANCE_CHECKED(QAbstractTableModel, implements)
        PUML_INHERITANCE_CHECKED(ISearchResultModel, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IFileWrapper, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
