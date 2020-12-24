#pragma once

#include "QModelIndex"
#include "QAbstractItemModel"

#include "common/Definitions.hpp"

class IFiltersModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit IFiltersModel(QObject *parent = nullptr);
    virtual ~IFiltersModel();

    virtual void setUsedRegex(const QString& regexStr) = 0;
    virtual void addCompletionData( const tFoundMatches& foundMatches ) = 0;
    virtual void resetCompletionData() = 0;
    virtual void resetData() = 0;
    virtual void filterRegexTokens( const QString& filter ) = 0;

    struct tFilteredEntry
    {
        QModelIndex parentIdx;
        int row;
        bool filtered;
    };
    typedef std::vector<tFilteredEntry> tFilteredEntryVec;

signals:

    void filteredEntriesChanged(const tFilteredEntryVec& filteredEntryVec, bool expandVisible);
    void regexUpdatedByUser( const QString& regex );
    void regexUpdatedByUserInvalid( const QModelIndex& index, const QString& error );
};
