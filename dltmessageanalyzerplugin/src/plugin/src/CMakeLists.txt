QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )

qt5_wrap_cpp(PROCESSED_MOCS
             ../api/form.h
             ../api/CDLTMessageAnalyzer.hpp)

add_library(DMA_plugin STATIC
    CDLTMessageAnalyzer.cpp
    form.cpp
    ${UI_HEADERS}
    ${PROCESSED_MOCS})

target_link_libraries( DMA_plugin PUBLIC
                      DMA_common
                      DMA_analyzer
                      DMA_searchView
                      DMA_plant_uml
                      DMA_log
                      DMA_settings
                      DMA_logo
                      DMA_logsWrapper
                      DMA_filtersView
                      DMA_filtersView_no_clang_tidy
                      DMA_groupedView
                      DMA_patternsView
                      DMA_PCRE
                      qdlt
                      Qt5::Widgets 
                      DMA_framework_base 
                      DMA_framework_component
                      DMA_Plantuml )
