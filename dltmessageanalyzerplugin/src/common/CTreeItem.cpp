/**
 * @file    CTreeItem.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CTreeItem.cpp class
 */

#include <stack>

#include "CTreeItem.hpp"
#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

const int sInvalidIdx = -1;

//CTreeItem
void CTreeItem::visit( const tVisitFunction& preVisitFunction,
                       const tVisitFunction& postVisitFunction,
                       bool visitMe, bool recursive,
                       bool visitSorted )
{
    if(preVisitFunction || postVisitFunction) // if at least one of the functions was provided.
    {
        static_cast<void>(visitInternal(preVisitFunction, postVisitFunction, visitMe, recursive, visitSorted, this));
    }
}

void CTreeItem::visitParents( const tVisitFunction& preVisitFunction,
                              const tVisitFunction& postVisitFunction,
                              bool visitMe,
                              bool visitRoot )
{
    auto isRoot = [](tTreeItemPtr pItem)->bool
    {
        bool bResult = false;

        if(nullptr != pItem && nullptr == pItem->mpParentItem)
        {
            bResult = true;
        }

        return bResult;
    };

    // if we shouldn't visit root and we are root
    if(false == visitRoot && true == isRoot(this))
    {
        // let's return
        return;
    }

    if(preVisitFunction || postVisitFunction) // if at least one of the functions was provided.
    {
        bool bContinue = true;

        std::stack<tTreeItemPtr> parentsStack;

        {
            tTreeItemPtr pAnalyzedParent = nullptr;

            if(true == visitMe)
            {
                pAnalyzedParent = this;
            }
            else
            {
                pAnalyzedParent = mpParentItem;
            }


            while(nullptr != pAnalyzedParent)
            {
                // if we shouldn't visit root and we' are root've reached root
                if(false == visitRoot && true == isRoot(pAnalyzedParent))
                {
                    // let's break the loop
                    break;
                }

                auto pNextAnalyzedParent = pAnalyzedParent->mpParentItem;
                auto parentGuard = pAnalyzedParent->getGuard();
                auto nextParentGuard = pNextAnalyzedParent ? pNextAnalyzedParent->getGuard() : tGuard();

                if(preVisitFunction)
                {
                    if(false == mbFirstLevelSorted)
                    {
                        sort(static_cast<int>(mSortingColumn), mSortOrder, true);
                    }

                    bContinue = preVisitFunction( pAnalyzedParent );
                }

                if(true == bContinue)
                {
                    // here we still collect items to stack, as post function could be still there.
                    if(false == parentGuard.expired()) // if item is alive and was not deleted in client's callback
                    {
                        parentsStack.push(pAnalyzedParent);
                    }

                    if(false == nextParentGuard.expired())
                    {
                        pAnalyzedParent = pNextAnalyzedParent;
                    }
                    else // no sense to go further
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if(true == bContinue)
        {
            if(postVisitFunction) // if post function was provided
            {
                while(false == parentsStack.empty()) // let's visit each parent of the stack
                {
                    // call will be done onlt for alive parents
                    auto* pAnalyzedParent = parentsStack.top();
                    parentsStack.pop();

                    if(false == mbFirstLevelSorted)
                    {
                        sort(static_cast<int>(mSortingColumn), mSortOrder, true);
                    }

                    bContinue = postVisitFunction(pAnalyzedParent);

                    if(false == bContinue)
                    {
                        break;
                    }
                }
            }
        }
    }
}

struct tSafeChildVisitItem
{
    CTreeItem::tGuard guard;
    tTreeItemPtr pItem;
};
typedef QVector<tSafeChildVisitItem> tSafeChildVisitVec;

bool CTreeItem::visitInternal( const tVisitFunction& preVisitFunction,
                               const tVisitFunction& postVisitFunction,
                               bool visitMe, bool recursive,
                               bool visitSorted, tTreeItemPtr pTraversingRoot )
{
    bool bResult = true;

    auto guard = getGuard();

    if(visitMe && bResult && preVisitFunction)
    {
        if(true == visitSorted && false == mbFirstLevelSorted)
        {
            sort(static_cast<int>(mSortingColumn), mSortOrder, true);
        }

        bResult = preVisitFunction(this);

        if(true == guard.expired()) // if item is not alive
        {
            // there is no sense to move on
            bResult = false;
        }
    }

    if(true == bResult)
    {
        tSafeChildVisitVec safeChildVisitVec;

        if(true == visitSorted)
        {
            for( auto pChild : mChildrenVec )
            {
                if(nullptr != pChild)
                {
                    tSafeChildVisitItem visitItem;
                    visitItem.guard = pChild->getGuard();
                    visitItem.pItem = pChild;
                    safeChildVisitVec.push_back(visitItem);
                }
            }
        }
        else
        {
            for( auto pChild : mChildItems )
            {
                if(nullptr != pChild)
                {
                    tSafeChildVisitItem visitItem;
                    visitItem.guard = pChild->getGuard();
                    visitItem.pItem = pChild;
                    safeChildVisitVec.push_back(visitItem);
                }
            }
        }

        for(const auto& visitItem : safeChildVisitVec)
        {
            if(nullptr != visitItem.pItem && false == visitItem.guard.expired())
            {
                if(true == recursive || this == pTraversingRoot)
                {
                    bResult = visitItem.pItem->visitInternal(preVisitFunction, postVisitFunction, true, recursive, visitSorted, pTraversingRoot);

                    if(false == bResult)
                    {
                        break;
                    }
                }
            }
        }
    }

    if(visitMe && bResult && postVisitFunction)
    {
        if(false == guard.expired())
        {
            if(true == visitSorted && false == mbFirstLevelSorted)
            {
                sort(static_cast<int>(mSortingColumn), mSortOrder, true);
            }

            bResult = postVisitFunction(this);
        }
        else
        {
            bResult = false;
        }
    }

    return bResult;
}

CTreeItem::tGuard CTreeItem::getGuard() const
{
    return tGuarded(mpGuard);
}

const CTreeItem::tChildrenMap& CTreeItem::getChildren() const
{
    return mChildItems;
}

void CTreeItem::sort(int column, Qt::SortOrder order, bool recursive)
{
    if( column != mSortingColumn || order != mSortOrder )
    {
        mbFirstLevelSorted = false;
        mbWholeSorted = false;
        mSortingColumn = column;
        mSortOrder = order;
    }

    if(false == mbFirstLevelSorted)
    {
        if(mSortingFunction)
        {
            mSortingFunction(mChildrenVec, mSortingColumn, mSortOrder);

            int counter = 0;

            for(auto& sortedChild : mChildrenVec)
            {
                sortedChild->setIdx(counter++);
            }
        }

        mbFirstLevelSorted = true;
    }

    if( true == recursive )
    {
        if( false == mbWholeSorted )
        {
            for( auto& child : mChildrenVec )
            {
                child->sort(mSortingColumn, mSortOrder, recursive);
            }

            mbWholeSorted = true;
        }
    }
}

void CTreeItem::setIdx(const int& val)
{
    mIdx = val;
}

tTreeItemPtr CTreeItem::search( const QStringList& searchKeys )
{
    tTreeItemPtr pResult = nullptr;

    bool bFound = true;

    tTreeItemPtr pCurrentlyAnalyzedItem = this;

    for( const auto& searchKey : searchKeys )
    {
        auto foundChild = pCurrentlyAnalyzedItem->mChildItems.find(searchKey);

        if( foundChild != pCurrentlyAnalyzedItem->mChildItems.end() )
        {
            pCurrentlyAnalyzedItem = foundChild.value();

            if(nullptr == pCurrentlyAnalyzedItem)
            {
                bFound = false;
            }
        }
        else
        {
            bFound = false;
        }

        if(false == bFound)
        {
            break;
        }
    }

    if(true == bFound)
    {
        pResult = pCurrentlyAnalyzedItem;
    }

    return pResult;
}

tTreeItemPtr CTreeItem::getParent() const
{
    return mpParentItem;
}

bool CTreeItem::isWholeSorted() const
{
    return mbWholeSorted;
}


bool CTreeItem::isFirstLevelSorted() const
{
    return mbFirstLevelSorted;
}

tTreeItemPtr CTreeItem::child(int row)
{
    if (row < 0 || row >= mChildrenVec.size())
        return nullptr;

    return mChildrenVec[row];
}


int CTreeItem::childCount() const
{
    return mChildItems.size();
}


int CTreeItem::row() const
{
    return getIdx();
}

const int& CTreeItem::getIdx() const
{
    return mIdx;
}

void CTreeItem::removeChild( const QString& key )
{
    auto foundChild = mChildItems.find(key);

    if(foundChild != mChildItems.end())
    {
        if(true == mbFirstLevelSorted)
        {
            mChildrenVec.erase(mChildrenVec.begin() + foundChild.value()->getIdx());

            auto counter = 0;

            for( auto& pChild : mChildrenVec )
            {
                if(nullptr != pChild)
                {
                    pChild->setIdx(counter++);
                }
            }
        }

        delete foundChild.value();
        mChildItems.erase(foundChild);
    }
}

CTreeItem::~CTreeItem()
{
    mChildrenVec.clear();
    qDeleteAll(mChildItems);
}


CTreeItem::CTreeItem(CTreeItem *pParent,
                     const int defaultSortingColumn,
                     const tSortingFunction& sortingFunction,
                     const tHandleDuplicateFunc& handleDuplicateFunc,
                     const tFindItemFunc& findFunc)
    :
      mChildItems(),
      mSortingFunction(sortingFunction),
      mFindFunc(findFunc),
      mHandleDuplicateFunc(handleDuplicateFunc),
      mData(),
      mChildrenVec(),
      mpGuard(std::make_shared<int>(0)),
      mpParentItem(pParent),
      mSortOrder(Qt::SortOrder::DescendingOrder),
      mIdx(sInvalidIdx),
      mSortingColumn(defaultSortingColumn),
      mbFirstLevelSorted(true),
      mbWholeSorted(true)
{}

tTreeItemPtr CTreeItem::appendChild(const tDataItem& key, const tData& additionItems)
{
    CTreeItem* pTreeItem = new CTreeItem(this,
                                         mSortingColumn,
                                         mSortingFunction,
                                         mHandleDuplicateFunc,
                                         mFindFunc);

    auto it = mChildItems.insert(key, pTreeItem);

    if(it.value()->getIdx() == sInvalidIdx)
    {
        mChildrenVec.push_back(pTreeItem);
        pTreeItem->setIdx(mChildItems.size());
    }

    mbFirstLevelSorted = false;
    mbWholeSorted = false;

    // copy data
    pTreeItem->mData = additionItems;

    return pTreeItem;
}

void CTreeItem::setData( const tData& data )
{
    mData = data;
}

bool CTreeItem::setColumnData( const tDataItem& dataItem, const int& column )
{
    bool bResult = false;

    if(column >= 0 && column <= mData.size())
    {
        mData[column] = dataItem;
        bResult = true;
    }

    return bResult;
}

void CTreeItem::appendColumn( const tDataItem &dataItem )
{
    mData.push_back(dataItem);
}

tDataItem& CTreeItem::getWriteableData( int column )
{
    Q_ASSERT(column >= 0);
    Q_ASSERT(column < mData.size());
    return mData[column];
}


void CTreeItem::addDataInternal( const tDataVec& dataVec,
                                 tTreeItemPtrVec& res,
                                 int dataVecItemIdx,
                                 tAfterAppendHandleLeafFunc afterAppendHandleLeafFunc )
{
    if(false == dataVec.empty() )
    {
        tTreeItemPtr pLeafChild = nullptr;
        const tData& additionItems = dataVec[static_cast<std::size_t>(dataVecItemIdx)];

        if(false == additionItems.empty())
        {
            auto findRes = mFindFunc(this, additionItems);

            if(true == findRes.bFound)
            {
                if(nullptr != findRes.pItem)
                {
                    auto foundChild = findRes.pItem;

                    pLeafChild = foundChild;

                    if(mHandleDuplicateFunc)
                    {
                        mHandleDuplicateFunc( foundChild, *dataVec.begin() );
                    }

                    auto newDataVecItemIdx = ++dataVecItemIdx;
                    if(static_cast<std::size_t>(newDataVecItemIdx) < dataVec.size() )
                    {
                        foundChild->addDataInternal( dataVec, res, newDataVecItemIdx, afterAppendHandleLeafFunc );
                    }
                }
            }
            else
            {
                auto pChild = appendChild(findRes.key, additionItems);

                if(pChild)
                {
                    pLeafChild = pChild;

                    auto newDataVecItemIdx = ++dataVecItemIdx;
                    if( static_cast<std::size_t>(newDataVecItemIdx) < dataVec.size() )
                    {
                        pChild->addDataInternal( dataVec, res, newDataVecItemIdx, afterAppendHandleLeafFunc );
                    }

                    res.push_back( pChild );
                }
            }

            mbFirstLevelSorted = false;
            mbWholeSorted = false;
        }

        if(nullptr != pLeafChild &&
           0 == pLeafChild->childCount() &&
           afterAppendHandleLeafFunc)
        {
            afterAppendHandleLeafFunc(pLeafChild);
        }
    }
}

tTreeItemPtrVec CTreeItem::addData( const tDataVec& dataVec,
                                    tAfterAppendHandleLeafFunc afterAppendHandleLeafFunc  )
{
    tTreeItemPtrVec result;
    result.reserve( static_cast<int>(dataVec.size()) );

    addDataInternal(dataVec, result, 0, afterAppendHandleLeafFunc);

    tTreeItemPtrVec reversedResult;
    reversedResult.reserve(result.size());

    for( auto it = result.end() - 1; it != result.begin() - 1; --it )
    {
        reversedResult.push_back(*it);
    }

    return reversedResult;
}

const tDataItem& CTreeItem::data(int column) const
{
    if(column >= 0 && column < mData.size())
    {
        return mData[column];
    }
    else
    {
        SEND_WRN(QString("[CTreeItem] Was not able to access data item row - %1, column - %2").arg(row()).arg(column));
        static const tDataItem sDummyResult;
        return sDummyResult;
    }
}

int CTreeItem::columnCount() const
{
    return mData.count();
}

const tDataItem& CTreeItem::getValue(const int& column) const
{
    if(column >= 0 && column < mData.size())
    {
        return mData[column];
    }
    else
    {
        static const tDataItem sDummyVal = -1;
        return sDummyVal;
    }
}

PUML_PACKAGE_BEGIN(DMA_Common)
    PUML_CLASS_BEGIN_CHECKED(CTreeItem)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTreeItem, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
