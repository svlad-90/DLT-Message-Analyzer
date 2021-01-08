/**
 * @file    CPatternsModel.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CPatternsModel.hpp class
 */

#pragma once

#include <functional>

#include "QVector"
#include "QPair"
#include "QModelIndex"

#include "common/Definitions.hpp"

#include "../api/IPatternsModel.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CPatternsModel : public IPatternsModel,
                       public CSettingsManagerClient
{
    Q_OBJECT

public:

    CPatternsModel(const tSettingsManagerPtr& pSettingsManager,
                   QObject *parent=nullptr);
    ~CPatternsModel() override;

    // Implementation of the IPatternsModel
    void updateView() override;
    void resetData() override;
    QModelIndex addData(const QString& alias,
                        const QString& regex,
                        Qt::CheckState isDefault = Qt::Unchecked) override;
    QModelIndex addData(const QString& alias,
                        const QString& regex,
                        Qt::CheckState isCombine,
                        Qt::CheckState isDefault) override;
    void updatePatternsInPersistency() override;
    tSearchResult search( const QString& alias ) override;
    QModelIndex editData(const QModelIndex& idx,
                         const QString& alias,
                         const QString& regex,
                         Qt::CheckState isDefault,
                         Qt::CheckState isCombine) override;
    void removeData(const QModelIndex& idx) override;
    QString getAliasEditName( const QModelIndex& idx ) override;
    void filterPatterns( const QString& filter ) override;
    void refreshRegexPatterns() override;
    // Implementation of the IPatternsModel ( END )

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

    typedef QSet<QModelIndex> tChangedIndexes;

    tChangedIndexes setIsDefault( const QModelIndex& idx, Qt::CheckState checkState, bool updateFilter, bool updateView );
    tChangedIndexes setIsCombine( const QModelIndex& idx, Qt::CheckState checkState, bool updateFilter, bool updateView );

    /**
     * @brief getIndexCheckState - returns check state for given index
     * @param idx - index to be checked
     * @return - check state for provided index
     */
    Qt::CheckState getIndexCheckState( const QModelIndex& idx) const;

    QStringList getSearchKeys( const QString& alias ) const;

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

    void resetPatternsToDefault();
    void clearSelectedPatterns();

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
