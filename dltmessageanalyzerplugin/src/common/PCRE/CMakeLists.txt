add_definitions(-DQT_NO_KEYWORDS)

set(RESTORE_CLANG_TIDY OFF)
set(CLANG_TIDY_CACHED_VAL "")

if( DMA_CLANG_TIDY_BUILD )
    set(RESTORE_CLANG_TIDY ON)
    set(CLANG_TIDY_CACHED_VAL CMAKE_CXX_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "")
endif()

add_library(DMA_PCRE STATIC
    PCREHelper.cpp)

if( RESTORE_CLANG_TIDY )
    set(CMAKE_CXX_CLANG_TIDY CLANG_TIDY_CACHED_VAL)
    set(CLANG_TIDY_CACHED_VAL "")
    set(RESTORE_CLANG_TIDY OFF)
endif()

################### QT ####################################
target_link_libraries(DMA_PCRE Qt5::Widgets )
################### QT ( END ) ############################

################### ANTLR_LINKAGE #########################
# add generated grammar to demo binary target
target_link_libraries(DMA_PCRE antlr4_PCRE_static)
target_link_libraries(DMA_PCRE antlr4_static)
################### ANTLR_LINKAGE ( END ) #################
