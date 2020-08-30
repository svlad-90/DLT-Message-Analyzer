add_library(DMA_analyzer STATIC
    IDLTMessageAnalyzerController.cpp
    IDLTMessageAnalyzerControllerConsumer.cpp
    CMTAnalyzer.cpp
    CContinuousAnalyzer.cpp
    CDLTRegexAnalyzerWorker.cpp)

################### QT ####################################
target_link_libraries(DMA_analyzer qdlt Qt5::Widgets )
################### QT ( END ) ############################
