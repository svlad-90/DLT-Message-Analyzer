# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(dltmessageanalyzerplugin)

INCLUDEPATH+=dltmessageanalyzerplugin

# plugin header files
HEADERS += \
    dltmessageanalyzerplugin/src/CBGColorAnimation.hpp \
    dltmessageanalyzerplugin/src/CConsoleCtrl.hpp \
    dltmessageanalyzerplugin/src/CContinuousAnalyzer.hpp \
    dltmessageanalyzerplugin/src/CDLTFileWrapper.hpp \
    dltmessageanalyzerplugin/src/CDLTMessageAnalyzer.hpp \
    dltmessageanalyzerplugin/src/CDLTMsgWrapper.hpp \
    dltmessageanalyzerplugin/src/CDLTRegexAnalyzerWorker.hpp \
    dltmessageanalyzerplugin/src/CFiltersModel.hpp \
    dltmessageanalyzerplugin/src/CFiltersView.hpp \
    dltmessageanalyzerplugin/src/CGroupedView.hpp \
    dltmessageanalyzerplugin/src/CLogo.hpp \
    dltmessageanalyzerplugin/src/CMTAnalyzer.hpp \
    dltmessageanalyzerplugin/src/CPatternsModel.hpp \
    dltmessageanalyzerplugin/src/CPatternsView.hpp \
    dltmessageanalyzerplugin/src/CRegexDirectoryMonitor.hpp \
    dltmessageanalyzerplugin/src/CSearchResultHighlightingDelegate.hpp \
    dltmessageanalyzerplugin/src/CSearchResultModel.hpp \
    dltmessageanalyzerplugin/src/CSearchResultView.hpp \
    dltmessageanalyzerplugin/src/CSettingsManager.hpp \
    dltmessageanalyzerplugin/src/CGroupedViewModel.hpp \
    dltmessageanalyzerplugin/src/Definitions.hpp \
    dltmessageanalyzerplugin/src/IDLTMessageAnalyzerController.hpp \
    dltmessageanalyzerplugin/src/IDLTMessageAnalyzerControllerConsumer.hpp \
    dltmessageanalyzerplugin/src/CTreeItem.hpp \
    dltmessageanalyzerplugin/src/dltmessageanalyzerplugin.hpp \
    dltmessageanalyzerplugin/src/form.h \
    dltmessageanalyzerplugin/src/variant/variant.hpp

# plugin source files
SOURCES += \
    dltmessageanalyzerplugin/src/CBGColorAnimation.cpp \
    dltmessageanalyzerplugin/src/CConsoleCtrl.cpp \
    dltmessageanalyzerplugin/src/CContinuousAnalyzer.cpp \
    dltmessageanalyzerplugin/src/CDLTFileWrapper.cpp \
    dltmessageanalyzerplugin/src/CDLTMsgWrapper.cpp \
    dltmessageanalyzerplugin/src/CDLTRegexAnalyzerWorker.cpp \
    dltmessageanalyzerplugin/src/CFiltersModel.cpp \
    dltmessageanalyzerplugin/src/CFiltersView.cpp \
    dltmessageanalyzerplugin/src/CGroupedView.cpp \
    dltmessageanalyzerplugin/src/CLogo.cpp \
    dltmessageanalyzerplugin/src/CMTAnalyzer.cpp \
    dltmessageanalyzerplugin/src/CPatternsModel.cpp \
    dltmessageanalyzerplugin/src/CPatternsView.cpp \
    dltmessageanalyzerplugin/src/CRegexDirectoryMonitor.cpp \
    dltmessageanalyzerplugin/src/CSearchResultHighlightingDelegate.cpp \
    dltmessageanalyzerplugin/src/CSearchResultModel.cpp \
    dltmessageanalyzerplugin/src/CSearchResultView.cpp \
    dltmessageanalyzerplugin/src/CSettingsManager.cpp \
    dltmessageanalyzerplugin/src/CGroupedViewModel.cpp \
    dltmessageanalyzerplugin/src/CTreeItem.cpp \
    dltmessageanalyzerplugin/src/Definitions.cpp \
    dltmessageanalyzerplugin/src/IDLTMessageAnalyzerController.cpp \
    dltmessageanalyzerplugin/src/IDLTMessageAnalyzerControllerConsumer.cpp \
    dltmessageanalyzerplugin/src/dltmessageanalyzerplugin.cpp \
    dltmessageanalyzerplugin/src/CDLTMessageAnalyzer.cpp\
    dltmessageanalyzerplugin/src/form.cpp

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
    dltmessageanalyzerplugin/src/dltmessageanalyzer.qrc
