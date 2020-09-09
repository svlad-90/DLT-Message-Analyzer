add_library(DMA_filtersView STATIC
    CFiltersModel.cpp
    CFiltersView.cpp)

set(RESTORE_CLANG_TIDY OFF)
set(CLANG_TIDY_CACHED_VAL "")

if( DMA_CLANG_TIDY_BUILD )
    set(RESTORE_CLANG_TIDY ON)
    set(CLANG_TIDY_CACHED_VAL CMAKE_CXX_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "")
endif()

add_library(DMA_filtersView_no_clang_tidy STATIC
    CFilterItemDelegate.cpp)

if( RESTORE_CLANG_TIDY )
    set(CMAKE_CXX_CLANG_TIDY CLANG_TIDY_CACHED_VAL)
    set(CLANG_TIDY_CACHED_VAL "")
    set(RESTORE_CLANG_TIDY OFF)
endif()

################### QT ####################################
target_link_libraries(DMA_filtersView_no_clang_tidy qdlt Qt5::Widgets )
################### QT ( END ) ############################

################### QT ####################################
target_link_libraries(DMA_filtersView qdlt Qt5::Widgets )
################### QT ( END ) ############################
