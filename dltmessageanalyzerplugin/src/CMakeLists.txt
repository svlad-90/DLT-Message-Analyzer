################### CPP ###################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
################### CPP ( END ) ###########################

########## OPTIONS ############
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(DMA_TC_MALLOC_OPTIMIZATION "[
        This option enables tcmalloc RAM usage optimization.
        You will need to link dlt-viewer against tcmalloc to use this feature.
        ]" OFF)
    option(DMA_TC_MALLOC_PROFILING "[
        This option enables tcmalloc profiling. Adds 'dump-memory-stats' command to the console view.
        You will need to link dlt-viewer against tcmalloc to use this feature.
        ]" OFF)

    if(DMA_TC_MALLOC_OPTIMIZATION)
        add_compile_definitions(DMA_TC_MALLOC_OPTIMIZATION_ENABLED)
        message(STATUS "DMA_TC_MALLOC_OPTIMIZATION enabled")
    else()
        add_compile_definitions(DMA_GLIBC_MALLOC_OPTIMIZATION_ENABLED)
    endif()

    if(DMA_TC_MALLOC_PROFILING)
        add_compile_definitions(DMA_TC_MALLOC_PROFILING_ENABLED)
        message(STATUS "DMA_TC_MALLOC_PROFILING enabled")
    endif()
endif()
########## OPTIONS ( END ) ####

################### QT_SPECIFIC ###########################
if(${QT_PREFIX}Core_VERSION VERSION_LESS "5.15.0")
    message("DMA_QT5_COMPATIBILITY_MODE is set")
    set(DMA_QT5_COMPATIBILITY_MODE TRUE)
else()
    set(DMA_QT5_COMPATIBILITY_MODE FALSE)
endif()

function (DMA_qt_wrap_cpp processed_mocs_var_name wrap_src_var_name)
    set(local_processed_mocs)
    if(DMA_QT5_COMPATIBILITY_MODE)
      qt5_wrap_cpp(local_processed_mocs ${${wrap_src_var_name}})
    else()
      qt_wrap_cpp(local_processed_mocs ${${wrap_src_var_name}})
    endif()
    set(${processed_mocs_var_name} ${local_processed_mocs} PARENT_SCOPE)
endfunction()

function (DMA_QT_WRAP_UI ui_headers_var_name wrap_ui_var_name)
    set(local_ui_headers)
    if(DMA_QT5_COMPATIBILITY_MODE)
      QT5_WRAP_UI(local_ui_headers ${${wrap_ui_var_name}})
    else()
      QT_WRAP_UI(local_ui_headers ${${wrap_ui_var_name}})
    endif()
    set(${ui_headers_var_name} ${local_ui_headers} PARENT_SCOPE)
endfunction()
################### QT_SPECIFIC ( END ) ###################

################### QCUSTOMPLOT ###########################
# function - sync_qcustomplot
# Syncs QCustomPlot, implementation, which is used for
# the creation of plots
function (DMA_sync_q_custom_plot)
    include(FetchContent)
    FetchContent_Declare(
      qcustomplot
      GIT_REPOSITORY https://github.com/svlad-90/QCustomPlot-library
      GIT_TAG        2.1.1.1
    ) 
    
    FetchContent_GetProperties(qcustomplot)
    if(NOT qcustomplot_POPULATED)
        FetchContent_Populate(qcustomplot)
        add_subdirectory(${qcustomplot_SOURCE_DIR} ${qcustomplot_BINARY_DIR})
    endif()

#    include_directories(${qcustomplot_SOURCE_DIR}/DMA_Plantuml/include)
endfunction(DMA_sync_q_custom_plot)
################### QCUSTOMPLOT ( END ) ###################

################### NLOHMANN_JSON #########################
# function - sync_qcustomplot
# Syncs QCustomPlot, implementation, which is used for
# the creation of plots
function (DMA_sync_nlohmann_json)
    include(FetchContent)
    FetchContent_Declare(
      nlohmannjson
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG        v3.11.3
    )

    FetchContent_GetProperties(nlohmannjson)
    if(NOT nlohmannjson_POPULATED)
        FetchContent_Populate(nlohmannjson)
        add_subdirectory(${nlohmannjson_SOURCE_DIR} ${nlohmannjson_BINARY_DIR})
    endif()

    include_directories(${nlohmannjson_SOURCE_DIR}/include)
endfunction(DMA_sync_nlohmann_json)
################### QCUSTOMPLOT ( END ) ###################

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
DMA_sync_q_custom_plot()
DMA_sync_nlohmann_json()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(TARGETS qcustomplot LIBRARY DESTINATION ${DLT_LIBRARY_INSTALLATION_PATH} )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(TARGETS qcustomplot RUNTIME DESTINATION ${DLT_EXECUTABLE_INSTALLATION_PATH} )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    install(TARGETS qcustomplot LIBRARY DESTINATION ${DLT_LIBRARY_INSTALLATION_PATH} )
endif()

################### DEPENDENCIES ( END )###################

################### COMPATIBILITY #########################
# Turn this on if you want build, compatible with the PLUGIN_INTERFACE_VERSION "1.0.0"
# add_definitions(-DPLUGIN_API_COMPATIBILITY_MODE_1_0_0)
################### COMPATIBILITY ( END ) #################

################### ANTLR #################################
# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

cmake_policy(SET CMP0071 NEW)

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
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/antlr/antlr-4.13.1-complete.jar)
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

################### PROJECT ###############################
enable_testing()
DMA_Prepare()
include(cmake/CMakeHelper.cmake)
################### PROJECT ( END ) #######################

set(TURN_OFF_RCC OFF)

if(NOT CMAKE_AUTORCC)
    set(CMAKE_AUTORCC ON)
    set(TURN_OFF_RCC ON)
endif()

include_directories(../../thirdparty/DMA_Plantuml/src)

# to force variant lite to use nonstd::variant
# otherwise the implementation will throw compile-time errors
# with C++17
add_compile_definitions(variant_CONFIG_SELECT_VARIANT=variant_VARIANT_NONSTD)

DMA_add_subdirectory_with_clang_tidy(common)
DMA_add_subdirectory_with_clang_tidy(components)
DMA_add_subdirectory_with_clang_tidy(plugin)

add_library(DLT-Message-Analyzer MODULE
    dltmessageanalyzerplugin.cpp
    resources/dltmessageanalyzer.qrc)

target_link_libraries(DLT-Message-Analyzer
    DMA_plugin
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
target_link_libraries(DLT-Message-Analyzer qdlt ${QT_PREFIX}::Widgets )
################### QT ( END ) ############################

########## ENSURE INSTALLATION OF QT DEPENDENCIES #########
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # used by the DLT Message Analyzer itself
    find_package(${QT_PREFIX} REQUIRED COMPONENTS Svg)
    # used by qcustomplot
    find_package(${QT_PREFIX} REQUIRED COMPONENTS PrintSupport)
    set(QT_LIBS
      ${QT_PREFIX}::Svg
      ${QT_PREFIX}::PrintSupport)

    foreach(QT_LIB IN ITEMS ${QT_LIBS})
      get_target_property(LIBRARY_PATH ${QT_LIB} LOCATION)
      install(FILES
          "${LIBRARY_PATH}"
          DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
          COMPONENT qt_libraries)
    endforeach()
endif()
####### ENSURE INSTALLATION OF QT DEPENDENCIES ( END ) ####

add_plugin(DLT-Message-Analyzer)
