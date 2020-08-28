################### CPP ###################################
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
################### CPP ( END ) ###########################

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

################### CLANG_TIDY ############################
if( DMA_CLANG_TIDY_BUILD ) 
    message("-- [DLT-Message-Analyzer]: clang-tidy build.")
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    list(APPEND CMAKE_CXX_CLANG_TIDY 
    "-format-style='file'"
    "-warnings-as-errors=*" )
endif()
################### CLANG_TIDY ( END ) ####################

################### DLT_MESSAGE_ANALYZER ##################
QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )

set(TURN_OFF_RCC OFF)

if(NOT CMAKE_AUTORCC)
    set(CMAKE_AUTORCC ON)
    set(TURN_OFF_RCC ON)
endif()

add_library(DLT-Message-Analyzer MODULE
    dltmessageanalyzerplugin.cpp
    CDLTMessageAnalyzer.cpp
    form.cpp
    settings/CSettingsManager.cpp
    settings/ISettingItem.cpp
    settings/CSettingItem.cpp
    logo/CLogo.cpp
    log/CConsoleCtrl.cpp
    log/CConsoleView.cpp
    log/CConsoleInputProcessor.cpp
    analyzer/IDLTMessageAnalyzerController.cpp
    analyzer/IDLTMessageAnalyzerControllerConsumer.cpp
    analyzer/CMTAnalyzer.cpp
    analyzer/CContinuousAnalyzer.cpp
    analyzer/CDLTRegexAnalyzerWorker.cpp
    common/CTreeItem.cpp
    common/Definitions.cpp
    common/CBGColorAnimation.cpp
    common/CRegexDirectoryMonitor.cpp
    common/CTableMemoryJumper.cpp
    common/OSHelper.cpp
    dltWrappers/CDLTFileWrapper.cpp
    dltWrappers/CDLTMsgWrapper.cpp
    filtersView/CFiltersModel.cpp
    filtersView/CFiltersView.cpp
    groupedView/CGroupedView.cpp
    groupedView/CGroupedViewModel.cpp
    patternsView/CPatternsModel.cpp
    patternsView/CPatternsView.cpp
    searchView/CSearchResultHighlightingDelegate.cpp
    searchView/CSearchResultModel.cpp
    searchView/CSearchResultView.cpp
#    uml/CSVGView.hpp
    plant_uml/CUMLView.cpp
    plant_uml/CImageViewer.cpp
    ${UI_HEADERS}
    resources/dltmessageanalyzer.qrc)

if(TURN_OFF_RCC)
    set(CMAKE_AUTORCC OFF)
    set(TURN_OFF_RCC OFF)
endif()
################### DLT_MESSAGE_ANALYZER ( END ) ##########

################### ANTLR_LINKAGE #########################
# add generated grammar to demo binary target
target_link_libraries(DLT-Message-Analyzer antlr4_PCRE_static)
target_link_libraries(DLT-Message-Analyzer antlr4_static)
################### ANTLR_LINKAGE ( END ) #################

################### PLANTUML ##############################
# Copy plantuml.jar after the build
add_custom_command(TARGET DLT-Message-Analyzer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/plantuml/plantuml.jar $<TARGET_FILE_DIR:DLT-Message-Analyzer>/)
################### PLANTUML ( END ) ######################

################### COMPATIBILITY #########################
# Turn this on if you want build, compatible with the PLUGIN_INTERFACE_VERSION "1.0.0"
# target_compile_definitions(DLT-Message-Analyzer PRIVATE -DPLUGIN_API_COMPATIBILITY_MODE_1_0_0)
################### COMPATIBILITY ( END ) #################

################### QT ####################################
target_link_libraries(DLT-Message-Analyzer qdlt Qt5::Widgets )
################### QT ( END ) ############################

################### INSTALLATION ##########################
install(TARGETS DLT-Message-Analyzer DESTINATION deploy/plugins)
################### INSTALLATION ( END ) ##################
