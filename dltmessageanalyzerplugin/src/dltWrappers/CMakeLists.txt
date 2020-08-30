add_library(DMA_dltWrappers STATIC
    CDLTFileWrapper.cpp
    CDLTMsgWrapper.cpp)

################### QT ####################################
target_link_libraries(DMA_dltWrappers qdlt Qt5::Widgets )
################### QT ( END ) ############################
