qt5_wrap_cpp(PROCESSED_MOCS
             ../api/CUMLView.hpp)

add_library(DMA_plant_uml STATIC
#   CSVGView.cpp
    CUMLView.cpp
    CImageViewer.cpp
    CUMLViewComponent.cpp
    ${PROCESSED_MOCS})

################### QT ####################################
target_link_libraries(DMA_plant_uml PUBLIC qdlt Qt5::Widgets DMA_framework_base DMA_framework_component )
################### QT ( END ) ############################
