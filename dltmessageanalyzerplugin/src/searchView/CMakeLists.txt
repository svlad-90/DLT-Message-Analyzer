add_library(DMA_searchView STATIC
    CSearchResultHighlightingDelegate.cpp
    CSearchResultModel.cpp
    CSearchResultView.cpp)

################### QT ####################################
target_link_libraries(DMA_searchView qdlt Qt5::Widgets )
################### QT ( END ) ############################
