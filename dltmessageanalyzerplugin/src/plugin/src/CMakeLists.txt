set(WRAP_SRC ../api/form.h
             ../api/CDLTMessageAnalyzer.hpp)
DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

set(WRAP_UI ${CMAKE_CURRENT_SOURCE_DIR}/form.ui)
DMA_QT_WRAP_UI(UI_HEADERS WRAP_UI)

add_library(DMA_plugin STATIC
    CDLTMessageAnalyzer.cpp
    form.cpp
    ${UI_HEADERS}
    ${PROCESSED_MOCS})

target_compile_definitions(DMA_plugin PRIVATE QCUSTOMPLOT_USE_LIBRARY)

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
                      qcustomplot
                      DMA_plotView
                      DMA_PCRE
                      DMA_regexHistory
                      DMA_coverageNote
                      qdlt
                      ${QT_PREFIX}::Widgets 
                      DMA_framework_base 
                      DMA_framework_component
                      DMA_Plantuml)
