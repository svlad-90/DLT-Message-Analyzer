#pragma once

#include "memory"

#include "QObject"
#include "QVector"
#include "QColor"
#include "QFont"
#include "QDateTime"
#include "QMap"

#include "common/Definitions.hpp"
#include "common/cpp_extensions.hpp"
#include "common/TOptional.hpp"

class ISettingsManager;
typedef std::shared_ptr<ISettingsManager> tSettingsManagerPtr;

class ISettingsManager : public QObject
{
    Q_OBJECT
public:

    ISettingsManager();
    virtual ~ISettingsManager();

    struct tOperationResult
    {
        bool bResult = false;
        QString err;
    };

    /**
     * @brief setUp - this method is using other methods of this class in order to set it up for further usage.
     * This method SHOULD be called before start of usage of this class.
     * @return - result of the operation
     */
    virtual tOperationResult setUp() = 0;

    /**
     * @brief storeConfigs - stores all types of configuration ( root, settings, regex-es ) from memory to JSON files
     */
    virtual tOperationResult storeConfigs() = 0;

    /**
     * @brief loadConfigs - loads all types of configuration ( root, settings, regex-es ) from JSON files to memory
     */
    virtual tOperationResult loadConfigs() = 0;

    struct tAliasItem
    {
        tAliasItem();
        tAliasItem(bool isDefault_, const QString& alias_, const QString& regex_);
        bool operator==(const tAliasItem&) const;
        bool isDefault;
        QString alias;
        QString regex;
    };
    typedef QMap<QString, tAliasItem> tAliasItemMap;

    enum class eRegexUsageStatisticsItemType
    {
        TEXT = 0,
        STORED_REGEX_PATTERN
    };

    typedef QString tRegexUsageStatisticsKey;

    struct tRegexUsageStatisticsItem
    {
        tRegexUsageStatisticsItem();
        tRegexUsageStatisticsItem(const uint32_t& usageCounter_, const QDateTime& updateDateTime_);
        bool operator==(const tRegexUsageStatisticsItem&) const;
        int usageCounter;
        QDateTime updateDateTime;
    };
    typedef QMap<tRegexUsageStatisticsKey, tRegexUsageStatisticsItem> tRegexUsageStatisticsItemData;
    typedef QMap<eRegexUsageStatisticsItemType, tRegexUsageStatisticsItemData> tRegexUsageStatisticsItemMap;

    //helpers
    virtual bool areAnyDefaultAliasesAvailable() const = 0;
    virtual void resetSearchResultColumnsVisibilityMap() = 0;
    virtual void resetSearchResultColumnsCopyPasteMap() = 0;
    virtual void resetSearchResultColumnsSearchMap() = 0;
    virtual void resetPatternsColumnsVisibilityMap() = 0;
    virtual void resetPatternsColumnsCopyPasteMap() = 0;
    virtual void resetRegexFiltersColumnsVisibilityMap() = 0;
    virtual void resetGroupedViewColumnsVisibilityMap() = 0;
    virtual void resetGroupedViewColumnsCopyPasteMap() = 0;
    virtual QString getRegexDirectory() const = 0;
    virtual QString getRegexUsageStatisticsDirectory() const = 0;
    virtual QString getSettingsFilepath() const = 0;
    virtual QString getUserSettingsFilepath() const = 0;
    virtual QString getRootSettingsFilepath() const = 0;
    virtual QString getDefaultPlantumlPath() const = 0;
    virtual QString getDefaultJavaPath() const = 0;
    virtual void refreshRegexConfiguration() = 0;

////////////////////////SETTERS/////////////////////////////

    // root settings
    virtual void setSettingsManagerVersion(const tSettingsManagerVersion& val) = 0;

    // regex settings
    virtual void setAliases(const tAliasItemMap& val) = 0;
    virtual void setAliasIsDefault(const QString& alias, bool isDefault) = 0;

    // regex usage statistics
    virtual void setRegexUsageStatistics(const tRegexUsageStatisticsItemMap& val) = 0;

    // general settings
    virtual void setNumberOfThreads(const int& val) = 0;
    virtual void setContinuousSearch(bool val) = 0;
    virtual void setCopySearchResultAsHTML(bool val) = 0;
    virtual void setMinimizePatternsViewOnSelection(bool val) = 0;
    virtual void setWriteSettingsOnEachUpdate(bool val) = 0;
    virtual void setCacheEnabled(bool val) = 0;
    virtual void setCacheMaxSizeMB(tCacheSizeMB val) = 0;
    virtual void setRDPMode(bool val) = 0;
    virtual void setRegexMonoHighlightingColor(const QColor& val) = 0;
    virtual void setHighlightActivePatterns(bool val) = 0;
    virtual void setPatternsHighlightingColor(const QColor& val) = 0;
    virtual void setSearchResultMonoColorHighlighting(bool val) = 0;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    virtual void setSearchResultHighlightingGradient(const tHighlightingGradient& val) = 0;
    virtual void setSearchResultColumnsVisibilityMap(const tSearchResultColumnsVisibilityMap& val) = 0;
    virtual void setSearchResultColumnsCopyPasteMap(const tSearchResultColumnsVisibilityMap& val) = 0;
    virtual void setSearchResultColumnsSearchMap(const tSearchResultColumnsVisibilityMap& val) = 0;
    virtual void setMarkTimeStampWithBold(bool val) = 0;
    virtual void setPatternsColumnsVisibilityMap(const tPatternsColumnsVisibilityMap& val) = 0;
    virtual void setPatternsColumnsCopyPasteMap(const tPatternsColumnsVisibilityMap& val) = 0;
    virtual void setCaseSensitiveRegex(bool val) = 0;
    virtual void setRegexFiltersColumnsVisibilityMap(const tRegexFiltersColumnsVisibilityMap& val) = 0;
    virtual void setFilterVariables(bool val) = 0;
    virtual void setGroupedViewColumnsVisibilityMap(const tGroupedViewColumnsVisibilityMap& val) = 0;
    virtual void setGroupedViewColumnsCopyPasteMap(const tGroupedViewColumnsVisibilityMap& val) = 0;
    virtual void setSubFilesHandlingStatus( const bool& val ) = 0;
    virtual void setFont_SearchView( const QFont& val ) = 0;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    virtual void setUML_FeatureActive(const bool& val) = 0;
    virtual void setUML_MaxNumberOfRowsInDiagram(const int& val) = 0;
    virtual void setUML_ShowArguments(const bool& val) = 0;
    virtual void setUML_WrapOutput(const bool& val) = 0;
    virtual void setUML_Autonumber(const bool& val) = 0;
    virtual void setPlotViewFeatureActive(const bool& val) = 0;
    virtual void setFiltersCompletion_CaseSensitive(const bool& val) = 0;
    virtual void setFiltersCompletion_MaxNumberOfSuggestions(const int& val) = 0;
    virtual void setFiltersCompletion_MaxCharactersInSuggestion(const int& val) = 0;
    virtual void setFiltersCompletion_CompletionPopUpWidth(const int& val) = 0;
    virtual void setFiltersCompletion_SearchPolicy(const bool& val) = 0;
    virtual void setSearchViewLastColumnWidthStrategy(const int& val) = 0;
    virtual void setPlantumlPathMode(const int& val) = 0;
    virtual void setPlantumlPathEnvVar(const QString& val) = 0;
    virtual void setPlantumlCustomPath(const QString& val) = 0;
    virtual void setJavaPathMode(const int& val) = 0;
    virtual void setJavaPathEnvVar(const QString& val) = 0;
    virtual void setJavaCustomPath(const QString& val) = 0;
    virtual void setGroupedViewFeatureActive(bool val) = 0;
    virtual void setRegexCompletion_CaseSensitive(const bool& val) = 0;
    virtual void setRegexCompletion_SearchPolicy(const bool& val) = 0;
    virtual void setUserName(const QString& val) = 0;
    virtual void setRegexInputFieldHeight(const int& linesNumber) = 0;

    /**
     * @brief setSelectedRegexFile - updates selected regex file
     * @param val - file name ( with extension ) of the file, which is located in the regex dir.
     * Path to regex dir is hard-coded at can't be changed as of now.
     */
    virtual void setSelectedRegexFile(const QString& val) = 0;

////////////////////////GETTERS/////////////////////////////

    // root settings
    virtual const tSettingsManagerVersion& getSettingsManagerVersion() const = 0;

    // regex settings
    virtual const tAliasItemMap& getAliases() const = 0;

    // regex usage statistics
    virtual const tRegexUsageStatisticsItemMap& getRegexUsageStatistics() const = 0;

    // general settings
    virtual const int& getNumberOfThreads() const = 0;
    virtual bool getContinuousSearch() const = 0;
    virtual bool getCopySearchResultAsHTML() const = 0;
    virtual bool getMinimizePatternsViewOnSelection()const = 0;
    virtual bool getWriteSettingsOnEachUpdate()const = 0;
    virtual bool getCacheEnabled() const = 0;
    virtual const tCacheSizeMB& getCacheMaxSizeMB() const = 0;
    virtual bool getRDPMode() const = 0;
    virtual const QColor& getRegexMonoHighlightingColor() const = 0;
    virtual bool getHighlightActivePatterns() const = 0;
    virtual const QColor& getPatternsHighlightingColor() const = 0;
    virtual bool getSearchResultMonoColorHighlighting() const = 0;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    virtual tHighlightingGradient getSearchResultHighlightingGradient() const = 0;
    virtual const tSearchResultColumnsVisibilityMap& getSearchResultColumnsVisibilityMap() const = 0;
    virtual const tSearchResultColumnsVisibilityMap& getSearchResultColumnsCopyPasteMap() const = 0;
    virtual const tSearchResultColumnsVisibilityMap& getSearchResultColumnsSearchMap() const = 0;
    virtual bool getMarkTimeStampWithBold() const = 0;
    virtual const tPatternsColumnsVisibilityMap& getPatternsColumnsVisibilityMap() const = 0;
    virtual const tPatternsColumnsVisibilityMap& getPatternsColumnsCopyPasteMap() const = 0;
    virtual bool getCaseSensitiveRegex() const = 0;
    virtual const tRegexFiltersColumnsVisibilityMap& getRegexFiltersColumnsVisibilityMap() const = 0;
    virtual bool getFilterVariables() const = 0;
    virtual QString getSelectedRegexFile() const = 0;
    virtual const tGroupedViewColumnsVisibilityMap& getGroupedViewColumnsVisibilityMap() const = 0;
    virtual const tGroupedViewColumnsVisibilityMap& getGroupedViewColumnsCopyPasteMap() const = 0;
    virtual bool getSubFilesHandlingStatus() const = 0;
    virtual const QFont& getFont_SearchView() const = 0;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    virtual const bool& getUML_FeatureActive() const = 0;
    virtual const int& getUML_MaxNumberOfRowsInDiagram() const = 0;
    virtual const bool& getUML_ShowArguments() const = 0;
    virtual const bool& getUML_WrapOutput() const = 0;
    virtual const bool& getUML_Autonumber() const = 0;
    virtual const bool& getPlotViewFeatureActive() const = 0;
    virtual const bool& getFiltersCompletion_CaseSensitive() const = 0;
    virtual const int& getFiltersCompletion_MaxNumberOfSuggestions() const = 0;
    virtual const int& getFiltersCompletion_MaxCharactersInSuggestion() const = 0;
    virtual const int& getFiltersCompletion_CompletionPopUpWidth() const = 0;
    virtual const bool& getFiltersCompletion_SearchPolicy() const = 0;
    virtual const int& getSearchViewLastColumnWidthStrategy() const = 0;
    virtual const int& getPlantumlPathMode() const = 0;
    virtual const QString& getPlantumlPathEnvVar() const = 0;
    virtual const QString& getPlantumlCustomPath() const = 0;
    virtual const int& getJavaPathMode() const = 0;
    virtual const QString& getJavaPathEnvVar() const = 0;
    virtual const QString& getJavaCustomPath() const = 0;
    virtual const bool& getGroupedViewFeatureActive() const = 0;
    virtual const bool& getRegexCompletion_CaseSensitive() const = 0;
    virtual const bool& getRegexCompletion_SearchPolicy() const = 0;
    virtual const QString& getUsername() const = 0;
    virtual const int& getRegexInputFieldHeight() const = 0;

    // allowed ranges
    virtual const TOptional<tRange<int>>& getSetting_NumberOfThreads_AllowedRange() const = 0;
    virtual const TOptional<tRange<tCacheSizeMB>>& getSetting_CacheMaxSizeMB_AllowedRange() const = 0;
    virtual const TOptional<tRange<int>>& getFiltersCompletion_MaxNumberOfSuggestions_AllowedRange() const = 0;
    virtual const TOptional<tRange<int>>& getFiltersCompletion_MaxCharactersInSuggestion_AllowedRange() const = 0;
    virtual const TOptional<tRange<int>>& getFiltersCompletion_CompletionPopUpWidth_AllowedRange() const = 0;

////////////////////////NOTIFICATIONS/////////////////////////////

signals:

    // root settings
    void settingsManagerVersionChanged( const tSettingsManagerVersion& settingsManagerVersion ) const;

    // regex settings
    void aliasesChanged( const tAliasItemMap& aliases );

    // regex usage statistics
    void regexUsageStatisticsChanged( const tRegexUsageStatisticsItemMap& regexUsageStatistics );

    // general settings
    void numberOfThreadsChanged( int numberOfThreads );
    void continuousSearchChanged( bool continuousSearch );
    void copySearchResultAsHTMLChanged( bool copySearchResultAsHTML );
    void minimizePatternsViewOnSelectionChanged( bool minimizePatternsViewOnSelection );
    void writeSettingsOnEachUpdateChanged( bool writeSettingsOnEachUpdate );
    void cacheEnabledChanged( bool cacheEnabled );
    void cacheMaxSizeMBChanged( const tCacheSizeMB& cacheMaxSizeMB );
    void RDPModeChanged( bool RDPMode );
    void regexMonoHighlightingColorChanged(const QColor& highlightingColor);
    void highlightActivePatternsChanged(bool highlightActivePatterns);
    void patternsHighlightingColorChanged(const QColor& patternsHighlightingColor);
    void searchResultMonoColorHighlightingChanged(bool searchResultMonoColorHighlighting);
    void searchResultHighlightingGradientChanged(const tHighlightingGradient& searchResultHighlightingGradient);
    void searchResultColumnsVisibilityMapChanged(const tSearchResultColumnsVisibilityMap& searchResultColumnsVisibilityMap);
    void searchResultColumnsCopyPasteMapChanged(const tSearchResultColumnsVisibilityMap& searchResultColumnsCopyPasteMap);
    void searchResultColumnsSearchMapChanged(const tSearchResultColumnsVisibilityMap& searchResultColumnsCopyPasteMap);
    void markTimeStampWithBoldChanged(bool markTimeStampWithBold);
    void patternsColumnsVisibilityMapChanged(const tPatternsColumnsVisibilityMap& patternsColumnsVisibilityMap);
    void patternsColumnsCopyPasteMapChanged(const tPatternsColumnsVisibilityMap& patternsColumnsCopyPasteMap);
    void caseSensitiveRegexChanged(bool bCaseSensitiveRegex);
    void regexFiltersColumnsVisibilityMapChanged(const tRegexFiltersColumnsVisibilityMap& regexFiltersColumnsVisibilityMap);
    void filterVariablesChanged(bool filterVariables);
    void selectedRegexFileChanged( const QString& regexDirectory );
    void groupedViewColumnsVisibilityMapChanged(const tGroupedViewColumnsVisibilityMap& groupedViewColumnsVisibilityMap);
    void groupedViewColumnsCopyPasteMapChanged(const tGroupedViewColumnsVisibilityMap& groupedViewColumnsCopyPasteMap);
    void subFilesHandlingStatusChanged(const bool& subFilesHandlingStatus);
    void font_SearchViewChanged(const QFont& font_SearchView);
    void UML_FeatureActiveChanged(const bool& UML_FeatureActive);
    void UML_MaxNumberOfRowsInDiagramChanged(const int& UML_MaxNumberOfRowsInDiagram);
    void UML_ShowArgumentsChanged(const bool& UML_ShowArguments);
    void UML_WrapOutputChanged(const bool& UML_WrapOutput);
    void UML_AutonumberChanged(const bool& UML_Autonumber);
    void plotViewFeatureActiveChanged(const bool& plotViewFeatureActive);
    void filtersCompletion_CaseSensitiveChanged(const bool& filtersCompletion_CaseSensitive);
    void filtersCompletion_MaxNumberOfSuggestionsChanged(const int& filtersCompletion_MaxNumberOfSuggestions);
    void filtersCompletion_MaxCharactersInSuggestionChanged(const int& filtersCompletion_MaxCharactersInSuggestion);
    void filtersCompletion_CompletionPopUpWidthChanged(const int& filtersCompletion_CompletionPopUpWidth);
    void filtersCompletion_SearchPolicyChanged(const bool& filtersCompletion_SearchPolicy);
    void searchViewLastColumnWidthStrategyChanged(const int& payloadWidthChanged);
    void plantumlPathModeChanged(const int& plantumlPathMode);
    void plantumlPathEnvVarChanged(const QString& plantumlPathEnvVar);
    void plantumlCustomPathChanged(const QString& plantumlPathEnvVar);
    void javaPathModeChanged(const int& plantumlPathMode);
    void javaPathEnvVarChanged(const QString& plantumlPathEnvVar);
    void javaCustomPathChanged(const QString& plantumlPathEnvVar);
    void groupedViewFeatureActiveChanged(const bool& groupedViewFeatureActive);
    void regexCompletion_CaseSensitiveChanged(const bool& regexCompletion_CaseSensitive);
    void regexCompletion_MaxNumberOfSuggestionsChanged(const int& regexCompletion_MaxNumberOfSuggestions);
    void regexCompletion_SearchPolicyChanged(const bool& regexCompletion_SearchPolicy);
    void usernameChanged(const QString& username);
    void regexInputFieldHeightChanged(const int& linesNumber);
};
