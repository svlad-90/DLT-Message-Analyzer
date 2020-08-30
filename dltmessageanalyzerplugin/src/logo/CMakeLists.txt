add_library(DMA_logo STATIC
    CLogo.cpp)

################### QT ####################################
target_link_libraries(DMA_logo qdlt Qt5::Widgets )
################### QT ( END ) ############################
