################### CPP ###################################
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
################### CPP ( END ) ###########################

################### COMMON_DEPS ###########################
include(FetchContent)
FetchContent_Declare(
  dma_common_deps
  GIT_REPOSITORY https://github.com/svlad-90/DMA_CommonDeps.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(dma_common_deps)
include(${FETCHCONTENT_BASE_DIR}/dma_common_deps-src/CMakeLists_Helper.txt)
################### COMMON_DEPS ( END ) ###################

################### POSITION INDEPENDENT CODE #############
# as we are building the shared library, all the involved code
# should be compiled with the -fPIC flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
################### POSITION INDEPENDENT CODE ( END ) #####

################### DEFINES ###############################
# enables generation of diagrams within DMA_Framework
add_definitions(-DPUML_ENABLED)
# enables force link which is used by DMA_Framework in diagrams generation
add_definitions(-DDMA_FORCE_LINK_ON)
################### DEFINES ( END ) #######################

################### DEPENDENCIES ##########################
DMA_sync_g_test_framework()
DMA_sync_plantuml()
DMA_sync_framework()
################### DEPENDENCIES ( END )###################

################### COMPATIBILITY #########################
# Turn this on if you want build, compatible with the PLUGIN_INTERFACE_VERSION "1.0.0"
# add_definitions(-DPLUGIN_API_COMPATIBILITY_MODE_1_0_0)
################### COMPATIBILITY ( END ) #################

################### ANTLR #################################
# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

cmake_policy(SET CMP0071 NEW)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/antlr/antlr-4.8-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(PCRE ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/pcre-parser/src/main/antlr4/nl/bigo/pcreparser/PCRE.g4
             LEXER PARSER LISTENER VISITOR
             PACKAGE pcre_parser)

# include generated files in project environment
include_directories(${ANTLR_PCRE_OUTPUT_DIR})

# We build PCRE as a separate static lib, which will be then linked to the shared lib of the plugin.
# That is done to avoid check of this lib with clang-tidy
add_library(antlr4_PCRE_static STATIC
    ${ANTLR_PCRE_CXX_OUTPUTS}
)

set_property(TARGET antlr4_PCRE_static PROPERTY POSITION_INDEPENDENT_CODE ON)

TARGET_LINK_LIBRARIES(antlr4_PCRE_static antlr4_static)
################### ANTLR ( END ) #########################

################### DLT_MESSAGE_ANALYZER ##################
QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

################### PROJECT ###############################
enable_testing()
DMA_Prepare()
################### PROJECT ( END ) #######################

set(TURN_OFF_RCC OFF)

if(NOT CMAKE_AUTORCC)
    set(CMAKE_AUTORCC ON)
    set(TURN_OFF_RCC ON)
endif()

include_directories(../../thirdparty/DMA_Plantuml/src)

DMA_add_subdirectory_with_clang_tidy(common)
DMA_add_subdirectory_with_clang_tidy(components)
DMA_add_subdirectory_with_clang_tidy(plant_uml)
DMA_add_subdirectory_with_clang_tidy(settings)
DMA_add_subdirectory_with_clang_tidy(logo)
DMA_add_subdirectory_with_clang_tidy(dltWrappers)
DMA_add_subdirectory_with_clang_tidy(filtersView)

add_library(DLT-Message-Analyzer MODULE
    dltmessageanalyzerplugin.cpp
    CDLTMessageAnalyzer.cpp
    form.cpp
    ${UI_HEADERS}
    resources/dltmessageanalyzer.qrc)

target_link_libraries(DLT-Message-Analyzer
    DMA_analyzer
    DMA_common
    DMA_searchView
    DMA_plant_uml
    DMA_log
    DMA_settings
    DMA_logo
    DMA_dltWrappers
    DMA_filtersView
    DMA_filtersView_no_clang_tidy
    DMA_groupedView
    DMA_patternsView
    DMA_PCRE
    DMA_Plantuml
    )

if(TURN_OFF_RCC)
    set(CMAKE_AUTORCC OFF)
    set(TURN_OFF_RCC OFF)
endif()
################### DLT_MESSAGE_ANALYZER ( END ) ##########

################### ANTLR_LINKAGE #########################
target_link_libraries(DLT-Message-Analyzer antlr4_PCRE_static)
target_link_libraries(DLT-Message-Analyzer antlr4_static)
################### ANTLR_LINKAGE ( END ) #################

################### PLANTUML ##############################
# Copy plantuml.jar after the build
add_custom_command(TARGET DLT-Message-Analyzer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/plantuml/plantuml.jar $<TARGET_FILE_DIR:DLT-Message-Analyzer>/)
################### PLANTUML ( END ) ######################

################### QT ####################################
target_link_libraries(DLT-Message-Analyzer qdlt Qt5::Widgets )
################### QT ( END ) ############################

add_plugin(DLT-Message-Analyzer)
