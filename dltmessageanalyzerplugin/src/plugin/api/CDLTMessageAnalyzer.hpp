/**
 * @file    CDLTMessageAnalyzer.hpp
 * @author  vgoncharuk
 * @brief   Definition of the CDLTMessageAnalyzer class
 */

#pragma once

#include "memory"

#include <QtWidgets/QTreeView>
#include <QtWidgets/QProgressBar>
#include "QtWidgets/QLabel"
#include "QtWidgets/QTableView"
#include "QtWidgets/QComboBox"

#include "plugininterface.h"

#ifdef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
#include "qdltpluginmanager.h"
#endif


#include "common/Definitions.hpp"
#include "components/analyzer/api/IDLTMessageAnalyzerControllerConsumer.hpp"
#include "components/settings/api/CSettingsManagerClient.hpp"
#include "components/coverageNote/api/ICoverageNoteProvider.hpp"

#include "QElapsedTimer"

class QRegularExpression;
class QCheckBox;
class QPushButton;

class IGroupedViewModel;
typedef std::shared_ptr<IGroupedViewModel> tGroupedViewModelPtr;
class CSearchResultView;
class CGroupedView;
class CPatternsView;
class CRegexDirectoryMonitor;
class CFiltersView;
class IFiltersModel;
class CUMLView;
class CSearchResultView;
class ISearchResultModel;
class CTableMemoryJumper;
class CCustomPlotExtended;
class CRegexHistoryTextEdit;

/**
 * @brief The CDLTMessageAnalyzer class - used as a main controller of the plugin.
 * Entry point to majority of plugin's business-logic.
 */
class CDLTMessageAnalyzer : public IDLTMessageAnalyzerControllerConsumer,
                            public CSettingsManagerClient
{
    Q_OBJECT

    public:

        CDLTMessageAnalyzer(const std::weak_ptr<IDLTMessageAnalyzerController>& pController,
                            const tGroupedViewModelPtr& pGroupedViewModel,
                            QLabel* pProgressBarLabel, QProgressBar* pProgressBar, CRegexHistoryTextEdit* pRegexLineEdit,
                            QLabel* pLabel, CPatternsView* pPatternsTableView, const tPatternsModelPtr& pPatternsModel,
                            QComboBox* pNumberOfThreadsCombobBox,
                            QCheckBox* pContinuousSearchCheckBox,
                            QLabel* pCacheStatusLabel, QTabWidget* pMainTabWidget,
                            QLineEdit* pPatternsSearchInput,
                            QComboBox* pRegexSelectionComboBox,
                            CFiltersView* pFiltersView, const std::shared_ptr<IFiltersModel>& pFiltersModel,
                            QLineEdit* pFiltersSearchInput,
                            CUMLView* pUMLView, const std::shared_ptr<CTableMemoryJumper>& pSearchViewTableJumper,
                            CSearchResultView* pSearchResultView,
                            const std::shared_ptr<ISearchResultModel>& pSearchResultModel,
                            const std::weak_ptr<IDLTLogsWrapperCreator>& pDLTLogsWrapperCreator,
                            const tSettingsManagerPtr& pSettingsManager,
                            CCustomPlotExtended* pCustomPlotExtended,
                            const tCoverageNoteProviderPtr& pCoverageNoteProvider);

        /**
         * @brief setFile - set's the file to be used for analysis
         * @param pFile - pointer to file to be used
         */
        void setFile(const tFileWrapperPtr& pFile);

        /**
         * @brief setMainTableView - sets main table view, which is used for jumping functioanlity
         * @param pMainTableView - pointer to the instance of the main table view
         */
        void setMainTableView( QTableView* pMainTableView );

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        /**
         * @brief configurationChanged - used to notify from the outside, that configuration of the dlt-viewer has changed.
         */
        void configurationChanged();

        /**
         * @brief setMessageDecoder - set's an instance of message decoder to be used for decoding of the analyzed messages
         * @param pMessageDecoder - pointer to instance of the message decoder
         */
        void setMessageDecoder( QDltMessageDecoder* pMessageDecoder );
#endif

        /**
         * @brief analyze - starts analysis
         * @param pSelectedAliases - collection of aliases that were selected for search.
         * @return - true in case of successful start of analysis.
         * False otherwise.
         * Note! This call will stop previous search, is one is already running.
         */
        bool analyze(const QStringList* pSelectedAliases = nullptr);

        /**
         * @brief cancel - cancels analysis, if one is running.
         * In case if nothing is running - does nothing
         */
        void cancel();

        /**
         * @brief addPattern - adds another regex search pattern
         * @param pattern - the pattern to be added.
         */
        void addPattern(const QString& pattern);

        /**
         * @brief deletePattern - start sequence of deletion of currently selected pattern
         */
        void deletePattern();

        /**
         * @brief overwritePattern - start sequence of overwriting of currently selected pattern
         */
        void overwritePattern();

        /**
         * @brief overwritePattern - start sequence of editing of currently selected pattern
         */
        void editPattern();

        /**
         * @brief exportGroupedViewToHTML - exports result of grouped view to HTML
         */
        void exportGroupedViewToHTML();

        /**
         * @brief searchView_clicked_jumpTo_inMainTable - scrolls main table of dlt-viewer to a specified index
         * @param index - index to jump to
         * @param setFocus - whether to set focus to the main table
         */
        void searchView_clicked_jumpTo_inMainTable(const QModelIndex &index);

        /**
         * @brief triggerContinuousAnalysis - updates "continuous analysis" setting
         * @param checked - whether user wants to turn continuous analysis on or off.
         */
        void triggerContinuousAnalysis(bool checked);

        /**
         * @brief triggerNumberOfThreads - updates selected number of threads to be used for analysis.
         * Saves selected number to persistency.
         * The value is fetched from UI elements, so no input parameters presented here.
         */
        void triggerNumberOfThreads();

        /**
         * @brief triggerRegexConfig - triggers consideration of the selected regex configuration file.
         * If selected file has changed - the set of regexes will be fetched from file and corresponding views will be updated.
         */
        void triggerRegexConfig();

        /**
         * @brief filterPatterns - filters pattern by a provided string.
         * @param filter - input string which is used for filtering.
         */
        void filterPatterns( const QString& filter );

        /**
         * @brief filterRegexTokens - filters regex tokens in filters view
         * @param filter - filter, which should be applied to the view
         */
        void filterRegexTokens( const QString& filter );

        enum class eStopAction
        {
            eCancelIfNotFinished,
            eStopIfNotFinished,
            eContinueIfNotFinished
        };

        /**
         * @brief stop - stops the running analysis, considering one of the following scenarios:
         * - cancel if not finished
         * - stop if not finished
         * - continue if not finished
         * @param stopAction - stop action to be applied.
         */
        void stop(eStopAction stopAction);

        /**
         * @brief connectionChanged - notifies this controller, that connection to ECU has changed.
         * @param connected - true if connection is available.
         * False otherwise.
         */
        void connectionChanged(bool connected);

        /**
        * @brief autoscrollStateChanged - notifies this controller, that autoscroll state has changed.
        * @param state - autoscroll state.
        */
        void autoscrollStateChanged(bool state);

        /**
         * @brief decodeMsg - allows to decode the message.
         * @param msg - message to be decoded.
         */
        void decodeMsg(QDltMsg& msg) const;

        /**
         * @brief createSequenceDiagram - trigger to create a sequence diagram
         */
        void createSequenceDiagram() const;

        /**
         * @brief jumpInMainTable - jumps in main dlt viewer table to a selected msg id
         * @param msgId - msg id to jump to
         */
        void jumpInMainTable(const tMsgId& msgId);

signals:
        /**
         * @brief analysisStatusChanged - fired as soon as analysis status is changing
         * @param analysisRunning - whether analysis is running or not.
         */
        void analysisStatusChanged( bool analysisRunning );

    protected:
        bool eventFilter(QObject* pObj, QEvent* pEvent) override;

    private:

        void setReuqestId( const tRequestId& val );

        void tryStop();

        // IDLTMessageAnalyzerControllerConsumer interface
        void progressNotification(const tProgressNotificationData& progressNotificationData);
        void animateError( QWidget* pAnimationWidget );

        void updateProgress(int progress, eRequestState requestState, bool silent);

        std::shared_ptr<QRegularExpression> createRegex( const QString& regex,
                                                         const QString& onSuccessMessages,
                                                         const QString& onFailureMessages,
                                                         bool appendRegexError,
                                                         QLineEdit* pRegexLineEdit,
                                                         QWidget* pErrorAnimationWidget = nullptr);

        std::shared_ptr<QRegularExpression> createRegex( const QString& regex,
                                                         const QString& onSuccessMessages,
                                                         const QString& onFailureMessages,
                                                         bool appendRegexError,
                                                         CRegexHistoryTextEdit* pRegexTextEdit,
                                                         QWidget* pErrorAnimationWidget = nullptr);

        void updateStatusLabel( const QString& text, bool isError = false );
        void processOverwritePattern(const QString& alias, const QString checkedRegex, const QModelIndex editItem = QModelIndex());

#ifdef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        void tryLoadDecoderPlugins();
#endif

        void handleLoadedConfig();
        void resetSearchRange();
        bool isContinuousAnalysis() const;
        void hidePatternLogic(bool userTriggered);
        void updatePatternsInPersistency();

        CDLTMessageAnalyzer(const CDLTMessageAnalyzer&) = delete;
        CDLTMessageAnalyzer& operator=(const CDLTMessageAnalyzer&) = delete;
        CDLTMessageAnalyzer(const CDLTMessageAnalyzer&&) = delete;
        CDLTMessageAnalyzer& operator=(const CDLTMessageAnalyzer&&) = delete;

    private:

        // default widgets
        QLabel* mpProgressBarLabel;
        QProgressBar* mpProgressBar;
        CRegexHistoryTextEdit* mpRegexTextEdit;
        QLabel* mpLabel;
        QComboBox* mpNumberOfThreadsCombobBox;
        QTableView* mpMainTableView;
        QCheckBox* mpContinuousSearchCheckBox;
        QLabel* mpCacheStatusLabel;
        QTabWidget* mpMainTabWidget;
        QLineEdit* mpFiltersSearchInput;

        //custom widgets and models
        tGroupedViewModelPtr mpGroupedViewModel;
        QComboBox* mpRegexSelectionComboBox;

        CSearchResultView* mpSearchResultView;
        std::shared_ptr<ISearchResultModel> mpSearchResultModel;

        CPatternsView* mpPatternsTreeView;
        tPatternsModelPtr mpPatternsModel;

        CFiltersView* mpFiltersView;
        std::shared_ptr<IFiltersModel> mpFiltersModel;

        CUMLView* mpUMLView;

        // internal states
        tIntRangeProperty mSearchRange;
        tRequestId mRequestId;
        int mNumberOfDots;
        bool mbIsConnected;
        tFileWrapperPtr mpFile;

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        QDltMessageDecoder* mpMessageDecoder;
#else
        tPluginPtrList mDecoderPluginsList;
        QDltPluginManager mPluginManager;
#endif
        std::shared_ptr<CRegexDirectoryMonitor> mpRegexDirectoryMonitor;

        // timers
#ifdef DEBUG_BUILD
        QElapsedTimer mMeasurementNotificationTimer;
#endif

        QElapsedTimer mMeasurementRequestTimer;
        std::shared_ptr<CTableMemoryJumper> mpSearchViewTableJumper;
        std::weak_ptr<IDLTLogsWrapperCreator> mpDLTLogsWrapperCreator;

        CCustomPlotExtended* mpCustomPlotExtended;
        tCoverageNoteProviderPtr mpCoverageNoteProvider;
};
