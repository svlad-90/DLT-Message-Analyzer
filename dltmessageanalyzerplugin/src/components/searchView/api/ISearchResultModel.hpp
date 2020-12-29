#pragma once

#include "QModelIndex"

#include "common/Definitions.hpp"

class ISearchResultModel
{
public:

    ISearchResultModel();
    virtual ~ISearchResultModel();

    virtual void updateView(const int& fromRow = 0) = 0;
    virtual std::pair<int /*rowNumber*/, QString /*diagramContent*/> getUMLDiagramContent() const = 0;
    virtual void resetData() = 0;
    virtual void setFile(const tFileWrapperPtr& pFile) = 0;
    virtual std::pair<bool, tIntRange> addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack) = 0;
    virtual int getFileIdx( const QModelIndex& idx ) const = 0;
};
