#pragma once

#include "QModelIndex"

#include "common/Definitions.hpp"

class IGroupedViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    IGroupedViewModel(QObject* pParent = nullptr);
    virtual ~IGroupedViewModel();

    virtual void setUsedRegex(const QString& regex) = 0;
    virtual void resetData() = 0;
    virtual void addMatches( const tGroupedViewIndices& groupedViewIndices,
                             const tFoundMatches& matches,
                             bool update ) = 0;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual std::pair<bool /*result*/, QString /*error*/> exportToHTML(QString& resultHTML) = 0;
    virtual tMsgIdSet getAllMessageIds(const QModelIndex& index) = 0;
    virtual void sortByCurrentSortingColumn() = 0;
};
