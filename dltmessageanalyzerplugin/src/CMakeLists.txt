set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_AUTOUIC ON)

# cmake 2.8.12 doesn't have AUTOUIC ??
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

target_link_libraries(dltmessageanalyzerplugin qdlt Qt5::Widgets )
install(TARGETS dltmessageanalyzerplugin DESTINATION deploy/plugins)
