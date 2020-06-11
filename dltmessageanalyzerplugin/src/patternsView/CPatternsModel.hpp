/**
 * @file    CPatternsModel.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CPatternsModel.hpp class
 */

#ifndef CPatternsModel_HPP
#define CPatternsModel_HPP

#include <functional>

#include "QVector"
#include "QPair"
#include "QModelIndex"

#include "../common/Definitions.hpp"

class CPatternsModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    CPatternsModel(QObject *parent=nullptr);
    ~CPatternsModel() override;

    void updateView();
    void beginResetModel_();
    void endResetModel_();
    void resetData();

    bool areAnyCombinedPatternsAvailable() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    void sort(int column, Qt::SortOrder order) override;

    QModelIndex addData(const QString& alias, const QString& regex, Qt::CheckState isDefault = Qt::Unchecked);
    QModelIndex addData(const QString& alias, const QString& regex, Qt::CheckState isCombine, Qt::CheckState isDefault);

    typedef QSet<QModelIndex> tChangedIndexes;

    tChangedIndexes setIsDefault( const QModelIndex& idx, Qt::CheckState checkState, bool updateFilter, bool updateView );
    tChangedIndexes setIsCombine( const QModelIndex& idx, Qt::CheckState checkState, bool updateFilter, bool updateView );

    void filterPatterns( const QString& filter );

    /**
     * @brief getIndexCheckState - returns check state for given index
     * @param idx - index to be checked
     * @return - check state for provided index
     */
    Qt::CheckState getIndexCheckState( const QModelIndex& idx) const;

    /**
     * @brief editData - edits pattern
     * @param idx - index of item to be edited
     * @param alias - new alias value
     * @param regex - new regex value
     * @param isDefault - new default value
     * @param isCombine - new combine value
     * @return - index of new item, or index of input "idx" parameter. Depends on type of change.
     * Anyway, client should avoid usage of input "idx" index, and stick to returned index instance after this call.
     */
    QModelIndex editData(const QModelIndex& idx, const QString& alias, const QString& regex, Qt::CheckState isDefault, Qt::CheckState isCombine);

    void removeData(const QModelIndex& idx);

    QStringList getSearchKeys( const QString& alias ) const;

    struct tSearchResult
    {
        bool bFound { false };
        QModelIndex foundIdx;
    };

    /**
     * @brief search - searches pattern by irs alias
     * @param alias - alias to be searched
     * @return - instance of tSearchResult, which provides result status of the search
     */
    tSearchResult search( const QString& alias );

    typedef std::function<bool(const QModelIndex&)> tVisitFunction;

    /**
     * @brief visit - visits each index of the model.
     * Following sequence is applied:
     * - preVisitFunction for node
     * - visit call of all children of the node
     * - postVisitFunction call for node
     * @param preVisitFunction - function, which is called before visiting all node's children
     * @param postVisitFunction - function, which is called after visiting all node's children
     * @param visitParentItem - tells whether we should visit parent or root item.
     * @param parentIdx - tells from which index to traverse the tree. In case of invalid idx - will start to traverse it from the root element.
     * Note! If preVisitFunction or postVisitFunction functions are returning false - the whole visiting sequence will be interrupted.
     */
    void visit( const tVisitFunction& preVisitFunction,
                const tVisitFunction& postVisitFunction,
                bool visitParentItem = false,
                QModelIndex parentIdx = QModelIndex()) const;

    void updatePatternsInPersistency();
    void resetPatternsToDefault();
    void clearSelectedPatterns();

    /**
     * @brief getAliasEditName - gets update name for given pattern
     * @param idx - input model index
     * @return - name to be used for edit purposes
     */
    QString getAliasEditName( const QModelIndex& idx );

    struct tFilteredEntry
    {
        QModelIndex parentIdx;
        int row;
        bool filtered;
    };
    typedef QVector<tFilteredEntry> tFilteredEntryVec;

signals:
    void filteredEntriesChanged(const tFilteredEntryVec& filteredEntryVec, bool expandVisible);

private:
    QModelIndex createIndexInternal(int arow, int acolumn, void *adata) const;
    void updateSubTree(const QModelIndex& idx);
    void filterPatternsInternal(bool ignoreEmptyFilter);

private:
    tTreeItemPtr mpRootItem;
    ePatternsColumn mSortingColumn;
    Qt::SortOrder mSortOrder;
    QString mFilter;
};

#endif // CPatternsModel_HPP
