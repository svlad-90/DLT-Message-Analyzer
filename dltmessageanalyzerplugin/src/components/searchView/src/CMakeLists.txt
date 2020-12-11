qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CSearchResultView.hpp
             ../api/CSearchViewComponent.hpp)

add_library(DMA_searchView STATIC
    CSearchResultHighlightingDelegate.cpp
    CSearchResultModel.cpp
    CSearchResultView.cpp
    CSearchViewComponent.cpp
    ISearchResultModel.cpp
    ${PROCESSED_MOCS})

################### QT ####################################
target_link_libraries(DMA_searchView qdlt Qt5::Widgets )
################### QT ( END ) ############################
