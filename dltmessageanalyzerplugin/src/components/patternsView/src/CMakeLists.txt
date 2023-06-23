set(WRAP_SRC ../api/IPatternsModel.hpp
             ../api/CPatternsView.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_patternsView STATIC
    CPatternsModel.cpp
    CPatternsView.cpp
    CPatternsViewComponent.cpp
    IPatternsModel.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_patternsView PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
