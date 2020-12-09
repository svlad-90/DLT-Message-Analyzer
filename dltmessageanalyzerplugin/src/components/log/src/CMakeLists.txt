qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CConsoleView.hpp)

add_library(DMA_log STATIC
    CConsoleCtrl.cpp
    CConsoleView.cpp
    CConsoleInputProcessor.cpp
    CLog.cpp
    CLogComponent.cpp
    ${PROCESSED_MOCS})

################### QT ####################################
target_link_libraries(DMA_log qdlt Qt5::Widgets )
################### QT ( END ) ############################
