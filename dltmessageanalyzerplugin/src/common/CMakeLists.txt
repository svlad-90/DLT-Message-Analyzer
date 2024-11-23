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
target_link_libraries(DMA_common PUBLIC qdlt ${QT_PREFIX}::Widgets )
################### QT ( END ) ############################

if(DMA_TC_MALLOC_OPTIMIZATION OR DMA_TC_MALLOC_PROFILING)
    target_link_libraries(DMA_common PRIVATE tcmalloc)
endif()
