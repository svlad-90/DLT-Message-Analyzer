set(WRAP_SRC ../api/CLogo.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_logo STATIC
    CLogo.cpp
    CLogoComponent.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_logo PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
