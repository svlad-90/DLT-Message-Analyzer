add_library(DMA_common STATIC
    CTreeItem.cpp
    Definitions.cpp
    CBGColorAnimation.cpp
    CRegexDirectoryMonitor.cpp
    CTableMemoryJumper.cpp
    OSHelper.cpp
    CQtHelper.cpp)

add_subdirectory(PCRE)
add_subdirectory(DMA_Plantuml)

################### QT ####################################
target_link_libraries(DMA_common qdlt Qt5::Widgets )
################### QT ( END ) ############################
