#ifndef CFILTERSMODEL_HPP
#define CFILTERSMODEL_HPP

#include "memory"

#include "QAbstractItemModel"

#include "../common/Definitions.hpp"
#include "../common/CTreeItem.hpp"

class CFiltersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CFiltersModel(QObject *parent = nullptr);

    void resetData();
    void setUsedRegex(const QString& regexStr);

    void addCompletionData( const tFoundMatches& foundMatches );
    void resetCompletionData();

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

    void filterRegexTokens( const QString& filter );

    struct tFilteredEntry
    {
        QModelIndex parentIdx;
        int row;
        bool filtered;
    };
    typedef std::vector<tFilteredEntry> tFilteredEntryVec;

    QStringList getCompletionData( const int& groupIndex, const QString& input, const int& maxNumberOfSuggestions, const int& maxLengthOfSuggestions );

signals:
    void filteredEntriesChanged(const tFilteredEntryVec& filteredEntryVec, bool expandVisible);
    void regexUpdatedByUser( const QString& regex );
    void regexUpdatedByUserInvalid( const QModelIndex& index, const QString& error );

private:

    QModelIndex createIndexInternal(int arow, int acolumn, void *adata) const;
    void filterRegexTokensInternal();

    void updateView();
    QModelIndex rootIndex() const;

    QPair<bool,QString> packRegex();
    void resetRootItem();

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

    typedef std::set<tQStringPtrWrapper> tStringPtrWrapperSet;
    typedef std::map<int /*group id*/, tStringPtrWrapperSet> tCompletionCache;
    tCompletionCache mCompletionCache;
};

Q_DECLARE_METATYPE(CFiltersModel::tFilteredEntryVec)

#endif // CFILTERSMODEL_HPP
