set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_AUTOUIC ON)

if( DMA_CLANG_TIDY_BUILD ) 
    message("-- [DLT-Message-Analyzer]: clang-tidy build.")
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    list(APPEND CMAKE_CXX_CLANG_TIDY 
    "-format-style='file'"
    "-warnings-as-errors=*" )
endif()

QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )

add_library(dltmessageanalyzerplugin MODULE 
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
    ${UI_HEADERS})

# Copy plantuml.jar after the build
add_custom_command(TARGET dltmessageanalyzerplugin POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/plantuml/plantuml.jar $<TARGET_FILE_DIR:dltmessageanalyzerplugin>/)

# Turn this on if you want build, compatible with the PLUGIN_INTERFACE_VERSION "1.0.0"
# target_compile_definitions(dltmessageanalyzerplugin PRIVATE -DPLUGIN_API_COMPATIBILITY_MODE_1_0_0)

target_link_libraries(dltmessageanalyzerplugin qdlt Qt5::Widgets )
install(TARGETS dltmessageanalyzerplugin DESTINATION deploy/plugins)
