#include "../api/IGroupedViewModel.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

IGroupedViewModel::IGroupedViewModel(QObject* pParent):
    QAbstractItemModel(pParent)
{

}

IGroupedViewModel::~IGroupedViewModel()
{

}

DMA_FORCE_LINK_ANCHOR_CPP(IGroupedViewModel)

PUML_PACKAGE_BEGIN(DMA_GroupedView_API)
    PUML_CLASS_BEGIN(IGroupedViewModel)
        PUML_PURE_VIRTUAL_METHOD(+, void setUsedRegex(const QString& regex))
        PUML_PURE_VIRTUAL_METHOD(+, void resetData())
        PUML_PURE_VIRTUAL_METHOD(+, void addMatches( const tFoundMatches& matches, bool update ))
        PUML_PURE_VIRTUAL_METHOD(+, int rowCount(const QModelIndex &parent = QModelIndex()) const)
        PUML_PURE_VIRTUAL_METHOD(+, std::pair<bool__QString> exportToHTML(QString& resultHTML))
    PUML_CLASS_END()
PUML_PACKAGE_END()
