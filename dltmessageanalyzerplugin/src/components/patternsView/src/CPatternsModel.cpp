/**
 * @file    CPatternsModel.cpp
 * @author  vgoncharuk
 * @brief   Declaration of the CPatternsModel.cpp class
 */

#include "CPatternsModel.hpp"

#include <QColor>
#include <QDebug>
#include <QStack>
#include <QRegularExpression>

#include "common/Definitions.hpp"
#include "common/CTreeItem.hpp"
#include "components/settings/api/ISettingsManager.hpp"

#include "DMA_Plantuml.hpp"

Q_DECLARE_METATYPE(CPatternsModel::tFilteredEntry)

static const char* sTreeLevelSeparator = "_";
static const char* sUsedCombinationLiteral = "%comb";
static const char* sUsedDefaultLiteral = "%def";

static QVector<tTreeItemPtr> sortingFunction (const QVector<tTreeItemPtr>& children,
        const int& sortingColumn,
        Qt::SortOrder sortingOrder)
{
    tTreeItem::tChildrenVector result;

    switch(static_cast<ePatternsColumn>(sortingColumn))
    {
        case ePatternsColumn::Alias:
        case ePatternsColumn::AliasTreeLevel:
        case ePatternsColumn::Regex:
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
        case ePatternsColumn::Default:
        case ePatternsColumn::Combine:
        {
            switch(sortingOrder)
            {
                case Qt::SortOrder::AscendingOrder:
                {
                    struct tComparator
                    {
                        Qt::CheckState bFlag;
                        QString alias;
                        bool operator< (const tComparator& rVal) const
                        {
                            bool bResult = false;

                            if( bFlag > rVal.bFlag )
                            {
                                bResult = true;
                            }
                            else if( bFlag < rVal.bFlag )
                            {
                                bResult = false;
                            }
                            else
                            {
                                bResult = alias.compare( rVal.alias, Qt::CaseInsensitive ) < 0;
                            }

                            return bResult;
                        }
                    };

                    QMultiMap<tComparator, tTreeItemPtr> sortedChildrenMap;

                    for(const auto& pChild : children)
                    {
                        if(nullptr != pChild)
                        {
                            const QString& subString = pChild->data(static_cast<int>(ePatternsColumn::AliasTreeLevel)).get<QString>();
                            auto bFlag = V_2_CS( pChild->data(sortingColumn) );

                            tComparator comparator;
                            comparator.alias = subString;
                            comparator.bFlag = bFlag;

                            sortedChildrenMap.insert( comparator, pChild);
                        }
                    }

                    for(const auto& pChild : sortedChildrenMap)
                    {
                        if(nullptr != pChild)
                        {
                            result.push_back(pChild);
                        }
                    }
                }
                    break;
                case Qt::SortOrder::DescendingOrder:
                {
                    struct tComparator
                    {
                        Qt::CheckState bFlag;
                        QString alias;
                        bool operator< (const tComparator& rVal) const
                        {
                            bool bResult = false;

                            if( bFlag > rVal.bFlag )
                            {
                                bResult = false;
                            }
                            else if( bFlag < rVal.bFlag )
                            {
                                bResult = true;
                            }
                            else
                            {
                                bResult = alias.compare( rVal.alias, Qt::CaseInsensitive ) < 0;
                            }

                            return bResult;
                        }
                    };

                    QMultiMap< tComparator, tTreeItemPtr> sortedChildrenMap;

                    for(const auto& pChild : children)
                    {
                        if(nullptr != pChild)
                        {
                            const QString& subString = pChild->data(static_cast<int>(ePatternsColumn::AliasTreeLevel)).get<QString>();
                            auto bFlag = V_2_CS( pChild->data(sortingColumn) );

                            tComparator comparator;
                            comparator.alias = subString;
                            comparator.bFlag = bFlag;

                            sortedChildrenMap.insert( comparator, pChild);
                        }
                    }

                    for(const auto& pChild : sortedChildrenMap)
                    {
                        if(nullptr != pChild)
                        {
                            result.push_back(pChild);
                        }
                    }
                }
                    break;
            }
        }
            break;
        case ePatternsColumn::AfterLastVisible:
        case ePatternsColumn::RowType:
        case ePatternsColumn::Last:
        case ePatternsColumn::IsFiltered:
            break;
    }

    return result;
}

CPatternsModel::CPatternsModel(const tSettingsManagerPtr& pSettingsManager,
                               QObject *parent):
    IPatternsModel(parent),
    CSettingsManagerClient(pSettingsManager),
    mpRootItem(nullptr),
    mSortingColumn(ePatternsColumn::AliasTreeLevel),
    mSortOrder(Qt::SortOrder::DescendingOrder),
    mFilter()
{
    mpRootItem = new CTreeItem(nullptr,
                               static_cast<int>(ePatternsColumn::AliasTreeLevel),
                               sortingFunction,
                               CTreeItem::tHandleDuplicateFunc(),
                               CTreeItem::tFindItemFunc());

    mpRootItem->appendColumn( getName(ePatternsColumn::AliasTreeLevel) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Default) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Combine) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Regex) );
    mpRootItem->appendColumn( getName(ePatternsColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Alias) );
    mpRootItem->appendColumn( getName(ePatternsColumn::IsFiltered) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Last) );
}

CPatternsModel::~CPatternsModel()
{
    if(mpRootItem)
    {
        delete mpRootItem;
        mpRootItem = nullptr;
    }
}

void CPatternsModel::updateView()
{
    emit dataChanged( index(0,0), index ( rowCount(), columnCount()) );
    emit layoutChanged();
}

QModelIndex CPatternsModel::index(int row, int column,
                  const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    tTreeItemPtr pParentItem;

    if (!parent.isValid())
        pParentItem = mpRootItem;
    else
        pParentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    if(nullptr != pParentItem)
    {
        tTreeItem *childItem = pParentItem->child(row);
        if (childItem)
            return createIndexInternal(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex CPatternsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if( 0 == mpRootItem->childCount() )
    {
        if( 0 == index.row() )
        {
            tTreeItem *childItem = static_cast<tTreeItemPtr>(index.internalPointer());
            tTreeItem *parentItem = childItem->getParent();

            if(parentItem != mpRootItem)
            {
                return QModelIndex();
            }
        }
        else
        {
            return QModelIndex();
        }
    }

    tTreeItem *childItem = static_cast<tTreeItemPtr>(index.internalPointer());
    tTreeItem *parentItem = childItem->getParent();

    if (nullptr == parentItem)
        return QModelIndex();

    return createIndexInternal(parentItem->row(), 0, parentItem);
}

int CPatternsModel::rowCount(const QModelIndex &parent) const
{
    int result = 0;

    tTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mpRootItem;
    else
        parentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    if(nullptr != parentItem)
    {
        parentItem->sort(static_cast<int>(mSortingColumn), mSortOrder, false);
        result =  parentItem->childCount();
    }

    return result;
}

static CTreeItem::tFindItemResult findItemFunc(const CTreeItem* pItem, const CTreeItem::tData& key)
{
    CTreeItem::tFindItemResult result;
    result.bFound = false;

    auto keyColumn = static_cast<int>(ePatternsColumn::AliasTreeLevel);

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
}

int CPatternsModel::columnCount(const QModelIndex &) const
{
     return static_cast<int>(ePatternsColumn::Last);
}

void CPatternsModel::resetData()
{
    beginResetModel();
    if(mpRootItem)
        delete mpRootItem;

    mpRootItem = new tTreeItem(nullptr,
                               static_cast<int>(ePatternsColumn::AliasTreeLevel),
                               sortingFunction,
                               CTreeItem::tHandleDuplicateFunc(),
                               findItemFunc);
    mpRootItem->appendColumn( getName(ePatternsColumn::AliasTreeLevel) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Default) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Combine) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Regex) );
    mpRootItem->appendColumn( getName(ePatternsColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Alias) );
    mpRootItem->appendColumn( getName(ePatternsColumn::IsFiltered) );
    mpRootItem->appendColumn( getName(ePatternsColumn::Last) );
    endResetModel();
    updateView();
}

QVariant CPatternsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QVariant result;

    if (role == Qt::DisplayRole && ( index.column() != static_cast<int>(ePatternsColumn::Default) &&
                                     index.column() != static_cast<int>(ePatternsColumn::Combine) ) )
    {
        tTreeItem *item = static_cast<tTreeItemPtr>(index.internalPointer());
        result = toQVariant( item->data(index.column()) );
    }
    else if( ( role == Qt::DisplayRole || role == Qt::CheckStateRole ) && ( index.column() == static_cast<int>(ePatternsColumn::Default) ||
             index.column() == static_cast<int>(ePatternsColumn::Combine) ) )
    {
        tTreeItem *item = static_cast<tTreeItemPtr>(index.internalPointer());
        result = V_2_CS( item->data(index.column()) );
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if(static_cast<int>(ePatternsColumn::Default) == index.column() ||
           static_cast<int>(ePatternsColumn::Combine) == index.column())
        {
            result = Qt::AlignLeft;
        }
    }
    else if (role == Qt::BackgroundRole)
    {
        result = QColor(0,0,0,0);
    }

    return result;
}

Qt::ItemFlags CPatternsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::NoItemFlags;

    if (true == index.isValid())
    {
        if ( index.column() == static_cast<int>(ePatternsColumn::Default) ||
             index.column() == static_cast<int>(ePatternsColumn::Combine))
        {
            result = QAbstractItemModel::flags(index) | Qt::ItemIsAutoTristate;
        }
        else
        {
            result = QAbstractItemModel::flags(index);
        }
    }

    return result;
}

QVariant CPatternsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return toQVariant( mpRootItem->data(section) );

    return QVariant();
}

QModelIndex CPatternsModel::addData(const QString& alias, const QString& regex, Qt::CheckState isDefault)
{
    return addData(alias, regex, isDefault, isDefault);
}

QModelIndex CPatternsModel::addData(const QString& alias, const QString& regex, Qt::CheckState isCombine, Qt::CheckState isDefault)
{
    QModelIndex result;

    if(nullptr != mpRootItem)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QStringList aliasParts = alias.split(sTreeLevelSeparator, QString::SkipEmptyParts);
#else
        QStringList aliasParts = alias.split(sTreeLevelSeparator, Qt::SkipEmptyParts);
#endif

        QString finalAlias;

        CTreeItem::tDataVec dataVec;

        int counter = 0;

        for(const auto& aliasPart : aliasParts)
        {
            ++counter;

            CTreeItem::tData data;
            data.push_back(tDataItem(aliasPart));

            if(false == finalAlias.isEmpty())
            {
                finalAlias.append(sTreeLevelSeparator);
            }

            finalAlias.append(aliasPart);

            if(counter == aliasParts.size())
            {
                data.push_back(tDataItem(isDefault));
                data.push_back(tDataItem(isCombine));
                data.push_back(tDataItem(regex));
                data.push_back(tDataItem(QString(""))); // after last visible
                data.push_back(tDataItem(finalAlias));
                data.push_back(tDataItem(ePatternsRowType::ePatternsRowType_Alias));
                data.push_back(tDataItem(false));
            }
            else
            {
                data.push_back(tDataItem(Qt::Unchecked));
                data.push_back(tDataItem(Qt::Unchecked));
                data.push_back(tDataItem(QString("")));
                data.push_back(tDataItem(QString(""))); // after last visible
                data.push_back(tDataItem(finalAlias));
                data.push_back(tDataItem(ePatternsRowType::ePatternsRowType_FakeTreeLevel));
                data.push_back(tDataItem(false));
            }

            dataVec.push_back(data);
        }

        auto addedItems = mpRootItem->addData(dataVec);

        if(false == addedItems.isEmpty())
        {
            for(auto& pAddedItem : addedItems)
            {
                auto pAddedItemParent = pAddedItem->getParent();

                if(nullptr != pAddedItemParent)
                {
                    pAddedItemParent->sort(static_cast<int>(mSortingColumn), mSortOrder, false);

                    pAddedItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);

                    auto parentIdx = createIndexInternal(pAddedItemParent->getIdx(), 0, pAddedItemParent);

                    beginInsertRows( parentIdx, pAddedItem->getIdx(), pAddedItem->getIdx() );

                    endInsertRows();
                }
            }

            tChangedIndexes updatedIndexes;

            // to properly set flags we need iterate from end to beginning
            for(auto it = addedItems.end() - 1; it != addedItems.begin() - 1; --it)
            {
                auto pAddedItem = *it;

                if(nullptr != pAddedItem)
                {
                    auto addedItemIndex = createIndexInternal(pAddedItem->row(), 0, pAddedItem);
                    auto updDefault = setIsDefault( addedItemIndex,
                                  V_2_CS( addedItemIndex.sibling(addedItemIndex.row(), static_cast<int>(ePatternsColumn::Default) ).data() ),
                                  false, false);
                    auto updCombined = setIsCombine( addedItemIndex,
                                  V_2_CS( addedItemIndex.sibling(addedItemIndex.row(), static_cast<int>(ePatternsColumn::Combine) ).data() ),
                                  false,
                                  false);

                    updatedIndexes.unite(updDefault);
                    updatedIndexes.unite(updCombined);
                }
            }

            if(false == updatedIndexes.isEmpty())
            {
                auto minIdx = std::min_element(updatedIndexes.begin(), updatedIndexes.end());
                auto maxIdx = std::max_element(updatedIndexes.begin(), updatedIndexes.end());
                dataChanged(*minIdx, *maxIdx);

                filterPatternsInternal(false);
            }
        }

        if(false == addedItems.isEmpty() && nullptr != addedItems.front())
        {
            auto pBottomAddedItem = addedItems.front();
            result = createIndexInternal(pBottomAddedItem->getIdx(), 0, pBottomAddedItem);
        }
    }

    return result;
}

QStringList CPatternsModel::getSearchKeys( const QString& alias ) const
{
    return alias.split(sTreeLevelSeparator);
}

CPatternsModel::tSearchResult CPatternsModel::search( const QString& alias )
{
    tSearchResult result;

    if(false == alias.isEmpty())
    {
        if(nullptr != mpRootItem)
        {
            auto searchKeys = getSearchKeys(alias);
            auto pFoundItem = mpRootItem->search(searchKeys);

            if(nullptr != pFoundItem)
            {
                result.bFound = true;
                result.foundIdx = createIndexInternal(pFoundItem->getIdx(), 0, pFoundItem);
            }
        }
    }
    else
    {
        if(nullptr != mpRootItem)
        {
            result.bFound = true;
            result.foundIdx = createIndexInternal(mpRootItem->getIdx(), 0, mpRootItem);
        }
    }

    return result;
}

QModelIndex CPatternsModel::editData(const QModelIndex& idx, const QString& alias, const QString& regex, Qt::CheckState isDefault, Qt::CheckState isCombine)
{
    QModelIndex result = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::AliasTreeLevel));

    if(false == idx.isValid())
        return QModelIndex();

    auto pElement = static_cast<CTreeItem*>(idx.internalPointer());

    if(nullptr != pElement)
    {
        const QString& old_alias = pElement->getWriteableData(static_cast<int>(ePatternsColumn::Alias)).get<QString>();

        if(old_alias == alias) // update of already existing item without change of its alias
        {
            pElement->getWriteableData(static_cast<int>(ePatternsColumn::Regex)) = regex;
            pElement->getWriteableData(static_cast<int>(ePatternsColumn::Default)) = isDefault;
            pElement->getWriteableData(static_cast<int>(ePatternsColumn::Combine)) = isCombine;

            // if there is empty regex & item hsa children
            if( true == regex.isEmpty() && 0 != pElement->childCount() )
            {
                // let's update it's row type
                pElement->getWriteableData(static_cast<int>(ePatternsColumn::RowType)) = ePatternsRowType::ePatternsRowType_FakeTreeLevel;
            }

            updateSubTree(idx);
        }
        else // re-naming of already existing item
        {
            struct tUpdateItem
            {
                QString alias;
                QString regex;
                Qt::CheckState isDefault;
                Qt::CheckState isCombine;
            };

            typedef QVector<tUpdateItem> tUpdateItemVec;

            tUpdateItemVec updateItemVec;

            QStack<QString> nameStack;

            auto preVisitFunction = [&pElement, &nameStack](tTreeItem* pItem)
            {
                if(nullptr != pItem && pItem != pElement)
                {
                    nameStack.push( pItem->data(static_cast<int>(ePatternsColumn::AliasTreeLevel)).get<QString>() );
                }

                return true;
            };

            auto postVisitFunction = [&pElement, &alias, &regex, &isDefault, &isCombine, &nameStack, &updateItemVec](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    auto rowType = pItem->data(static_cast<int>(ePatternsColumn::RowType)).get<ePatternsRowType>();

                    // we are adding only real meaningful patterns, not fakes
                    if(ePatternsRowType::ePatternsRowType_Alias == rowType)
                    {
                        QString newAlias = alias;
                        QString newRegex( regex );

                        auto getCheckStateFromParentCheckState = [](Qt::CheckState parentVal, Qt::CheckState myCurVal)->Qt::CheckState
                        {
                            Qt::CheckState res = Qt::Checked;

                            switch(parentVal)
                            {
                                case Qt::Checked:
                                res = Qt::Checked;
                                    break;
                                case Qt::Unchecked:
                                res = Qt::Unchecked;
                                    break;
                                case Qt::PartiallyChecked:
                                res = myCurVal;
                                    break;
                            }

                            return res;
                        };

                        if(pItem != pElement) // if it is not the initially changed element
                        {
                            for(const auto& namePart : nameStack)
                            {
                                if(false == newAlias.isEmpty())
                                {
                                    newAlias.append(sTreeLevelSeparator);
                                }

                                newAlias.append(namePart);
                            }

                            // let's get element's regex, as it is not changing, only parent's one is changing
                            newRegex = pItem->data(static_cast<int>(ePatternsColumn::Regex)).get<QString>();
                        }

                        tUpdateItem updateItem;
                        updateItem.alias = newAlias;
                        updateItem.regex = newRegex;
                        updateItem.isCombine = getCheckStateFromParentCheckState( isCombine,
                                                                                  V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Combine))));
                        updateItem.isDefault = getCheckStateFromParentCheckState( isDefault,
                                                                                  V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Default))));

                        updateItemVec.push_back(updateItem);
                    }

                    if(pItem != pElement)
                    {
                        nameStack.pop();
                    }
                }

                return true;
            };

            pElement->visit(preVisitFunction, postVisitFunction, true);

            //let's remove existing pattern
            auto removeIdx = createIndexInternal(pElement->row(), 0, pElement);
            removeData(removeIdx); // let's remove old item

            for( const auto& updateItem : updateItemVec )
            {
                addData(updateItem.alias, updateItem.regex, updateItem.isCombine, updateItem.isDefault);
            }

            // search for element to jump to
            auto searchRes = search(alias);

            if(true == searchRes.bFound)
            {
                result = searchRes.foundIdx;
                filterPatternsInternal(true);
            }
        }
    }

    return result;
}

void CPatternsModel::removeData(const QModelIndex& idx)
{
    if(false == idx.isValid())
        return;

    auto* pElement = static_cast<CTreeItem*>(idx.internalPointer());

    if(nullptr != pElement)
    {
        // remove parents
        auto visitParentsFunction = [this, &pElement](CTreeItem* pVisitParent)
        {
            bool bResult = true;

            if(nullptr != pVisitParent)
            {
                // if element has no children or it is a selected element
                if( (0 == pVisitParent->childCount() &&
                     ePatternsRowType::ePatternsRowType_FakeTreeLevel
                     == pVisitParent->data(static_cast<int>(ePatternsColumn::RowType)).get<ePatternsRowType>())
                    || pVisitParent == pElement)
                {
                    auto* pGrandParent = pVisitParent->getParent();

                    if(nullptr != pGrandParent)
                    {
                        auto pGrandParentIdx = createIndexInternal(pGrandParent->getIdx(), 0, pGrandParent);
                        beginRemoveRows(pGrandParentIdx, pVisitParent->getIdx(), pVisitParent->getIdx());
                        pGrandParent->removeChild( pVisitParent->data(static_cast<int>(ePatternsColumn::AliasTreeLevel) ).get<QString>() );
                        endRemoveRows();
                    }
                    else
                    {
                        bResult = false;
                        updateSubTree(createIndexInternal(pVisitParent->getIdx(), 0, pVisitParent));
                    }
                }
                else
                {
                    bResult = false;
                    updateSubTree(createIndexInternal(pVisitParent->getIdx(), 0, pVisitParent));
                }
            }
            else
            {
                bResult = false;
            }

            return bResult;
        };

        pElement->visitParents(visitParentsFunction, CTreeItem::tVisitFunction(), true, false);
    }

    filterPatternsInternal(false);
}

void CPatternsModel::sort(int column, Qt::SortOrder order)
{
    mSortingColumn = toPatternsColumn(column);
    mSortOrder = order;

    if(nullptr != mpRootItem)
    {
        mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);
        filterPatternsInternal(false);
    }

    updateView();
}

void CPatternsModel::updateSubTree(const QModelIndex& idx)
{
    if(true == idx.isValid())
    {
        auto* pElement = static_cast<CTreeItem*>(idx.internalPointer());

        if(nullptr != pElement)
        {
            pElement->sort(static_cast<int>(mSortingColumn), mSortOrder, true);

            auto firstColumnIdx = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::AliasTreeLevel));
            auto lastColumnIdx = idx.sibling(idx.row(), static_cast<int>(ePatternsColumn::Last) - 1);

            emit dataChanged( firstColumnIdx, lastColumnIdx );
            emit layoutChanged();
        }
    }
}

void CPatternsModel::updatePatternsInPersistency()
{
    if(nullptr != mpRootItem)
    {
        ISettingsManager::tAliasItemMap aliasMap;

        auto preVisitFunction = [&aliasMap](const tTreeItem* pItem)
        {
            if(nullptr != pItem)
            {
                auto rowType = pItem->data(static_cast<int>(ePatternsColumn::RowType))
                        .get<ePatternsRowType>();

                if(ePatternsRowType::ePatternsRowType_Alias == rowType)
                {
                    const QString& regex = pItem->data(static_cast<int>(ePatternsColumn::Regex)).get<QString>();

                    if( false == regex.isEmpty() )
                    {
                        auto isDefault = V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Default)) );

                        const QString& alias = pItem->data(static_cast<int>(ePatternsColumn::Alias)).get<QString>();

                        ISettingsManager::tAliasItem aliasItem(isDefault == Qt::Checked, alias, regex);
                        aliasMap.insert(alias, aliasItem);
                    }
                }
            }

            return true;
        };

        mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);

        // if something has changed
        if(aliasMap != getSettingsManager()->getAliases())
        {
            // let's update them in the persistency
            getSettingsManager()->setAliases( aliasMap );
        }
    }
}

CPatternsModel::tChangedIndexes CPatternsModel::setIsDefault( const QModelIndex& idx, Qt::CheckState checkState, bool updateFilter, bool updateView )
{
    tChangedIndexes result;

    if(false == idx.isValid())
    {
        return result;
    }

    auto pElement = static_cast<CTreeItem*>(idx.internalPointer());

    if(nullptr != pElement)
    {
        pElement->getWriteableData(static_cast<int>(ePatternsColumn::Default)) = checkState;
        result.insert(idx);

        auto visitParentsFunction = [this, &result](CTreeItem* pVisitParent)
        {
            if(nullptr != pVisitParent)
            {
                auto updateIdx = createIndexInternal(pVisitParent->getIdx(),
                                                     static_cast<int>(ePatternsColumn::Default),
                                                     pVisitParent);
                auto& val = pVisitParent->getWriteableData(static_cast<int>(ePatternsColumn::Default));
                val = getIndexCheckState(updateIdx);
                result.insert(updateIdx);
            }

            return true;
        };

        pElement->visitParents( visitParentsFunction, CTreeItem::tVisitFunction(), false, false );

        auto visitChildrenFunction = [this, &checkState, &result](CTreeItem* pChild)
        {
            if(nullptr != pChild)
            {
                auto updateIdx = createIndexInternal(pChild->getIdx(),
                                                     static_cast<int>(ePatternsColumn::Default),
                                                     pChild);
                auto& val = pChild->getWriteableData(static_cast<int>(ePatternsColumn::Default));
                val = checkState;
                result.insert(updateIdx);
            }

            return true;
        };

        pElement->visit( visitChildrenFunction, CTreeItem::tVisitFunction(), false );
    }

    if(true == updateFilter)
    {
        filterPatternsInternal(false);
    }

    if(true == updateView)
    {
        if(false == result.isEmpty())
        {
            auto minIdx = std::min_element(result.begin(), result.end());
            auto maxIdx = std::max_element(result.begin(), result.end());
            dataChanged(*minIdx, *maxIdx);
        }
    }

    return result;
}

CPatternsModel::tChangedIndexes CPatternsModel::setIsCombine( const QModelIndex& idx, Qt::CheckState checkState, bool updateFilter, bool updateView  )
{
    tChangedIndexes result;

    if(false == idx.isValid())
    {
        return result;
    }

    auto pElement = static_cast<CTreeItem*>(idx.internalPointer());

    if(nullptr != pElement)
    {
        pElement->getWriteableData(static_cast<int>(ePatternsColumn::Combine)) = checkState;
        result.insert(idx);

        auto visitParentsFunction = [this, &result](CTreeItem* pVisitParent)
        {
            if(nullptr != pVisitParent)
            {
                auto updateIdx = createIndexInternal(pVisitParent->getIdx(),
                                                     static_cast<int>(ePatternsColumn::Combine),
                                                     pVisitParent);
                auto& val = pVisitParent->getWriteableData(static_cast<int>(ePatternsColumn::Combine));
                val = getIndexCheckState(updateIdx);
                result.insert(updateIdx);
            }

            return true;
        };

        pElement->visitParents( visitParentsFunction, CTreeItem::tVisitFunction(), false, false );

        auto visitChildrenFunction = [this, &checkState, &result](CTreeItem* pChild)
        {
            if(nullptr != pChild)
            {
                auto updateIdx = createIndexInternal(pChild->getIdx(),
                                                     static_cast<int>(ePatternsColumn::Combine),
                                                     pChild);
                auto& val = pChild->getWriteableData(static_cast<int>(ePatternsColumn::Combine));
                val = checkState;
                result.insert(updateIdx);
            }

            return true;
        };

        pElement->visit( visitChildrenFunction, CTreeItem::tVisitFunction(), false );
    }

    if(true == updateFilter)
    {
        filterPatternsInternal(false);
    }

    if(true == updateView)
    {
        if(false == result.isEmpty())
        {
            auto minIdx = std::min_element(result.begin(), result.end());
            auto maxIdx = std::max_element(result.begin(), result.end());
            dataChanged(*minIdx, *maxIdx);
        }
    }

    return result;
}

Qt::CheckState CPatternsModel::getIndexCheckState( const QModelIndex& idx) const
{
    Qt::CheckState result = Qt::Unchecked;

    if(false == idx.isValid())
    {
        return result;
    }

    tTreeItem *pItem = static_cast<tTreeItemPtr>(idx.internalPointer());

    if(nullptr != pItem)
    {
        if(0 == pItem->childCount())
        {
            result = V_2_CS( pItem->data(idx.column()) );
        }
        else
        {
            bool bSomeChildrenSelected = false;
            bool bAllChildrenSelected = true;

            const auto& children = pItem->getChildren();

            for( const auto& pChild : children )
            {
                if(nullptr != pChild)
                {
                    if( Qt::Checked == V_2_CS( pChild->data(idx.column() ) ) )
                    {
                        bSomeChildrenSelected = true;
                    }
                    else if( Qt::PartiallyChecked == V_2_CS( pChild->data(idx.column() ) ) )
                    {
                        bSomeChildrenSelected = true;
                        bAllChildrenSelected = false;
                        break;
                    }
                    else
                    {
                        bAllChildrenSelected = false;

                        if(true == bSomeChildrenSelected)
                        {
                            break;
                        }
                    }
                }
            }

            if( true == bAllChildrenSelected )
            {
                result = Qt::Checked;
            }
            else if( true == bSomeChildrenSelected )
            {
                result = Qt::PartiallyChecked;
            }
            else
            {
                result = Qt::Unchecked;
            }
        }
    }

    return result;
}

bool CPatternsModel::areAnyCombinedPatternsAvailable() const
{
    bool bResult = false;

    if(nullptr != mpRootItem)
    {
        auto preVisitFunction = [&bResult](const tTreeItem* pItem)
        {
            if(nullptr != pItem)
            {
                auto combineCheckedState = V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Combine)) );
                bResult = combineCheckedState == Qt::Checked;
            }

            return !bResult;
        };

        mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);
    }

    return bResult;
}

void CPatternsModel::visit( const tVisitFunction& preVisitFunction,
                            const tVisitFunction& postVisitFunction,
                            bool visitParentItem,
                            QModelIndex parentIdx) const
{
    if(nullptr != mpRootItem)
    {
        auto preVisitFunction_ = [this, &preVisitFunction](const tTreeItem* pItem)
        {
            bool bResult = true;

            if(preVisitFunction)
            {
                QModelIndex idx = createIndexInternal(pItem->getIdx(), 0, const_cast<tTreeItem*>(pItem) );
                bResult = preVisitFunction( idx );
            }

            return bResult;
        };

        auto postVisitFunction_ = [this, &postVisitFunction](const tTreeItem* pItem)
        {
            bool bResult = true;

            if(postVisitFunction)
            {
                QModelIndex idx = createIndexInternal(pItem->getIdx(), 0, const_cast<tTreeItem*>(pItem) );
                bResult = postVisitFunction( idx );
            }

            return bResult;
        };

        if(true == parentIdx.isValid())
        {
            tTreeItem *pItem = static_cast<tTreeItemPtr>(parentIdx.internalPointer());
            pItem->visit(preVisitFunction_, postVisitFunction_, visitParentItem);
        }
        else
        {
            mpRootItem->visit(preVisitFunction_, postVisitFunction_, visitParentItem);
        }
    }
}

QModelIndex CPatternsModel::createIndexInternal(int arow, int acolumn, void *adata) const
{
    //qDebug() << "createIndexInternal: arow - " << arow << "; acolumn - " << acolumn << "; adata - " << adata;
    QModelIndex result;

    // root node's index should be invalid
    if(reinterpret_cast<CTreeItem*>(adata) != mpRootItem)
    {
        result = createIndex(arow, acolumn, adata);
    }

    return result;
}

void CPatternsModel::resetPatternsToDefault()
{
    if(nullptr != mpRootItem)
    {
        tChangedIndexes updatedIndexes;

        auto preVisitFunction = [this, &updatedIndexes](tTreeItem* pItem)
        {
            if(nullptr != pItem)
            {
                auto isDefault = V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Default)) );
                auto isCombine = V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Combine)) );

                if(isDefault != isCombine)
                {
                    auto updIndexesCombine = setIsCombine( createIndexInternal( pItem->getIdx(), static_cast<int>(ePatternsColumn::Combine), pItem ), isDefault, false, false );
                    updatedIndexes.unite(updIndexesCombine);
                }
            }

            return true;
        };

        mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);

        if(false == updatedIndexes.isEmpty())
        {
            auto minIdx = std::min_element(updatedIndexes.begin(), updatedIndexes.end());
            auto maxIdx = std::max_element(updatedIndexes.begin(), updatedIndexes.end());
            dataChanged(*minIdx, *maxIdx);

            filterPatternsInternal(false);
        }
    }
}

void CPatternsModel::clearSelectedPatterns()
{
    if(nullptr != mpRootItem)
    {
        tChangedIndexes updatedIndexes;

        auto preVisitFunction = [this, &updatedIndexes](tTreeItem* pItem)
        {
            if(nullptr != pItem)
            {
                auto updIndexesCombine = setIsCombine( createIndexInternal( pItem->getIdx(),
                                                       static_cast<int>(ePatternsColumn::Combine), pItem ),
                                                       Qt::Unchecked,
                                                       false,
                                                       false);
                updatedIndexes.unite(updIndexesCombine);
            }

            return true;
        };

        mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, false);

        if(false == updatedIndexes.isEmpty())
        {
            auto minIdx = std::min_element(updatedIndexes.begin(), updatedIndexes.end());
            auto maxIdx = std::max_element(updatedIndexes.begin(), updatedIndexes.end());
            dataChanged(*minIdx, *maxIdx);
        }

        filterPatternsInternal(false);
    }
}

QString CPatternsModel::getAliasEditName( const QModelIndex& idx )
{
    QString result;

    tTreeItem *pItem = static_cast<tTreeItemPtr>(idx.internalPointer());

    if(nullptr != pItem)
    {
        auto rowType = pItem->data(static_cast<int>(ePatternsColumn::RowType)).get<ePatternsRowType>();

        switch(rowType)
        {
            case ePatternsRowType::ePatternsRowType_Alias:
            result = pItem->data(static_cast<int>(ePatternsColumn::Alias)).get<QString>();
                break;
            case ePatternsRowType::ePatternsRowType_FakeTreeLevel:
            {
                auto visitParentsFunction = [&result, &pItem](CTreeItem* pVisitParent)
                {
                    if(nullptr != pVisitParent)
                    {
                        result.append(pVisitParent->data(static_cast<int>(ePatternsColumn::AliasTreeLevel)).get<QString>());

                        if(pVisitParent != pItem) // for all, except last element
                        {
                            result.append(sTreeLevelSeparator); // let's add _ separator to name
                        }
                    }

                    return true;
                };

                pItem->visitParents( CTreeItem::tVisitFunction(), visitParentsFunction, true, false );
            }
                break;
        }
    }

    return result;
}

void CPatternsModel::filterPatternsInternal(bool ignoreEmptyFilter)
{
    if(false == ignoreEmptyFilter && true == mFilter.isEmpty())
    {
        return;
    }

    if(nullptr != mpRootItem)
    {
        QRegularExpression splitterRegex( "(?<!\\\\)\\|" );

        auto splitRegex = mFilter.split(splitterRegex);

        bool bIsComb = false;
        bool bIsDef = false;
        QString finalFilter;

        if(true == splitRegex.isEmpty())
        {
            finalFilter = mFilter;
        }
        else
        {
            for( const auto& regexPart : splitRegex )
            {
                auto lowerRegexPart = regexPart.toLower();

                if( lowerRegexPart == sUsedCombinationLiteral)
                {
                    bIsComb = true;
                }
                else if( lowerRegexPart == sUsedDefaultLiteral)
                {
                    bIsDef = true;
                }
                else
                {
                    if(false == finalFilter.isEmpty())
                    {
                        finalFilter.append("|");
                    }

                    finalFilter.append(regexPart);
                }
            }
        }

        QVector<CTreeItem*> visibleElements;

        {
            QRegularExpression regex = QRegularExpression(finalFilter, QRegularExpression::CaseInsensitiveOption);

            auto preVisitFunction = [&visibleElements, &finalFilter, &bIsComb, &bIsDef, &regex](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    bool bFiltered = true;

                    if(true == bFiltered && true == bIsComb)
                    {
                        bFiltered = V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Combine)) ) != Qt::Checked;

                        pItem->getWriteableData(static_cast<int>(ePatternsColumn::IsFiltered)) = bFiltered;

                        if(false == bFiltered)
                        {
                            visibleElements.push_back(pItem);
                        }
                    }

                    if(true == bFiltered && true == bIsDef)
                    {
                        bFiltered = V_2_CS( pItem->data(static_cast<int>(ePatternsColumn::Default)) ) != Qt::Checked;

                        pItem->getWriteableData(static_cast<int>(ePatternsColumn::IsFiltered)) = bFiltered;

                        if(false == bFiltered)
                        {
                            visibleElements.push_back(pItem);
                        }
                    }

                    if(true == bFiltered)
                    {
                        if(false == finalFilter.isEmpty())
                        {
                            if(true == regex.isValid())
                            {
                                const QString& checkString = pItem->data(static_cast<int>(ePatternsColumn::Alias)).get<QString>();

                                QRegularExpressionMatch match = regex.match(checkString);

                                bFiltered = !match.hasMatch();
                            }
                            else
                            {
                                if(false == bIsDef && false == bIsComb)
                                {
                                    bFiltered = false;
                                }
                            }
                        }
                        else
                        {
                            if(false == bIsDef && false == bIsComb)
                            {
                                bFiltered = false;
                            }
                        }
                    }

                    pItem->getWriteableData(static_cast<int>(ePatternsColumn::IsFiltered)) = bFiltered;

                    if(false == bFiltered)
                    {
                        visibleElements.push_back(pItem);
                    }
                }

                return true;
            };

            auto postVisitFunction = [this, &visibleElements](tTreeItem* pItem)
            {
                auto pParent = pItem->getParent();

                if(nullptr != pParent && pParent != mpRootItem &&
                   false == pParent->getWriteableData(static_cast<int>(ePatternsColumn::IsFiltered)).get<bool>())
                {
                    // child inherits parents status
                    pItem->getWriteableData(static_cast<int>(ePatternsColumn::IsFiltered)) = false;
                    visibleElements.push_back(pItem);
                }

                return true;
            };

            mpRootItem->visit(preVisitFunction, postVisitFunction, false);
        }

        for(auto& pVisibleElement : visibleElements)
        {
            if(nullptr != pVisibleElement)
            {
                auto preVisitParentsFunction = [](tTreeItem* pItem)
                {
                    if(nullptr != pItem)
                    {
                        pItem->getWriteableData(static_cast<int>(ePatternsColumn::IsFiltered)) = false;
                    }

                    return true;
                };

                pVisibleElement->visitParents(preVisitParentsFunction, CTreeItem::tVisitFunction(), false, false);
            }
        }

        tFilteredEntryVec filteredEntryVec;

        {
            auto preVisitFunction = [this, &filteredEntryVec](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    tFilteredEntry filteredEntry;
                    filteredEntry.row = pItem->getIdx();
                    filteredEntry.filtered = pItem->data(static_cast<int>(ePatternsColumn::IsFiltered)).get<bool>();

                    auto pParent = pItem->getParent();

                    if(nullptr != pParent)
                    {
                        filteredEntry.parentIdx = createIndexInternal(pParent->getIdx(), 0, pParent);
                    }

                    filteredEntryVec.push_back(filteredEntry);
                }

                return true;
            };

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);

            filteredEntriesChanged(filteredEntryVec, !mFilter.isEmpty());
        }
    }
}

void CPatternsModel::filterPatterns( const QString& filter )
{
    mFilter = filter;
    filterPatternsInternal(true);
}

void CPatternsModel::refreshRegexPatterns()
{
    resetData();

    const auto& aliases = getSettingsManager()->getAliases();
    for(const auto& alias : aliases)
    {
        addData(alias.alias, alias.regex, alias.isDefault ? Qt::Checked : Qt::Unchecked );
    }

    patternsRefreshed();
}

PUML_PACKAGE_BEGIN(DMA_PatternsView)
    PUML_CLASS_BEGIN_CHECKED(CPatternsModel)
        PUML_INHERITANCE_CHECKED(IPatternsModel, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTreeItem, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
