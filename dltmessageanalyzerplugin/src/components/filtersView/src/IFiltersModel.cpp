#include "../api/IFiltersModel.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IFiltersModel::IFiltersModel(QObject *parent):
QAbstractItemModel(parent)
{

}

IFiltersModel::~IFiltersModel()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(IFiltersModel)

PUML_PACKAGE_BEGIN(DMA_FiltersView_API)
    PUML_CLASS_BEGIN(IFiltersModel)
        PUML_INHERITANCE_CHECKED(QAbstractItemModel, implements)
        PUML_PURE_VIRTUAL_METHOD( +, void setUsedRegex(const QString& regexStr) )
        PUML_PURE_VIRTUAL_METHOD( +, void addCompletionData( const tFoundMatches& foundMatches ) )
        PUML_PURE_VIRTUAL_METHOD( +, void resetCompletionData() )
        PUML_PURE_VIRTUAL_METHOD( +, void resetData() )
        PUML_PURE_VIRTUAL_METHOD( +, void filterRegexTokens( const QString& filter ) )
    PUML_CLASS_END()
PUML_PACKAGE_END()
