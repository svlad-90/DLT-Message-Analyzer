add_library(DMA_log STATIC
    CConsoleCtrl.cpp
    CConsoleView.cpp
    CConsoleInputProcessor.cpp
    CLog.cpp)

################### QT ####################################
target_link_libraries(DMA_log qdlt Qt5::Widgets )
################### QT ( END ) ############################
