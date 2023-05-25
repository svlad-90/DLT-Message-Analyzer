add_definitions(-DQT_NO_KEYWORDS)

DMA_SuppressClangTidy_START()
add_library(DMA_PCRE STATIC
    PCREHelper.cpp)
DMA_SuppressClangTidy_END()

################### QT ####################################
target_link_libraries(DMA_PCRE ${QT_PREFIX}::Widgets )
################### QT ( END ) ############################

################### ANTLR_LINKAGE #########################
# add generated grammar to demo binary target
target_link_libraries(DMA_PCRE antlr4_PCRE_static)
target_link_libraries(DMA_PCRE antlr4_static)
################### ANTLR_LINKAGE ( END ) #################
