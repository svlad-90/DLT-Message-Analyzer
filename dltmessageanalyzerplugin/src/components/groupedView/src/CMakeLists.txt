qt_wrap_cpp(PROCESSED_MOCS
             ../api/CGroupedView.hpp)

add_library(DMA_groupedView STATIC
    CGroupedView.cpp
    CGroupedViewModel.cpp
    CGroupedViewComponent.cpp
    IGroupedViewModel.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_groupedView PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
