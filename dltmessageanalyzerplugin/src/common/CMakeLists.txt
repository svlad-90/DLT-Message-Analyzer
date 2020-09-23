add_library(DMA_common STATIC
    CTreeItem.cpp
    Definitions.cpp
    CBGColorAnimation.cpp
    CRegexDirectoryMonitor.cpp
    CTableMemoryJumper.cpp
    OSHelper.cpp)

add_subdirectory(PCRE)

################### QT ####################################
target_link_libraries(DMA_common qdlt Qt5::Widgets )
################### QT ( END ) ############################
