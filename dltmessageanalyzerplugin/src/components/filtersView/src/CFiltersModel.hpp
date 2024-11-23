#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "common/CTreeItem.hpp"

#include "../api/IFiltersModel.hpp"
#include "components/settings/api/CSettingsManagerClient.hpp"

class CFiltersModel : public IFiltersModel,
                      public CSettingsManagerClient
{
    Q_OBJECT

public:
    explicit CFiltersModel(const tSettingsManagerPtr& pSettingsManager,
                           QObject *parent = nullptr);

    // Implementation of the IFiltersModel
    void setUsedRegex(const QString& regexStr) override;
    void addCompletionData( const tFoundMatches& foundMatches ) override;
    void resetCompletionData() override;
    void resetData() override;
    void filterRegexTokens( const QString& filter ) override;
    // Implementation of the IFiltersModel ( END )

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QStringList getCompletionData( const int& groupIndex, const QString& input, const int& maxNumberOfSuggestions, const int& maxLengthOfSuggestions );

private:

    QModelIndex createIndexInternal(int arow, int acolumn, void *adata) const;
    void filterRegexTokensInternal();

    void updateView();
    QModelIndex rootIndex() const;

    QPair<bool,QString> packRegex();
    void resetRootItem();

    void updateVarGroupsMap();

    CFiltersModel(const CFiltersModel&) = delete;
    CFiltersModel& operator=(const CFiltersModel&) = delete;
    CFiltersModel(const CFiltersModel&&) = delete;
    CFiltersModel& operator=(const CFiltersModel&&) = delete;

private:

    tTreeItemSharedPtr mpRootItem;
    QString mRegex;
    eRegexFiltersColumn mSortingColumn;
    Qt::SortOrder mSortOrder;
    CTreeItem::tSortingFunction mSortingHandler;
    QString mFilter;

    typedef std::set<QString> tStringSet;
    typedef std::map<int /*group id*/, tStringSet> tCompletionCache;
    tCompletionCache mCompletionCache;

    typedef std::set<int /*group id*/> tVarGroupsMap;
    tVarGroupsMap mVarGroupsMap;
};

Q_DECLARE_METATYPE(CFiltersModel::tFilteredEntryVec)
