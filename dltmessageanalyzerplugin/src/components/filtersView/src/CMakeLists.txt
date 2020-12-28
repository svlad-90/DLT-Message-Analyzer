qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CFiltersView.hpp
             ../api/IFiltersModel.hpp)

add_library(DMA_filtersView STATIC
    CFiltersModel.cpp
    CFiltersView.cpp
    CFiltersViewComponent.cpp
    IFiltersModel.cpp
    ${PROCESSED_MOCS})

target_link_libraries(DMA_filtersView PUBLIC qdlt Qt5::Widgets DMA_framework_base DMA_framework_component )

DMA_SuppressClangTidy_START()
add_library(DMA_filtersView_no_clang_tidy STATIC
    CFilterItemDelegate.cpp)
DMA_SuppressClangTidy_END()

target_link_libraries(DMA_filtersView_no_clang_tidy PUBLIC qdlt Qt5::Widgets DMA_framework_base DMA_framework_component )
