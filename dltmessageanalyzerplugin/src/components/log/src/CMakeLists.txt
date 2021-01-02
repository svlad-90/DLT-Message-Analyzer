qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CConsoleView.hpp)

add_library(DMA_log STATIC
    CConsoleCtrl.cpp
    CConsoleView.cpp
    CConsoleInputProcessor.cpp
    CLog.cpp
    CLogComponent.cpp
    ${PROCESSED_MOCS})

target_link_libraries( DMA_log PUBLIC
                       qdlt
                       Qt5::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
