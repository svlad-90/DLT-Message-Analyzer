add_library(DMA_patternsView STATIC
    CPatternsModel.cpp
    CPatternsView.cpp)

################### QT ####################################
target_link_libraries(DMA_patternsView qdlt Qt5::Widgets )
################### QT ( END ) ############################
