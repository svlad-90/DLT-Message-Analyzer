qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CGroupedView.hpp
             ../api/CGroupedViewComponent.hpp)

add_library(DMA_groupedView STATIC
    CGroupedView.cpp
    CGroupedViewModel.cpp
    CGroupedViewComponent.cpp
    IGroupedViewModel.cpp
    ${PROCESSED_MOCS})

################### QT ####################################
target_link_libraries(DMA_groupedView PUBLIC qdlt Qt5::Widgets DMA_framework_base DMA_framework_component )
################### QT ( END ) ############################
