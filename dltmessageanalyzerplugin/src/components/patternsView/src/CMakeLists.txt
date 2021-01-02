qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CPatternsView.hpp)

add_library(DMA_patternsView STATIC
    CPatternsModel.cpp
    CPatternsView.cpp
    CPatternsViewComponent.cpp
    IPatternsModel.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_patternsView PUBLIC
                       qdlt
                       Qt5::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
