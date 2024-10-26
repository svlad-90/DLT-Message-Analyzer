#include "../api/ICoverageNoteProvider.hpp"

#include "dma/base/ForceLink.hpp"

#include "DMA_Plantuml.hpp"

ICoverageNoteProvider::ICoverageNoteProvider(QObject *parent): QObject(parent)
{}

ICoverageNoteProvider::~ICoverageNoteProvider()
{}

DMA_FORCE_LINK_ANCHOR_CPP(ICoverageNoteProvider)

PUML_PACKAGE_BEGIN(DMA_CoverageNote_API)
    PUML_CLASS_BEGIN(ICoverageNoteProvider)
        PUML_INHERITANCE_CHECKED(QObject, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
