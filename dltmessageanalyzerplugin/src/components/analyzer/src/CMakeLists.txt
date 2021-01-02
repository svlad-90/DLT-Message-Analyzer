qt5_wrap_cpp(PROCESSED_MOCS
             ../api/IDLTMessageAnalyzerControllerConsumer.hpp
             ../api/IDLTMessageAnalyzerController.hpp)

add_library(DMA_analyzer STATIC
    IDLTMessageAnalyzerController.cpp
    IDLTMessageAnalyzerControllerConsumer.cpp
    CMTAnalyzer.cpp
    CContinuousAnalyzer.cpp
    CDLTRegexAnalyzerWorker.cpp
    CAnalyzerComponent.cpp
    ${PROCESSED_MOCS}
    )

target_link_libraries( DMA_analyzer PUBLIC
                       qdlt
                       Qt5::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
