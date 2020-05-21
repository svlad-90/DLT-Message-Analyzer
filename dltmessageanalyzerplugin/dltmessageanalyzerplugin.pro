# include global settings for all DLT Viewer Plugins
include( ../plugin.pri )

# Put intermediate files in the build directory
MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

# target name
TARGET = $$qtLibraryTarget(dltmessageanalyzerplugin)

# plugin header files
HEADERS += \
    CBGColorAnimation.hpp \
    CConsoleCtrl.hpp \
    CContinuousAnalyzer.hpp \
    CDLTFileWrapper.hpp \
    CDLTMessageAnalyzer.hpp \
    CDLTMsgWrapper.hpp \
    CDLTRegexAnalyzerWorker.hpp \
    CFiltersModel.hpp \
    CFiltersView.hpp \
    CGroupedView.hpp \
    CLogo.hpp \
    CMTAnalyzer.hpp \
    CPatternsModel.hpp \
    CPatternsView.hpp \
    CRegexDirectoryMonitor.hpp \
    CSearchResultHighlightingDelegate.hpp \
    CSearchResultModel.hpp \
    CSearchResultView.hpp \
    CSettingsManager.hpp \
    CGroupedViewModel.hpp \
    Definitions.hpp \
    IDLTMessageAnalyzerController.hpp \
    IDLTMessageAnalyzerControllerConsumer.hpp \
    CTreeItem.hpp \
    dltmessageanalyzerplugin.hpp \
    form.h \
    variant\variant.hpp

# plugin source files
SOURCES += \
    CBGColorAnimation.cpp \
    CConsoleCtrl.cpp \
    CContinuousAnalyzer.cpp \
    CDLTFileWrapper.cpp \
    CDLTMsgWrapper.cpp \
    CDLTRegexAnalyzerWorker.cpp \
    CFiltersModel.cpp \
    CFiltersView.cpp \
    CGroupedView.cpp \
    CLogo.cpp \
    CMTAnalyzer.cpp \
    CPatternsModel.cpp \
    CPatternsView.cpp \
    CRegexDirectoryMonitor.cpp \
    CSearchResultHighlightingDelegate.cpp \
    CSearchResultModel.cpp \
    CSearchResultView.cpp \
    CSettingsManager.cpp \
    CGroupedViewModel.cpp \
    CTreeItem.cpp \
    Definitions.cpp \
    IDLTMessageAnalyzerController.cpp \
    IDLTMessageAnalyzerControllerConsumer.cpp \
    dltmessageanalyzerplugin.cpp \
    CDLTMessageAnalyzer.cpp\
    form.cpp

# plugin forms
FORMS += \
    form.ui

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
    dltmessageanalyzer.qrc
