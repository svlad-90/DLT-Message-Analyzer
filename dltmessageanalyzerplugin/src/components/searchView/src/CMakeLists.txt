set(WRAP_SRC ../api/CSearchResultView.hpp
             ../api/CSearchViewComponent.hpp
             ../api/ISearchResultModel.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

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
