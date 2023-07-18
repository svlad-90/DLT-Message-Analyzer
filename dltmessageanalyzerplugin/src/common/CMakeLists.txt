add_library(DMA_common STATIC
    BaseDefinitions.cpp
    CTreeItem.cpp
    Definitions.cpp
    PlotDefinitions.cpp
    CBGColorAnimation.cpp
    CRegexDirectoryMonitor.cpp
    CTableMemoryJumper.cpp
    OSHelper.cpp)

add_subdirectory(PCRE)

################### QT ####################################
target_link_libraries(DMA_common qdlt ${QT_PREFIX}::Widgets )
################### QT ( END ) ############################
