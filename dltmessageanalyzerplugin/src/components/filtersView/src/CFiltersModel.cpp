#include "assert.h"

#include "stack"

#include "QDebug"
#include "QVector"
#include "QApplication"
#include "QDateTime"
#include "QRegularExpression"

#include "components/settings/api/ISettingsManager.hpp"
#include "CFiltersModel.hpp"
#include "components/log/api/CLog.hpp"
#include "common/PCRE/PCREHelper.hpp"

#include "DMA_Plantuml.hpp"

#ifdef DEBUG_BUILD
#include "QElapsedTimer"
#endif

CFiltersModel::CFiltersModel(const tSettingsManagerPtr& pSettingsManager,
                             QObject *parent)
    : IFiltersModel(parent),
      CSettingsManagerClient(pSettingsManager),
      mRegex(),
      mSortingColumn(eRegexFiltersColumn::Index),
      mSortOrder(Qt::SortOrder::AscendingOrder),
      mSortingHandler(),
      mFilter(),
      mCompletionCache(),
      mVarGroupsMap()
{
    mSortingHandler = [](QVector<tTreeItemPtr>& children,
                         const int& sortingColumn,
                         Qt::SortOrder sortingOrder)
    {
        tTreeItem::tChildrenVector result;

        switch(static_cast<eRegexFiltersColumn>(sortingColumn))
        {
            case eRegexFiltersColumn::ItemType:
            case eRegexFiltersColumn::Value:
            {
                struct tComparator
                {
                    QString val;
                    bool operator< (const tComparator& rVal) const
                    {
                        return val.compare( rVal.val, Qt::CaseInsensitive ) < 0;
                    }
                };

                QMultiMap<tComparator, tTreeItemPtr> sortedChildrenMap;

                for(const auto& pChild : children)
                {
                    if(nullptr != pChild)
                    {
                        const QString& subString = pChild->data(sortingColumn).get<QString>();
                        tComparator comparator;
                        comparator.val = subString;
                        sortedChildrenMap.insert(comparator, pChild);
                    }
                }

                for(const auto& pChild : sortedChildrenMap)
                {
                    if(nullptr != pChild)
                    {
                        switch(sortingOrder)
                        {
                            case Qt::SortOrder::AscendingOrder:
                            {
                                result.push_back(pChild);
                            }
                                break;
                            case Qt::SortOrder::DescendingOrder:
                            {
                                result.push_front(pChild);
                            }
                                break;
                        }
                    }
                }
            }
                break;
            case eRegexFiltersColumn::Index:
            {
                QMultiMap<int, tTreeItemPtr> sortedChildrenMap;

                for(const auto& pChild : children)
                {
                    if(nullptr != pChild)
                    {
                        int val = pChild->data(static_cast<int>(sortingColumn)).get<int>();
                        sortedChildrenMap.insert(val, pChild);
                    }
                }

                for(const auto& pChild : sortedChildrenMap)
                {
                    if(nullptr != pChild)
                    {
                        switch(sortingOrder)
                        {
                            case Qt::SortOrder::AscendingOrder:
                            {
                                result.push_back(pChild);
                            }
                                break;
                            case Qt::SortOrder::DescendingOrder:
                            {
                                result.push_front(pChild);
                            }
                                break;
                        }
                    }
                }
            }
            break;
            default:
                break;
        }

        children = result;
    };

    connect(getSettingsManager().get(), &ISettingsManager::filterVariablesChanged,
    this, [this](bool)
    {
        filterRegexTokensInternal();
    });

    resetRootItem();
}

void CFiltersModel::resetRootItem()
{
    mpRootItem = std::make_shared<tTreeItem>(nullptr, static_cast<int>(mSortingColumn),
                               mSortingHandler,
                               CTreeItem::tHandleDuplicateFunc(),
                               CTreeItem::tFindItemFunc());
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Value) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Index) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::ItemType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Color) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Range) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::RowType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::IsFiltered) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupName) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupSyntaxType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupIndex) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Last) );
}

QModelIndex CFiltersModel::rootIndex() const
{
    return QModelIndex();
}

void CFiltersModel::updateView()
{
    emit dataChanged( index(0,0), index ( rowCount(), columnCount()) );
    emit layoutChanged();
}

QModelIndex CFiltersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    tTreeItemPtr pParentItem;

    if (parent == rootIndex())
        pParentItem = mpRootItem.get();
    else
        pParentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    tTreeItem *childItem = pParentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex CFiltersModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    tTreeItem *childItem = static_cast<tTreeItemPtr>(index.internalPointer());
    tTreeItem *parentItem = childItem->getParent();

    if (parentItem == mpRootItem.get())
        return rootIndex();
    else
        return createIndex(parentItem->row(), 0, parentItem);
}

int CFiltersModel::rowCount(const QModelIndex &parent) const
{
     tTreeItem *parentItem;

    if (parent == rootIndex())
        parentItem = mpRootItem.get();
    else
        parentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    parentItem->sort(static_cast<int>(mSortingColumn), mSortOrder, false);

    return parentItem->childCount();
}

int CFiltersModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(eRegexFiltersColumn::Last);
}

QVariant CFiltersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QVariant result;

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        tTreeItem *pItem = static_cast<tTreeItemPtr>(index.internalPointer());
        result = toQVariant(pItem->data(index.column()));
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if( static_cast<int>(eRegexFiltersColumn::Index) == index.column() )
        {
            result = Qt::AlignCenter;
        }
        else if( static_cast<int>(eRegexFiltersColumn::ItemType) == index.column() ||
                 static_cast<int>(eRegexFiltersColumn::Value) == index.column() )
        {
            result = Qt::AlignLeft;
        }
    }

    return result;
}

bool CFiltersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(false == index.isValid())
    {
        return false;
    }

    bool bResult = false;

    if(role == Qt::EditRole)
    {
        auto pItem = static_cast<CTreeItem*>(index.internalPointer());
        const auto& currentValue = pItem->data(index.column());
        auto newValue = toRegexDataItem(value, static_cast<eRegexFiltersColumn>(index.column()));

        if(currentValue != newValue)
        {
            bResult = pItem->setColumnData(newValue, index.column());
        }

        if( true == bResult )
        {
            auto packRes = packRegex();

            bResult = packRes.first;

            if(false == bResult)
            {
                bResult = pItem->setColumnData(currentValue, index.column());
                QString error;
                error.append("Regex update is ignored due to the following error: \"").append(packRes.second).append("\"");
                regexUpdatedByUserInvalid( index, error );
            }
        }
    }

    return bResult;
}

QPair<bool,QString> CFiltersModel::packRegex()
{
    QPair<bool,QString> result;
    result.first = false;

    if(nullptr != mpRootItem)
    {
        QString regexStr;

        auto preVisitFunction = [&regexStr](tTreeItem* pItem)
        {
            switch( pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>() )
            {
                case eRegexFiltersRowType::Text:
                {
                    regexStr.append( pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>() );
                }
                    break;
                case eRegexFiltersRowType::VarGroup:
                case eRegexFiltersRowType::NonVarGroup:
                {
                    const auto& groupName = pItem->data(static_cast<int>(eRegexFiltersColumn::GroupName)).get<QString>();

                    regexStr.append("(");

                    if(false == groupName.isEmpty())
                    {
                        const eGroupSyntaxType& groupSyntaxType = static_cast<eGroupSyntaxType>(pItem->data(static_cast<int>(eRegexFiltersColumn::GroupSyntaxType)).get<int>());

                        switch(groupSyntaxType)
                        {
                        case eGroupSyntaxType::SYNTAX_1:
                            regexStr.append("?<");
                            break;
                        case eGroupSyntaxType::SYNTAX_2:
                            regexStr.append("?'");
                            break;
                        case eGroupSyntaxType::SYNTAX_3:
                            regexStr.append("?P<");
                            break;
                        }

                        regexStr.append(groupName);

                        switch(groupSyntaxType)
                        {
                        case eGroupSyntaxType::SYNTAX_1:
                        case eGroupSyntaxType::SYNTAX_3:
                            regexStr.append(">");
                            break;
                        case eGroupSyntaxType::SYNTAX_2:
                            regexStr.append("'");
                            break;
                        }
                    }
                }
                    break;
            }

            return true;
        };

        auto postVisitFunction = [&regexStr](tTreeItem* pItem)
        {
            switch( pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>() )
            {
                case eRegexFiltersRowType::Text:{} break;
                case eRegexFiltersRowType::NonVarGroup:
                case eRegexFiltersRowType::VarGroup:
                {
                    regexStr.append(")");
                }
            }

            return true;
        };

        mpRootItem->visit(preVisitFunction, postVisitFunction, false, true, false);

        QRegularExpression regex( addRegexOptions( regexStr ) );

        result.first = regex.isValid();

        if(true == result.first)
        {
            regexUpdatedByUser(regexStr);
        }
        else
        {
            SEND_ERR(QString("Pack regex: regex - %1").arg(regexStr));
            SEND_ERR(QString("Pack regex: error - %1").arg(regex.errorString()));
            result.second = getFormattedRegexError( regex );
        }
    }

    return result;
}

Qt::ItemFlags CFiltersModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags result = QAbstractItemModel::flags(index);

    if(index.column() == static_cast<int>(eRegexFiltersColumn::Value) &&
       index.sibling(index.row(), static_cast<int>(eRegexFiltersColumn::RowType)).data().value<eRegexFiltersRowType>() == eRegexFiltersRowType::Text)
    {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

QVariant CFiltersModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return toQVariant(mpRootItem->data(section));

    return QVariant();
}

void CFiltersModel::resetData()
{
    beginResetModel();
    resetRootItem();
    endResetModel();
    updateView();
}

void CFiltersModel::setUsedRegex(const QString& regexStr)
{
    QString regex_ = addRegexOptions( regexStr );

    // we should reset a group
    resetData();

    //we need to parse regex into a set of groups here.
    if(nullptr != mpRootItem)
    {
#ifdef DEBUG_BUILD
        QElapsedTimer timer;
        timer.start();
#endif

        QRegularExpression regex(regex_);

#ifdef DEBUG_BUILD
        SEND_MSG(QString("[CFiltersModel][%1] It took %2 ms to create check regex")
                 .arg(__FUNCTION__)
                 .arg(timer.elapsed()));
#endif

        if(true == regex.isValid()) // let's use more deep functionality in order to check correctness of passed regex
        {
            // parse regex
#ifdef DEBUG_BUILD
            timer.restart();
#endif

            resetRootItem();

#ifdef DEBUG_BUILD
            SEND_MSG(QString("[CFiltersModel][%1] It took %2 ms to reset root item")
                     .arg(__FUNCTION__)
                     .arg(timer.elapsed()));

            timer.restart();
#endif

            parseRegexFiltersView(mpRootItem, regexStr);

#ifdef DEBUG_BUILD
            SEND_MSG(QString("[CFiltersModel][%1] It took %2 ms to parse regex")
                     .arg(__FUNCTION__)
                     .arg(timer.elapsed()));

            timer.restart();
#endif

            updateVarGroupsMap();

#ifdef DEBUG_BUILD
            SEND_MSG(QString("[CFiltersModel][%1] It took %2 ms to update var group map")
                     .arg(__FUNCTION__)
                     .arg(timer.elapsed()));

            timer.restart();
#endif

            filterRegexTokensInternal();

#ifdef DEBUG_BUILD
            SEND_MSG(QString("[CFiltersModel][%1] It took %2 ms to filter the tokens")
                     .arg(__FUNCTION__)
                     .arg(timer.elapsed()));
#endif
        }
    }
}

void CFiltersModel::updateVarGroupsMap()
{
    if(nullptr != mpRootItem)
    {
        mVarGroupsMap.clear();

        auto preVisitFunction = [this](tTreeItem* pItem)
        {
            if(nullptr != pItem)
            {
                bool bIsVar = eRegexFiltersRowType::VarGroup == pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>();

                if(true == bIsVar)
                {
                    mVarGroupsMap.insert( pItem->data(static_cast<int>(eRegexFiltersColumn::GroupIndex)).get<int>() );
                }
            }

            return true;
        };

        mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);
    }
}

void CFiltersModel::sort(int column, Qt::SortOrder order)
{
    mSortingColumn = toRegexFiltersColumn(column);
    mSortOrder = order;

    if(nullptr != mpRootItem)
    {
        mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);
        filterRegexTokensInternal();
    }

    updateView();
}

QModelIndex CFiltersModel::createIndexInternal(int arow, int acolumn, void *adata) const
{
    QModelIndex result;

    // root node's index should be invalid
    if(reinterpret_cast<CTreeItem*>(adata) != mpRootItem.get())
    {
        result = createIndex(arow, acolumn, adata);
    }

    return result;
}

void CFiltersModel::filterRegexTokens( const QString& filter )
{
    mFilter = filter;
    filterRegexTokensInternal();
}

void CFiltersModel::filterRegexTokensInternal()
{
    //QElapsedTimer time;

    //time.start();

    bool bFilterVariables = getSettingsManager()->getFilterVariables();

    if(nullptr != mpRootItem)
    {
        //SEND_MSG(QString("~0 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));

        QVector<CTreeItem*> visibleElements;

        {
            QRegularExpression regex = QRegularExpression(mFilter, QRegularExpression::CaseInsensitiveOption);

            auto preVisitFunction = [this, &visibleElements, &bFilterVariables, &regex](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    bool bFiltered = false;

                    if(true == bFilterVariables)
                    {
                        auto rowType = pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>();
                        bFiltered = rowType != eRegexFiltersRowType::VarGroup;
                    }

                    if(false == bFiltered)
                    {
                        if(false == mFilter.isEmpty())
                        {
                            if(true == regex.isValid())
                            {
                                const QString& checkString = pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>();

                                QRegularExpressionMatch match = regex.match(checkString);

                                bFiltered = !match.hasMatch();

                                pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = bFiltered;

                                if(false == bFiltered)
                                {
                                    visibleElements.push_back(pItem);
                                }
                            }
                        }
                        else
                        {
                            if(true == bFilterVariables)
                            {
                                auto rowType = pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>();
                                bFiltered = rowType != eRegexFiltersRowType::VarGroup;
                            }
                            else
                            {
                                bFiltered = false;
                            }
                        }
                    }

                    pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = bFiltered;

                    if(false == bFiltered)
                    {
                        visibleElements.push_back(pItem);
                    }
                }

                return true;
            };

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);
        }

        {
            auto preVisitFunction = [this, &visibleElements](tTreeItem* pItem)
            {
                auto pParent = pItem->getParent();

                if(nullptr != pParent && pParent != mpRootItem.get())
                {
                    bool bIsParentFiltered = pParent->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)).get<bool>();
                    bool bIsItemFiltered = pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)).get<bool>();

                    if(false == bIsParentFiltered && bIsItemFiltered != bIsParentFiltered)
                    {
                        // child inherits parents status
                        pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = false;
                        visibleElements.push_back(pItem);
                    }
                }

                return true;
            };

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);
        }

        //SEND_MSG(QString("~1 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));

        for(auto& pVisibleElement : visibleElements)
        {
            if(nullptr != pVisibleElement)
            {
                auto preVisitParentsFunction = [](tTreeItem* pItem)
                {
                    if(nullptr != pItem)
                    {
                        pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = false;
                    }

                    return true;
                };

                pVisibleElement->visitParents(preVisitParentsFunction, CTreeItem::tVisitFunction(), false, false);
            }
        }

        //SEND_MSG(QString("~2 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));

        {
            tFilteredEntryVec filteredEntryVec;

            auto preVisitFunction = [this, &filteredEntryVec](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    tFilteredEntry filteredEntry;
                    filteredEntry.row = pItem->getIdx();
                    filteredEntry.filtered = pItem->data(static_cast<int>(eRegexFiltersColumn::IsFiltered)).get<bool>();

                    auto pParent = pItem->getParent();

                    if(nullptr != pParent)
                    {
                        filteredEntry.parentIdx = createIndexInternal(pParent->getIdx(), 0, pParent);

                        //if(mpRootItem != pParent)
                        //{
                        //    SEND_MSG(QString("[CFiltersModel][%1] Parent |%2|%3|%4|. Parent index |%5|%6|%7| assigned to index |%8|%9|%10|")
                        //             .arg(__FUNCTION__)
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Index)).get<int>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Index)).data().value<int>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::ItemType)).data().value<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Value)).data().value<QString>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Index)).get<int>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>()));
                        //}
                        //else
                        //{
                        //    SEND_MSG(QString("[CFiltersModel][%1] Parent |%2|%3|%4|. Parent index |%5|%6|%7| assigned to index |%8|%9|%10|")
                        //             .arg(__FUNCTION__)
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Index)).get<QString>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Index)).data().value<int>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::ItemType)).data().value<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Value)).data().value<QString>())
                        //            .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Index)).get<int>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>()));
                        //}
                    }

                    filteredEntryVec.push_back(filteredEntry);
                }

                return true;
            };

            if(false == mpRootItem->isWholeSorted())
            {
                mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);
            }

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);

            filteredEntriesChanged(filteredEntryVec, true );
        }
    }

    //SEND_MSG(QString("~3 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));
}

void CFiltersModel::addCompletionData( const tFoundMatches& foundMatches )
{
    if(false == mVarGroupsMap.empty())
    {
        for(const auto& foundMatch : foundMatches.foundMatchesVec)
        {
            auto foundVarGroup = mVarGroupsMap.find(foundMatch.idx);

            if(foundVarGroup != mVarGroupsMap.end())
            {
                mCompletionCache[foundMatch.idx].insert( foundMatch.matchStr );
            }
        }
    }
}

void CFiltersModel::resetCompletionData()
{
    mCompletionCache.clear();
}

QStringList CFiltersModel::getCompletionData( const int& groupIndex,
                                              const QString& input,
                                              const int& maxNumberOfSuggestions,
                                              const int& maxLengthOfSuggestions )
{
    QStringList result;

    auto foundCompletionSet = mCompletionCache.find(groupIndex);

    if(foundCompletionSet != mCompletionCache.end())
    {
        int numberOfSuggestions = 0;

        auto caseSensitiveOption = getSettingsManager()->getFiltersCompletion_CaseSensitive() ?
                    Qt::CaseSensitive :
                    Qt::CaseInsensitive;

        for(const auto& completionItem : foundCompletionSet->second)
        {
            bool bStringFound = false;

            if(false == getSettingsManager()->getFiltersCompletion_SearchPolicy())
            {
                bStringFound = completionItem.startsWith(input, caseSensitiveOption);
            }
            else
            {
                bStringFound = completionItem.contains(input, caseSensitiveOption);
            }

            if(bStringFound)
            {
                result.push_back(completionItem.mid(0, maxLengthOfSuggestions));
                ++numberOfSuggestions;
            }

            if(numberOfSuggestions >= maxNumberOfSuggestions)
            {
                break;
            }
        }
    }

    return result;
}

PUML_PACKAGE_BEGIN(DMA_FiltersView)
    PUML_CLASS_BEGIN_CHECKED(CFiltersModel)
        PUML_INHERITANCE_CHECKED(IFiltersModel, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTreeItem, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
