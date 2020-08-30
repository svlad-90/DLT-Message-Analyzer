add_library(DMA_filtersView STATIC
    CFiltersModel.cpp
    CFiltersView.cpp)

################### QT ####################################
target_link_libraries(DMA_filtersView qdlt Qt5::Widgets )
################### QT ( END ) ############################
