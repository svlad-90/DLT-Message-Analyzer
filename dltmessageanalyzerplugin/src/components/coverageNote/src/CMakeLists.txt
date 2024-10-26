set(WRAP_SRC ../api/ICoverageNoteProvider.hpp
    CCoverageNoteProvider.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_coverageNote STATIC
    CCoverageNoteComponent.cpp
    CCoverageNoteProvider.cpp
    Types.cpp
    ICoverageNoteProvider.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_coverageNote PUBLIC
                       ${QT_PREFIX}::Widgets
                       DMA_settings
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
