/**
 * @file    CDLTMessageAnalyzer.hpp
 * @author  vgoncharuk
 * @brief   Definition of the CDLTMessageAnalyzer class
 */

#ifndef CDLTMESSAGEANALYZER_HPP
#define CDLTMESSAGEANALYZER_HPP

#include "memory"

#include <QtWidgets/QTreeView>
#include <QtWidgets/QProgressBar>
#include "QtWidgets/QLabel"
#include "QtWidgets/QTableView"
#include "QtWidgets/QComboBox"

#include "plugininterface.h"
#include "qdltpluginmanager.h"

#include "common/Definitions.hpp"
#include "analyzer/IDLTMessageAnalyzerControllerConsumer.hpp"

#ifdef DEBUG_BUILD
#include "QTime"
#endif

class QRegularExpression;
class QCheckBox;
class QPushButton;

class CGroupedViewModel;
typedef CGroupedViewModel* tGroupedViewModelPtr;
class CSearchResultModel;
typedef CSearchResultModel* tSearchResultModelPtr;
class CSearchResultView;
class CGroupedView;
class CPatternsView;
class CRegexDirectoryMonitor;
class CFiltersView;
class CFiltersModel;

/**
 * @brief The CDLTMessageAnalyzer class - used as a main controller of the plugin.
 * Entry point to majority of plugin's business-logic.
 */
class CDLTMessageAnalyzer : public IDLTMessageAnalyzerControllerConsumer
{
    Q_OBJECT

    public:
    /**
         * @brief CDLTMessageAnalyzer - Constructor. Creates instance of dlt message analyzer
         * @param pController - the weak pointer to controller
         * @param pGroupedView - instance of grouped view
         * @param pProgressBarLabel - instance of progress bar label
         * @param pProgressBar - instance of progress bar
         * @param regexLineEdit - instance of regex line edit
         * @param pLabel - instance of status label
         * @param pPatternsTableView - instance of patterns table view
         * @param pNumberOfThreadsCombobBox - "number of threads" combo-box
         * @param pSearchResultTableView - search result table view
         * @param pContinuousSearchCheckBox - instance of
         * @param pCacheStatusLabel - instance of cache status label
         * @param pMainTabWidget - instance of main tab widget
         * @param pPatternsSearchInput - instance of regex patterns search input widget
         * @param pRegexSelectionComboBox - instance of regex selection combo-box
         * @param pFiltersView - instance of filters view
         * @param pFiltersSearchInput - instance of filters search input
         */
        CDLTMessageAnalyzer(const std::weak_ptr<IDLTMessageAnalyzerController>& pController,
                            CGroupedView* pGroupedView, QLabel* pProgressBarLabel, QProgressBar* pProgressBar, QLineEdit* regexLineEdit,
                            QLabel* pLabel, CPatternsView* pPatternsTableView, QComboBox* pNumberOfThreadsCombobBox,
                            CSearchResultView* pSearchResultTableView,
                            QCheckBox* pContinuousSearchCheckBox,
                            QLabel* pCacheStatusLabel, QTabWidget* pMainTabWidget,
                            QLineEdit* pPatternsSearchInput,
                            QComboBox* pRegexSelectionComboBox,
                            CFiltersView* pFiltersView, QLineEdit* pFiltersSearchInput);

        /**
         * Destructor
         */
        ~CDLTMessageAnalyzer();

        /**
         * @brief setFile - set's the file to be used for analysis
         * @param pFile - pointer to file to be used
         */
        void setFile(const tDLTFileWrapperPtr& pFile);

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        /**
         * @brief setMainTableView - sets main table view, which is used for jumping functioanlity
         * @param pMainTableView - pointer to the instance of the main table view
         */
        void setMainTableView( QTableView* pMainTableView );

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
         * @return - trye in case of successful start of analysis.
         * False otherwise.
         * Note! This call will stop previous search, is one is already running.
         */
        bool analyze();

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
         * @param index - index to jump to.
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
         * @brief decodeMsg - allows to decode the message.
         * @param msg - message to be decoded.
         */
        void decodeMsg(QDltMsg& msg) const;

signals:
        /**
         * @brief analysisStatusChanged - fired as soon as analysis status is changing
         * @param analysisRunning - whether analysis is running or not.
         */
        void analysisStatusChanged( bool analysisRunning );

    private:

        void setReuqestId( const tRequestId& val );

        void tryStop();

        // IDLTMessageAnalyzerControllerConsumer interface
        void progressNotification(const tRequestId &requestId,
                                  const eRequestState &requestState,
                                  const int8_t &progress,
                                  const tFoundMatchesPack &processedMatches);
        void animateError( QWidget* pAnimationWidget );

        void updateProgress(int progress, eRequestState requestState, bool silent);

        std::shared_ptr<QRegularExpression> createRegex( const QString& regex,
                                                         const QString& onSuccessMessages,
                                                         const QString& onFailureMessages,
                                                         bool appendRegexError,
                                                         QWidget* pErrorAnimationWidget = nullptr );

        void updateStatusLabel( const QString& text, bool isError = false );
        void processOverwritePattern(const QString& alias, const QString checkedRegex, const QModelIndex editItem = QModelIndex());

#ifdef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        void tryLoadDecoderPlugins();
#endif

        void handleLoadedConfig();
        void handleLoadedRegexConfig();
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
        QLineEdit* mpRegexLineEdit;
        QLabel* mpLabel;
        QComboBox* mpNumberOfThreadsCombobBox;
        QTableView* mpMainTableView;
        QCheckBox* mpContinuousSearchCheckBox;
        QLabel* mpCacheStatusLabel;
        QTabWidget* mpMainTabWidget;
        QLineEdit* mpFiltersSearchInput;

        //custom widgets and models
        CGroupedView* mpGroupedResultView;
        tGroupedViewModelPtr mpGroupedViewModel;
        QComboBox* mpRegexSelectionComboBox;

        CSearchResultView* mpSearchResultTableView;
        tSearchResultModelPtr mpSearchResultModel;

        CPatternsView* mpPatternsTreeView;
        tPatternsModelPtr mpAvailablePatternsModel;

        CFiltersView* mpFiltersView;
        CFiltersModel* mpFiltersModel;

        // internal states
        tRangeProperty mSearchRange;
        tRequestId mRequestId;
        int mNumberOfDots;
        bool mbIsConnected;
        tDLTFileWrapperPtr mpFile;

#ifndef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
        QDltMessageDecoder* mpMessageDecoder;
#else
        tPluginPtrList mDecoderPluginsList;
#endif
        QDltPluginManager mPluginManager;
        std::shared_ptr<CRegexDirectoryMonitor> mpRegexDirectoryMonitor;

        // timers
#ifdef DEBUG_BUILD
        QTime mMeasurementNotificationTimer;
#endif

        QTime mMeasurementRequestTimer;
};

#endif // CDLTMESSAGEANALYZER_HPP
