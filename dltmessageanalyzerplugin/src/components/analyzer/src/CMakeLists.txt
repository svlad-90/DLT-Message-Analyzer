set(WRAP_SRC ../api/IDLTMessageAnalyzerControllerConsumer.hpp
             ../api/IDLTMessageAnalyzerController.hpp)

DMA_qt_wrap_cpp(PROCESSED_MOCS WRAP_SRC)

add_library(DMA_analyzer STATIC
    IDLTMessageAnalyzerController.cpp
    IDLTMessageAnalyzerControllerConsumer.cpp
    CMTAnalyzer.cpp
    CContinuousAnalyzer.cpp
    CDLTRegexAnalyzerWorker.cpp
    CAnalyzerComponent.cpp
    Definitions.cpp
    DefinitionsInternal.cpp
    ${PROCESSED_MOCS}
    )

target_link_libraries( DMA_analyzer PUBLIC
                       qdlt
                       ${QT_PREFIX}::Widgets
                       DMA_framework_base
                       DMA_framework_component
                       DMA_Plantuml )
