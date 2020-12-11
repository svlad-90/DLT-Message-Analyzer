#include "../api/ISearchResultModel.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

ISearchResultModel::ISearchResultModel()
{

}

ISearchResultModel::~ISearchResultModel()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(ISearchResultModel)

PUML_PACKAGE_BEGIN(DMA_SearchView_API)
    PUML_CLASS_BEGIN(ISearchResultModel)
        PUML_PURE_VIRTUAL_METHOD( +, void updateView(const int& fromRow = 0) )
        PUML_PURE_VIRTUAL_METHOD( +,  std::pair<int__QString > getUMLDiagramContent() const )
        PUML_PURE_VIRTUAL_METHOD( +,  void resetData() )
        PUML_PURE_VIRTUAL_METHOD( +,  void setFile(const tDLTFileWrapperPtr& pFile) )
        PUML_PURE_VIRTUAL_METHOD( +,  std::pair<bool__tIntRange> addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack) )
        PUML_PURE_VIRTUAL_METHOD( +,  int getFileIdx( const QModelIndex& idx ) const )
    PUML_CLASS_END()
PUML_PACKAGE_END()
