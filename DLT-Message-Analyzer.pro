# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# Turn this on if you want build, compatible with the PLUGIN_INTERFACE_VERSION "1.0.0"
#DEFINES += PLUGIN_API_COMPATIBILITY_MODE_1_0_0

# target name
TARGET = $$qtLibraryTarget(dltmessageanalyzerplugin)

INCLUDEPATH+=dltmessageanalyzerplugin

# plugin header files
HEADERS += \
    dltmessageanalyzerplugin/src/CDLTMessageAnalyzer.hpp \
    dltmessageanalyzerplugin/src/dltmessageanalyzerplugin.hpp \
    dltmessageanalyzerplugin/src/form.h \
    \
    dltmessageanalyzerplugin/src/settings/CSettingsManager.hpp \
    \
    dltmessageanalyzerplugin/src/logo/CLogo.hpp \
    \
    dltmessageanalyzerplugin/src/log/CConsoleCtrl.hpp \
    \
    dltmessageanalyzerplugin/src/analyzer/IDLTMessageAnalyzerController.hpp \
    dltmessageanalyzerplugin/src/analyzer/IDLTMessageAnalyzerControllerConsumer.hpp \
    dltmessageanalyzerplugin/src/analyzer/CDLTRegexAnalyzerWorker.hpp \
    dltmessageanalyzerplugin/src/analyzer/CMTAnalyzer.hpp \
    dltmessageanalyzerplugin/src/analyzer/CContinuousAnalyzer.hpp \
    \
    dltmessageanalyzerplugin/src/common/Definitions.hpp \
    dltmessageanalyzerplugin/src/common/CTreeItem.hpp \
    dltmessageanalyzerplugin/src/common/variant/variant.hpp \
    dltmessageanalyzerplugin/src/common/CBGColorAnimation.hpp \
    dltmessageanalyzerplugin/src/common/CRegexDirectoryMonitor.hpp \
    \
    dltmessageanalyzerplugin/src/dltWrappers/CDLTFileWrapper.hpp \
    dltmessageanalyzerplugin/src/dltWrappers/CDLTMsgWrapper.hpp \
    \
    dltmessageanalyzerplugin/src/filtersView/CFiltersModel.hpp \
    dltmessageanalyzerplugin/src/filtersView/CFiltersView.hpp \
    \
    dltmessageanalyzerplugin/src/groupedView/CGroupedView.hpp \
    dltmessageanalyzerplugin/src/groupedView/CGroupedViewModel.hpp \
    \
    dltmessageanalyzerplugin/src/patternsView/CPatternsModel.hpp \
    dltmessageanalyzerplugin/src/patternsView/CPatternsView.hpp \
    \
    dltmessageanalyzerplugin/src/searchView/CSearchResultHighlightingDelegate.hpp \
    dltmessageanalyzerplugin/src/searchView/CSearchResultModel.hpp \
    dltmessageanalyzerplugin/src/searchView/CSearchResultView.hpp

# plugin source files
SOURCES += \
    dltmessageanalyzerplugin/src/dltmessageanalyzerplugin.cpp \
    dltmessageanalyzerplugin/src/CDLTMessageAnalyzer.cpp\
    dltmessageanalyzerplugin/src/form.cpp \
    \
    dltmessageanalyzerplugin/src/settings/CSettingsManager.cpp \
    \
    dltmessageanalyzerplugin/src/logo/CLogo.cpp \
    \
    dltmessageanalyzerplugin/src/log/CConsoleCtrl.cpp \
    \
    dltmessageanalyzerplugin/src/analyzer/IDLTMessageAnalyzerController.cpp \
    dltmessageanalyzerplugin/src/analyzer/IDLTMessageAnalyzerControllerConsumer.cpp \
    dltmessageanalyzerplugin/src/analyzer/CMTAnalyzer.cpp \
    dltmessageanalyzerplugin/src/analyzer/CContinuousAnalyzer.cpp \
    dltmessageanalyzerplugin/src/analyzer/CDLTRegexAnalyzerWorker.cpp \
    \
    dltmessageanalyzerplugin/src/common/CTreeItem.cpp \
    dltmessageanalyzerplugin/src/common/Definitions.cpp \
    dltmessageanalyzerplugin/src/common/CBGColorAnimation.cpp \
    dltmessageanalyzerplugin/src/common/CRegexDirectoryMonitor.cpp \
    \
    dltmessageanalyzerplugin/src/dltWrappers/CDLTFileWrapper.cpp \
    dltmessageanalyzerplugin/src/dltWrappers/CDLTMsgWrapper.cpp \
    \
    dltmessageanalyzerplugin/src/filtersView/CFiltersModel.cpp \
    dltmessageanalyzerplugin/src/filtersView/CFiltersView.cpp \
    \
    dltmessageanalyzerplugin/src/groupedView/CGroupedView.cpp \
    dltmessageanalyzerplugin/src/groupedView/CGroupedViewModel.cpp \
    \
    dltmessageanalyzerplugin/src/patternsView/CPatternsModel.cpp \
    dltmessageanalyzerplugin/src/patternsView/CPatternsView.cpp \
    \
    dltmessageanalyzerplugin/src/searchView/CSearchResultHighlightingDelegate.cpp \
    dltmessageanalyzerplugin/src/searchView/CSearchResultModel.cpp \
    dltmessageanalyzerplugin/src/searchView/CSearchResultView.cpp

# plugin forms
FORMS += \
    dltmessageanalyzerplugin/src/form.ui

# Detect QT5 and comply to new Widgets hierarchy
greaterThan(QT_VER_MAJ, 4) {
    QT += widgets
    INCLUDEPATH += QtWidgets
    DEFINES += QT5
}

linux-g++ {

    CONFIG += c++14
    QMAKE_CXXFLAGS = -std=c++14
    QMAKE_CXXFLAGS += -Wall
    QMAKE_CXXFLAGS += -Wextra
    # Limit symbol visibility to avoid symbol clashes between different
    # plugins
    QMAKE_CXXFLAGS += -fvisibility=hidden
}

win32 {
    CONFIG += c++11
}

RESOURCES += \
    dltmessageanalyzerplugin/src/resources/dltmessageanalyzer.qrc
