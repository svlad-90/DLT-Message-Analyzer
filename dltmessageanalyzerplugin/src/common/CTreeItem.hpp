/**
 * @file    CGroupedViewItem.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CGroupedViewItem class
 */

#ifndef CTreeItem_HPP
#define CTreeItem_HPP

#include <vector>
#include <functional>
#include "QString"
#include "QVector"

#include "Definitions.hpp"
#include "../common/variant/variant.hpp"

class CTreeItem
{
public:
    typedef QVector<tDataItem> tData;

    typedef std::vector<tData> tDataVec;
    typedef std::function<void(CTreeItem* oldItem, const tData& newData)> tHandleDuplicateFunc;

    typedef QVector<tTreeItemPtr> tChildrenVector;
    typedef std::function< void(tChildrenVector&, const int& /*sortingColumn*/, Qt::SortOrder) > tSortingFunction;

    typedef std::function<void(CTreeItem*)> tAfterAppendHandleLeafFunc;

    /**
     * @brief tFindItemResult - result of search of the element.
     */
    struct tFindItemResult
    {
        bool bFound = false;    // tells whether element was found
        tTreeItemPtr pItem;       // found element, or nullptr, in case if element was not found
        tDataItem key;          // key, which can be used to store newly created sub-element.
                                // should be filled in ONLY if element was not found
    };

    /**
     * @brief tFindItemFunc - function, which is used by the tree in order to find elements by keys
     */
    typedef std::function< tFindItemResult(const tTreeItemPtr pItem, const tData& key) > tFindItemFunc;

    /**
     * @brief CTreeItem
     * @param pParent - parent of this ite
     * @param defaultSortingColumn - default sorting column which will be used for sorting
     * @param sortingFunction - sorting function, which inteprets data and sorts a collection
     * @param handleDuplicateFunc - function, which helps to handle addition of duplicated keys
     */
    explicit CTreeItem(CTreeItem *pParent,
                       const int defaultSortingColumn,
                       const tSortingFunction& sortingFunction,
                       const tHandleDuplicateFunc& handleDuplicateFunc,
                       const tFindItemFunc& findFunc);
    ~CTreeItem();

    typedef std::function< bool(tTreeItemPtr) > tVisitFunction;

    /**
     * @brief visit - visits each node of the tree.
     * Following sequence is applied:
     * - preVisitFunction for node
     * - visitInternal call of all children of the node
     * - postVisitFunction call for node
     * @param preVisitFunction - function, which is called before visiting all node's children
     * @param postVisitFunction - function, which is called after visiting all node's children
     * @param visitMe - whether root node should be visited or not
     * @param recursive - whether we should visit all nested children, or only first level children
     * @param visitSorted - whether we should visit sorted children or unsorted ones.
     * Note! If preVisitFunction or postVisitFunction functions are returning false - the whole visiting sequence will be interrupted.
     */
    void visit( const tVisitFunction& preVisitFunction,
                const tVisitFunction& postVisitFunction,
                bool visitMe = true,
                bool recursive = true,
                bool visitSorted = true );

    /**
     * @brief visitParents - visits all hierarchy of parents of this item
     * @param preVisitFunction - function, which is called before visiting all node's parents
     * @param postVisitFunction - function, which is called after visiting all node's parents
     * @param visitMe - whether we should visit this specific node
     * @param visitRoot - whether we should visit the root node of the tree
     * Note! If preVisitFunction or postVisitFunction functions are returning false - the whole visiting sequence will be interrupted.
     */
    void visitParents( const tVisitFunction& preVisitFunction,
                       const tVisitFunction& postVisitFunction,
                       bool visitMe = false,
                       bool visitRoot = true );

    typedef std::shared_ptr<int> tGuarded;
    typedef std::weak_ptr<int> tGuard;

    /**
     * @brief getGuard - gets guard, which allows to check whether this instance is still alive
     * @return  - instance of guard
     */
    tGuard getGuard() const;

    /**
     * @brief sort - sorts the sub-tree
     * @param column - by which column to sort
     * @param order - in which order to sort
     * @param recursive - whether sorting should be recursive, or should be applied only to first level children
     */
    void sort(int column, Qt::SortOrder order, bool recursive );

    typedef QMap<tDataItem, tTreeItemPtr> tChildrenMap;

    /**
     * @brief getChildren - gets const reference to choldren's collection
     * @return - const reference to choldren's collection
     */
    const tChildrenMap& getChildren() const;

    /**
     * @brief recursive search by vector of keys.
     * @param searchKeys - list of strings, which is used as a search key
     * @return - found item or nullptr if nothing was found
     */
    tTreeItemPtr search( const QStringList& searchKeys);

    /**
     * @brief isWholeSorted - shows, whether the whole sub-tree was already sorted
     * @return - returns true, if while sub-tree was already sorted
     */
    bool isWholeSorted() const;

    /**
     * @brief isFirstLevelSorted - shows, whether first level children was already sorted
     * @return - returns true, if first-level children of this node were already sorted
     */
    bool isFirstLevelSorted() const;

    /**
     * @brief getParent - get's parent of this tree node
     * @return - the parent of this tree item, or a null ptr if it is a root node.
     */
    tTreeItemPtr getParent() const;

    /**
     * @brief child - get's first level child by row
     * @param row - idx of the first level child, which should be returned
     * @return - first level child by the specified idx
     */
    tTreeItemPtr child(int row);

    /**
     * @brief childCount
     * @return - number of first level children
     */
    int childCount() const;

    /**
     * @brief row - get's child idx of this row. Actually, it does the same, as the getIdx method
     * @return - id of row, to which this item is related on some level of the tree
     */
    int row() const;

    /**
     * @brief getIdx - gets index of element in parent's collection
     * @return - index
     */
    const int& getIdx() const;

    /**
     * @brief removeChild - removes first level child, if it exists
     * @param key - key to the child, which should be removed
     */
    void removeChild( const QString& key );

    /**
     * @brief appendChild - appends child to this node of the tree
     * @param key - key, which identifies the child
     * @param additionItems - data of the child
     * @return - added child
     */
    tTreeItemPtr appendChild(const tDataItem& key, const tData& additionItems);

    /**
     * @brief getValue - get's value by a specified column
     * @param column - column, for which the data was requested
     * @return - const refeerence to the data variant
     */
    const tDataItem& getValue(const int& column) const;

    /**
     * @brief appendColumn - appends data item to the vector of variants, which this element stores
     * @param dataItem - item, which should be added to the element's data container
     */
    void appendColumn( const tDataItem &dataItem );

    /**
     * @brief getWriteableData - does the same as getValue, vut gets a WRITABLE data.
     * @param column - column, for which the data was requested
     * @return - refeerence to the data variant
     */
    tDataItem& getWriteableData( int column );

    /**
     * @brief addData - adds data to the tree
     * @param dataVec - data to ba added
     * @param afterAppendHandleLeafFunc - optional functionl object. It will be called
     * with the 'leaf' tree item of the append operation when it is over
     * @return - returns tTreeItemPtrVec, which represents all added elements.
     * Note! Elements are sorted from most bottom-level to most top-level
     */
    tTreeItemPtrVec addData( const tDataVec& dataVec,
                             tAfterAppendHandleLeafFunc afterAppendHandleLeafFunc = tAfterAppendHandleLeafFunc() );

    /**
     * @brief data - get's data of this node by specified column
     * @param column - idx of the column, for which data is requested
     * @return - const reference to data variant
     */
    const tDataItem& data(int column) const;

    /**
     * @brief columnCount
     * @return - available number of the columns
     */
    int columnCount() const;

    /**
     * @brief setData - updates the data, which is stored inside a tree node
     * @param data - data to be stored
     */
    void setData( const tData& data );

    /**
     * @brief setData - updates the column data, which is stored inside a tree node
     * @param dataItem - data to be stored
     * @param column - column to be updated
     * @return - true, if update was successful
     */
    bool setColumnData( const tDataItem& dataItem, const int& column );

private:

    void addDataInternal( const tDataVec& dataVec,
                          tTreeItemPtrVec& res,
                          int dataVecItemIdx,
                          tAfterAppendHandleLeafFunc afterAppendHandleLeafFunc );

    CTreeItem(const CTreeItem&) = delete;
    CTreeItem& operator=(const CTreeItem&) = delete;
    CTreeItem(const CTreeItem&&) = delete;
    CTreeItem& operator=(const CTreeItem&&) = delete;

    bool visitInternal( const tVisitFunction& preVisitFunction,
                        const tVisitFunction& postVisitFunction,
                        bool visitMe,
                        bool recursive,
                        bool visitSorted,
                        tTreeItemPtr pTraversingRoot);

    void setIdx(const int& val);

private:

    tChildrenMap mChildItems;
    tSortingFunction mSortingFunction;
    tFindItemFunc mFindFunc;
    tHandleDuplicateFunc mHandleDuplicateFunc;
    tData mData;
    QVector<tTreeItemPtr> mChildrenVec;
    tGuarded mpGuard;
    tTreeItemPtr mpParentItem;
    Qt::SortOrder mSortOrder;
    int mIdx; // idx in parent's collection
    int mSortingColumn;
    bool mbFirstLevelSorted;
    bool mbWholeSorted;
};

#endif // CTreeItem_HPP
