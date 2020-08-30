add_library(DMA_plant_uml STATIC
#   CSVGView.hpp
    CUMLView.cpp
    CImageViewer.cpp)

################### QT ####################################
target_link_libraries(DMA_plant_uml qdlt Qt5::Widgets )
################### QT ( END ) ############################
