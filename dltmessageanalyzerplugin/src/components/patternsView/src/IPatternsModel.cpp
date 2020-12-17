#include "../api/IPatternsModel.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IPatternsModel::IPatternsModel()
{

}

IPatternsModel::~IPatternsModel()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(IPatternsModel)

PUML_PACKAGE_BEGIN(DMA_PatternsView_API)
    PUML_CLASS_BEGIN(IPatternsModel)
        PUML_PURE_VIRTUAL_METHOD( +,  void updateView() )
        PUML_PURE_VIRTUAL_METHOD( +, void resetData() )
        PUML_PURE_VIRTUAL_METHOD( +, QModelIndex addData(const QString& alias,
                                                         const QString& regex,
                                                         Qt::CheckState isDefault = Qt::Unchecked) )
        PUML_PURE_VIRTUAL_METHOD( +, QModelIndex addData(const QString& alias,
                                                         const QString& regex,
                                                         Qt::CheckState isCombine,
                                                         Qt::CheckState isDefault) )
        PUML_PURE_VIRTUAL_METHOD( +, void updatePatternsInPersistency() )
        PUML_PURE_VIRTUAL_METHOD( +, tSearchResult search( const QString& alias ) )
        PUML_PURE_VIRTUAL_METHOD( +, QModelIndex editData(const QModelIndex& idx,
                             const QString& alias,
                             const QString& regex,
                             Qt::CheckState isDefault, Qt::CheckState isCombine) )
        PUML_PURE_VIRTUAL_METHOD( +, removeData(const QModelIndex& idx) )
        PUML_PURE_VIRTUAL_METHOD( +, QString getAliasEditName( const QModelIndex& idx ) )
        PUML_PURE_VIRTUAL_METHOD( +, void filterPatterns( const QString& filter ) )
    PUML_CLASS_END()
PUML_PACKAGE_END()
