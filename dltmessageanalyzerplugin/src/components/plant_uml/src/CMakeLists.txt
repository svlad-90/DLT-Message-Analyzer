set(WRAP_SRC ../api/CUMLView.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_plant_uml STATIC
#   CSVGView.cpp
    CUMLView.cpp
    CImageViewer.cpp
    CUMLViewComponent.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_plant_uml PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
