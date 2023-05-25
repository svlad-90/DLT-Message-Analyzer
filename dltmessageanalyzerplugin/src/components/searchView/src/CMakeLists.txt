qt_wrap_cpp(PROCESSED_MOCS
             ../api/CSearchResultView.hpp
             ../api/CSearchViewComponent.hpp)

add_library(DMA_searchView STATIC
    CSearchResultHighlightingDelegate.cpp
    CSearchResultModel.cpp
    CSearchResultView.cpp
    CSearchViewComponent.cpp
    ISearchResultModel.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_searchView PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
