#pragma once

#include "QModelIndex"

#include "common/Definitions.hpp"

class IGroupedViewModel
{
public:

    IGroupedViewModel();
    virtual ~IGroupedViewModel();

    virtual void setUsedRegex(const QString& regex) = 0;
    virtual void resetData() = 0;
    virtual void addMatches( const tFoundMatches& matches, bool update ) = 0;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual std::pair<bool /*result*/, QString /*error*/> exportToHTML(QString& resultHTML) = 0;
};
