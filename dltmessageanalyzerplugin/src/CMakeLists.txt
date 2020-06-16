set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_AUTOUIC ON)

QT5_WRAP_UI( UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/form.ui )

add_library(dltmessageanalyzerplugin MODULE 
	dltmessageanalyzerplugin.cpp 
    CDLTMessageAnalyzer.cpp
    form.cpp
    settings/CSettingsManager.cpp
    logo/CLogo.cpp
    log/CConsoleCtrl.cpp
    analyzer/IDLTMessageAnalyzerController.cpp
    analyzer/IDLTMessageAnalyzerControllerConsumer.cpp
    analyzer/CMTAnalyzer.cpp
    analyzer/CContinuousAnalyzer.cpp
    analyzer/CDLTRegexAnalyzerWorker.cpp
    common/CTreeItem.cpp
    common/Definitions.cpp
    common/CBGColorAnimation.cpp
    common/CRegexDirectoryMonitor.cpp
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
                          	${UI_HEADERS})

# Turn this on if you want build, compatible with the PLUGIN_INTERFACE_VERSION "1.0.0"
#target_compile_definitions(dltmessageanalyzerplugin PRIVATE -DPLUGIN_API_COMPATIBILITY_MODE_1_0_0)

target_link_libraries(dltmessageanalyzerplugin qdlt Qt5::Widgets )
install(TARGETS dltmessageanalyzerplugin DESTINATION deploy/plugins)
