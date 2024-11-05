set(WRAP_SRC ../api/CPlotViewComponent.hpp
             ../api/CCustomPlotExtended.hpp
             QCPGantt.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_plotView STATIC
    CPlotViewComponent.cpp
    CCustomPlotExtended.cpp
    QCPGantt.cpp
    CScrollableLegend.cpp
    ${PROCESSED_MOCS})

find_package(${QT_PREFIX} REQUIRED COMPONENTS Svg)
target_compile_definitions(DMA_plotView PRIVATE QCUSTOMPLOT_USE_LIBRARY)

target_link_libraries( DMA_plotView PUBLIC
                       qdlt
                       qcustomplot
                       ${QT_PREFIX}::Svg
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
