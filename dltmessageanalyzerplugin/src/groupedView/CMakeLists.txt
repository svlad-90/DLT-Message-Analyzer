add_library(DMA_groupedView STATIC
    CGroupedView.cpp
    CGroupedViewModel.cpp)

################### QT ####################################
target_link_libraries(DMA_groupedView qdlt Qt5::Widgets )
################### QT ( END ) ############################
