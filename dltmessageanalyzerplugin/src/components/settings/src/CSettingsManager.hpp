/**
 * @file    CSettingsManager.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSettingsManager class
 * Class is not thread-safe!
 */
#pragma once

#include "memory"
#include <mutex>

#include "../api/ISettingsManager.hpp"
#include "TSettingItem.hpp"

class CSettingsManager : public ISettingsManager
{
    Q_OBJECT

public:

    /**
     * @brief CSettingsManager - default constructor.
     */
    CSettingsManager();

    tOperationResult storeConfigs() override;
    tOperationResult loadConfigs() override;

    //helpers
    bool areAnyDefaultAliasesAvailable() const override;
    void resetSearchResultColumnsVisibilityMap() override;
    void resetSearchResultColumnsCopyPasteMap() override;
    void resetSearchResultColumnsSearchMap() override;
    void resetPatternsColumnsVisibilityMap() override;
    void resetPatternsColumnsCopyPasteMap() override;
    void resetRegexFiltersColumnsVisibilityMap() override;
    void resetGroupedViewColumnsVisibilityMap() override;
    void resetGroupedViewColumnsCopyPasteMap() override;
    QString getRegexDirectory() const override;
    QString getRegexUsageStatisticsDirectory() const override;
    QString getSettingsFilepath() const override;
    QString getUserSettingsFilepath() const override;
    QString getRootSettingsFilepath() const override;
    QString getDefaultPlantumlPath() const override;
    QString getDefaultJavaPath() const override;
    void refreshRegexConfiguration() override;

////////////////////////SETTERS/////////////////////////////

    // root settings
    void setSettingsManagerVersion(const tSettingsManagerVersion& val) override;

    // regex settings
    void setAliases(const tAliasItemMap& val) override;
    void setAliasIsDefault(const QString& alias, bool isDefault) override;

    // regex usage statistics
    virtual void setRegexUsageStatistics(const tRegexUsageStatisticsItemMap& val) override;

    // general settings
    void setNumberOfThreads(const int& val) override;
    void setContinuousSearch(bool val) override;
    void setCopySearchResultAsHTML(bool val) override;
    void setMinimizePatternsViewOnSelection(bool val) override;
    void setWriteSettingsOnEachUpdate(bool val) override;
    void setCacheEnabled(bool val) override;
    void setCacheMaxSizeMB(tCacheSizeMB val) override;
    void setRDPMode(bool val) override;
    void setRegexMonoHighlightingColor(const QColor& val) override;
    void setHighlightActivePatterns(bool val) override;
    void setPatternsHighlightingColor(const QColor& val) override;
    void setSearchResultMonoColorHighlighting(bool val) override;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    void setSearchResultHighlightingGradient(const tHighlightingGradient& val) override;
    void setSearchResultColumnsVisibilityMap(const tSearchResultColumnsVisibilityMap& val) override;
    void setSearchResultColumnsCopyPasteMap(const tSearchResultColumnsVisibilityMap& val) override;
    void setSearchResultColumnsSearchMap(const tSearchResultColumnsVisibilityMap& val) override;
    void setMarkTimeStampWithBold(bool val) override;
    void setPatternsColumnsVisibilityMap(const tPatternsColumnsVisibilityMap& val) override;
    void setPatternsColumnsCopyPasteMap(const tPatternsColumnsVisibilityMap& val) override;
    void setCaseSensitiveRegex(bool val) override;
    void setRegexFiltersColumnsVisibilityMap(const tRegexFiltersColumnsVisibilityMap& val) override;
    void setFilterVariables(bool val) override;
    void setGroupedViewColumnsVisibilityMap(const tGroupedViewColumnsVisibilityMap& val) override;
    void setGroupedViewColumnsCopyPasteMap(const tGroupedViewColumnsVisibilityMap& val) override;
    void setSubFilesHandlingStatus( const bool& val ) override;
    void setFont_SearchView( const QFont& val ) override;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    void setUML_FeatureActive(const bool& val) override;
    void setUML_MaxNumberOfRowsInDiagram(const int& val) override;
    void setUML_ShowArguments(const bool& val) override;
    void setUML_WrapOutput(const bool& val) override;
    void setUML_Autonumber(const bool& val) override;
    void setPlotViewFeatureActive(const bool& val) override;
    void setFiltersCompletion_CaseSensitive(const bool& val) override;
    void setFiltersCompletion_MaxNumberOfSuggestions(const int& val) override;
    void setFiltersCompletion_MaxCharactersInSuggestion(const int& val) override;
    void setFiltersCompletion_CompletionPopUpWidth(const int& val) override;
    void setFiltersCompletion_SearchPolicy(const bool& val) override;
    void setSearchViewLastColumnWidthStrategy(const int& val) override;
    void setPlantumlPathMode(const int& val) override;
    void setPlantumlPathEnvVar(const QString& val) override;
    void setPlantumlCustomPath(const QString& val) override;
    void setJavaPathMode(const int& val) override;
    void setJavaPathEnvVar(const QString& val) override;
    void setJavaCustomPath(const QString& val) override;
    void setGroupedViewFeatureActive(bool val) override;
    void setRegexCompletion_CaseSensitive(const bool& val) override;
    void setRegexCompletion_SearchPolicy(const bool& val) override;
    void setUserName(const QString& val) override;
    void setRegexInputFieldHeight(const int& linesNumber) override;

    void setSelectedRegexFile(const QString& val) override;

////////////////////////GETTERS/////////////////////////////

    // root settings
    const tSettingsManagerVersion& getSettingsManagerVersion() const override;

    // regex settings
    const tAliasItemMap& getAliases() const override;

    // regex usage statistics
    const tRegexUsageStatisticsItemMap& getRegexUsageStatistics() const override;

    // general settings
    const int& getNumberOfThreads() const override;
    bool getContinuousSearch() const override;
    bool getCopySearchResultAsHTML() const override;
    bool getMinimizePatternsViewOnSelection()const override;
    bool getWriteSettingsOnEachUpdate()const override;
    bool getCacheEnabled() const override;
    const tCacheSizeMB& getCacheMaxSizeMB() const override;
    bool getRDPMode() const override;
    const QColor& getRegexMonoHighlightingColor() const override;
    bool getHighlightActivePatterns() const override;
    const QColor& getPatternsHighlightingColor() const override;
    bool getSearchResultMonoColorHighlighting() const override;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    tHighlightingGradient getSearchResultHighlightingGradient() const override;
    const tSearchResultColumnsVisibilityMap& getSearchResultColumnsVisibilityMap() const override;
    const tSearchResultColumnsVisibilityMap& getSearchResultColumnsCopyPasteMap() const override;
    const tSearchResultColumnsVisibilityMap& getSearchResultColumnsSearchMap() const override;
    bool getMarkTimeStampWithBold() const override;
    const tPatternsColumnsVisibilityMap& getPatternsColumnsVisibilityMap() const override;
    const tPatternsColumnsVisibilityMap& getPatternsColumnsCopyPasteMap() const override;
    bool getCaseSensitiveRegex() const override;
    const tRegexFiltersColumnsVisibilityMap& getRegexFiltersColumnsVisibilityMap() const override;
    bool getFilterVariables() const override;
    QString getSelectedRegexFile() const override;
    const tGroupedViewColumnsVisibilityMap& getGroupedViewColumnsVisibilityMap() const override;
    const tGroupedViewColumnsVisibilityMap& getGroupedViewColumnsCopyPasteMap() const override;
    bool getSubFilesHandlingStatus() const override;
    const QFont& getFont_SearchView() const override;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    const bool& getUML_FeatureActive() const override;
    const int& getUML_MaxNumberOfRowsInDiagram() const override;
    const bool& getUML_ShowArguments() const override;
    const bool& getUML_WrapOutput() const override;
    const bool& getUML_Autonumber() const override;
    const bool& getPlotViewFeatureActive() const override;
    const bool& getFiltersCompletion_CaseSensitive() const override;
    const int& getFiltersCompletion_MaxNumberOfSuggestions() const override;
    const int& getFiltersCompletion_MaxCharactersInSuggestion() const override;
    const int& getFiltersCompletion_CompletionPopUpWidth() const override;
    const bool& getFiltersCompletion_SearchPolicy() const override;
    const int& getSearchViewLastColumnWidthStrategy() const override;
    const int& getPlantumlPathMode() const override;
    const QString& getPlantumlPathEnvVar() const override;
    const QString& getPlantumlCustomPath() const override;
    const int& getJavaPathMode() const override;
    const QString& getJavaPathEnvVar() const override;
    const QString& getJavaCustomPath() const override;
    const bool& getGroupedViewFeatureActive() const override;
    const bool& getRegexCompletion_CaseSensitive() const override;
    const bool& getRegexCompletion_SearchPolicy() const override;
    const QString& getUsername() const override;
    const int& getRegexInputFieldHeight() const override;

    // allowed ranges
    const TRangedSettingItem<int>::tOptionalAllowedRange& getSetting_NumberOfThreads_AllowedRange() const override;
    const TRangedSettingItem<tCacheSizeMB>::tOptionalAllowedRange& getSetting_CacheMaxSizeMB_AllowedRange() const override;
    const TRangedSettingItem<int>::tOptionalAllowedRange& getFiltersCompletion_MaxNumberOfSuggestions_AllowedRange() const override;
    const TRangedSettingItem<int>::tOptionalAllowedRange& getFiltersCompletion_MaxCharactersInSuggestion_AllowedRange() const override;
    const TRangedSettingItem<int>::tOptionalAllowedRange& getFiltersCompletion_CompletionPopUpWidth_AllowedRange() const override;

    tOperationResult setUp() override;

private: // methods

    ////////////////BACKWARD_COMPATIBILITY//////////////////////////////////////////////

    /**
     * @brief backwardCompatibility - calls all versions of backward compatibility functions
     * @return - result of the operation
     */
    tOperationResult backwardCompatibility();

    /**
     * @brief backwardCompatibility_V0_V1 - function,
     * which handles backward compatibility between V0 and V1 of settings manager
     * @return - result of the operation
     */
    tOperationResult backwardCompatibility_V0_V1();

    /**
     * @brief backwardCompatibility_V1_V2 - function,
     * which handles backward compatibility between V1 and V2 of settings manager
     * @return - result of the operation
     */
    tOperationResult backwardCompatibility_V1_V2();

    ////////////////ROOT_CONFIG//////////////////////////////////////////////

    /**
     * @brief loadRootConfig - loads root configuration, which contains version of used settings manager and most general values
     * @return - result of the operation
     */
    tOperationResult loadRootConfig();

    /**
     * @brief storeRootConfig - stores root configuration, which contains version of used settings manager and most general values
     * @return - result of the operation
     */
    tOperationResult storeRootConfig();

    ////////////////SETTINGS_CONFIG//////////////////////////////////////////////

    /**
     * @brief storeSettingsConfig - stores config to a predefined file ( path is hard-coded )
     * @return - result of the operation
     */
    tOperationResult storeSettingsConfig();

    /**
     * @brief loadSettingsConfig - loads general configuration from a file ( path is hard-coded )
     * @return - result of the operation
     */
    tOperationResult loadSettingsConfig();

    /**
     * @brief storeSettingsConfigCustomPath - stores config to a specified file
     * @param filepath - filepath to a config file
     * @return - result of the operation
     */
    tOperationResult storeSettingsConfigCustomPath(const QString& filepath);

    /**
     * @brief loadSettingsConfigCustomPath - loads general configuration from a specified file
     * @param filepath - path to a config file
     * @return - result of the operation
     */
    tOperationResult loadSettingsConfigCustomPath(const QString& filepath);

    ////////////////REGEX_CONFIG//////////////////////////////////////////////

    /**
     * @brief storeRegexConfigCustomPath - stores regex config to a specified config file
     * @return - result of the operation
     */
    tOperationResult storeRegexConfigCustomPath( const QString& filePath ) const;

    /**
     * @brief loadRegexConfigCustomPath - loads regex config from a specified file
     * @param filePath - path to a file with configuration
     * @return - result of the operation
     */
    tOperationResult loadRegexConfigCustomPath( const QString& filePath );

    /**
     * @brief clearRegexConfig - clears regex configuration
     * that is stored in the RAM
     */
    void clearRegexConfig();

    ////////////////REGEX_COMPLETION_DATA/////////////////////////////////////

    /**
     * @brief storeRegexUsageStatisticsDataCustomPath - stores regex
     * usage statistics to a specified config file
     * @return - result of the operation
     */
    tOperationResult storeRegexUsageStatisticsDataCustomPath( const QString& filePath ) const;

    /**
     * @brief loadaRegexUsageStatisticsDataCustomPath - loads regex
     * usage statistics from a specified config file
     * @param filePath - path to a file with configuration
     * @return - result of the operation
     */
    tOperationResult loadRegexUsageStatisticsDataCustomPath( const QString& filePath );

    /**
     * @brief clearRegexUsageStatisticsData - clears regex usage
     * statistics that is stored in the RAM
     */
    void clearRegexUsageStatisticsData();

private: // methods

    /**
     * @brief tryStoreSettingsConfig - checks, whether it is needed to store the non-root settings.
     * If needed - stores the data to file.
     */
    void tryStoreSettingsConfig();

    /**
     * @brief tryStoreRootConfig - checks, whether it is needed to store the root settings.
     * If needed - stores the data to file.
     */
    void tryStoreRootConfig();

    TSettingItem<bool> createBooleanSettingsItem(const QString& key,
                                                 const TSettingItem<bool>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<bool>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const bool& defaultValue) const;

    TSettingItem<QColor> createColorSettingsItem(const QString& key,
                                                 const TSettingItem<QColor>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<QColor>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const QColor& defaultValue) const;

    TSettingItem<tHighlightingGradient> createHighlightingGradientSettingsItem(const QString& key,
                                                 const TSettingItem<tHighlightingGradient>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tHighlightingGradient>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tHighlightingGradient& defaultValue) const;

    TSettingItem<tAliasItemMap> createAliasItemMapSettingsItem(const QString& key,
                                                 const TSettingItem<tAliasItemMap>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tAliasItemMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tAliasItemMap& defaultValue) const;

    TSettingItem<tRegexUsageStatisticsItemMap> createRegexUsageStatisticsItemMapSettingsItem(const QString& key,
                                                 const TSettingItem<tRegexUsageStatisticsItemMap>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tRegexUsageStatisticsItemMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tRegexUsageStatisticsItemMap& defaultValue) const;

    TSettingItem<tSearchResultColumnsVisibilityMap> createSearchResultColumnsVisibilityMapSettingsItem(const QString& key,
                                                 const TSettingItem<tSearchResultColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tSearchResultColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tSearchResultColumnsVisibilityMap& defaultValue) const;

    TSettingItem<tPatternsColumnsVisibilityMap> createPatternsColumnsVisibilityMapSettingsItem(const QString& key,
                                                 const TSettingItem<tPatternsColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tPatternsColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tPatternsColumnsVisibilityMap& defaultValue) const;

    TSettingItem<tGroupedViewColumnsVisibilityMap> createGroupedViewColumnsVisibilityMapSettingsItem(const QString& key,
                                                 const TSettingItem<tGroupedViewColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tGroupedViewColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tGroupedViewColumnsVisibilityMap& defaultValue) const;

    TSettingItem<tRegexFiltersColumnsVisibilityMap> createRegexFiltersColumnsVisibilityMapSettingsItem(const QString& key,
                                                 const TSettingItem<tRegexFiltersColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tRegexFiltersColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tRegexFiltersColumnsVisibilityMap& defaultValue) const;

    TSettingItem<QString> createStringSettingsItem(const QString& key,
                                                 const TSettingItem<QString>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<QString>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const QString& defaultValue) const;

    TSettingItem<QFont> createFontSettingsItem(const QString& key,
                                                 const TSettingItem<QFont>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<QFont>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const QFont& defaultValue) const;

    template<typename T, typename = cpp_14::enable_if_t<std::is_arithmetic<T>::value>>
    TSettingItem<T> createArithmeticSettingsItem(const QString& key,
                                                 const typename TSettingItem<T>::tUpdateDataFunc& updateDataFunc,
                                                 const typename TSettingItem<T>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const T& defaultValue) const
    {
        auto writeFunc = [&key](const T& value)->QJsonObject
        {
            QJsonObject result;
            result.insert( key, QJsonValue( static_cast<double>(value) ) );
            return result;
        };

        auto readFunc = [](const QJsonValueRef& JSONItem,
                           T& data,
                           const T&)->bool
        {
            bool bResult = false;

            if(true == JSONItem.isDouble())
            {
                data = static_cast<T>(JSONItem.toDouble());
                bResult = true;
            }

            return bResult;
        };

        return TSettingItem<T>(key,
                               defaultValue,
                               writeFunc,
                               readFunc,
                               updateDataFunc,
                               updateFileFunc);
    }

    template<typename T, typename = cpp_14::enable_if_t<std::is_arithmetic<T>::value>>
    TRangedSettingItem<T> createRangedArithmeticSettingsItem(const QString& key,
                                                 const typename TRangedSettingItem<T>::tUpdateDataFunc& updateDataFunc,
                                                 const typename TRangedSettingItem<T>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const typename TRangedSettingItem<T>::tOptionalAllowedRange& allowedRange,
                                                 const T& defaultValue) const
    {
        auto writeFunc = [&key](const T& value)->QJsonObject
        {
            QJsonObject result;
            result.insert( key, QJsonValue( static_cast<double>(value) ) );
            return result;
        };

        auto readFunc = [](const QJsonValueRef& JSONItem,
                           T& data,
                           const T&)->bool
        {
            bool bResult = false;

            if(true == JSONItem.isDouble())
            {
                data = static_cast<T>(JSONItem.toDouble());
                bResult = true;
            }

            return bResult;
        };

        return TRangedSettingItem<T>(key,
                               defaultValue,
                               allowedRange,
                               writeFunc,
                               readFunc,
                               updateDataFunc,
                               updateFileFunc);
    }

private: // fields

    TSettingItem<tSettingsManagerVersion> mSetting_SettingsManagerVersion;
    TSettingItem<tAliasItemMap> mSetting_Aliases;
    TRangedSettingItem<int> mSetting_NumberOfThreads;
    TSettingItem<bool> mSetting_ContinuousSearch;
    TSettingItem<bool> mSetting_CopySearchResultAsHTML;
    TSettingItem<bool> mSetting_MinimizePatternsViewOnSelection;
    TSettingItem<bool> mSetting_WriteSettingsOnEachUpdate;
    TSettingItem<bool> mSetting_CacheEnabled;
    TRangedSettingItem<tCacheSizeMB> mSetting_CacheMaxSizeMB;
    TSettingItem<bool> mSetting_RDPMode;
    TSettingItem<QColor> mSetting_RegexMonoHighlightingColor;
    TSettingItem<bool> mSetting_HighlightActivePatterns;
    TSettingItem<QColor> mSetting_PatternsHighlightingColor;
    TSettingItem<bool> mSetting_SearchResultMonoColorHighlighting;

    std::recursive_mutex mSearchResultHighlightingGradientProtector;
    TSettingItem<tHighlightingGradient> mSetting_SearchResultHighlightingGradient;

    TSettingItem<tSearchResultColumnsVisibilityMap> mSetting_SearchResultColumnsVisibilityMap;
    TSettingItem<tSearchResultColumnsVisibilityMap> mSetting_SearchResultColumnsCopyPasteMap;
    TSettingItem<tSearchResultColumnsVisibilityMap> mSetting_SearchResultColumnsSearchMap;
    TSettingItem<bool> mSetting_MarkTimeStampWithBold;
    TSettingItem<tPatternsColumnsVisibilityMap> mSetting_PatternsColumnsVisibilityMap;
    TSettingItem<tPatternsColumnsVisibilityMap> mSetting_PatternsColumnsCopyPasteMap;
    TSettingItem<bool> mSetting_CaseSensitiveRegex;
    TSettingItem<tRegexFiltersColumnsVisibilityMap> mSetting_RegexFiltersColumnsVisibilityMap;
    TSettingItem<bool> mSetting_FilterVariables;
    TSettingItem<QString> mSetting_SelectedRegexFile; // name of the regex file to be used.
    TSettingItem<tGroupedViewColumnsVisibilityMap> mSetting_GroupedViewColumnsVisibilityMap;
    TSettingItem<tGroupedViewColumnsVisibilityMap> mSetting_GroupedViewColumnsCopyPasteMap;
    TSettingItem<bool> mSetting_SubFilesHandlingStatus;

    // font settings
    TSettingItem<QFont> mSetting_Font_SearchView;

    // UML settings
    std::recursive_mutex mUML_FeatureActiveProtector;
    TSettingItem<bool> mSetting_UML_FeatureActive;
    TSettingItem<int> mSetting_UML_MaxNumberOfRowsInDiagram;
    TSettingItem<bool> mSetting_UML_ShowArguments;
    TSettingItem<bool> mSetting_UML_WrapOutput;
    TSettingItem<bool> mSetting_UML_Autonumber;

    // Plot view settings
    std::recursive_mutex mPlotViewFeatureActiveProtector;
    TSettingItem<bool> mSetting_PlotViewFeatureActive;

    // Filters view completion settings
    TSettingItem<bool> mSetting_FiltersCompletion_CaseSensitive;
    TRangedSettingItem<int> mSetting_FiltersCompletion_MaxNumberOfSuggestions;
    TRangedSettingItem<int> mSetting_FiltersCompletion_MaxCharactersInSuggestion;
    TRangedSettingItem<int> mSetting_FiltersCompletion_CompletionPopUpWidth;
    TSettingItem<bool> mSetting_FiltersCompletion_SearchPolicy; // 0 - startWith; 1 - contains

    // Regex completion settings
    TSettingItem<bool> mSetting_RegexCompletion_CaseSensitive;
    TSettingItem<bool> mSetting_RegexCompletion_SearchPolicy; // 0 - startWith; 1 - contains

    TRangedSettingItem<int> mSetting_SearchViewLastColumnWidthStrategy;

    // Plantuml path settings
    TRangedSettingItem<int> mSetting_PlantumlPathMode;
    TSettingItem<QString> mSetting_PlantumlPathEnvVar;
    TSettingItem<QString> mSetting_PlantumlCustomPath;

    // Java path settings
    TRangedSettingItem<int> mSetting_JavaPathMode;
    TSettingItem<QString> mSetting_JavaPathEnvVar;
    TSettingItem<QString> mSetting_JavaCustomPath;

    // Grouped view settings
    TSettingItem<bool> mSetting_GroupedViewFeatureActive;

    // Regex usage statistics
    TSettingItem<tRegexUsageStatisticsItemMap> mSetting_RegexUsageStatistics;

    TSettingItem<QString> mSetting_Username;
    TSettingItem<int> mSetting_RegexInputFieldHeight;

    typedef ISettingItem* tSettingItemPtr;
    typedef std::vector<tSettingItemPtr> tSettingItemsPtrVec;

    tSettingItemsPtrVec mRootSettingItemPtrVec;
    tSettingItemsPtrVec mUserSettingItemPtrVec;
    tSettingItemsPtrVec mPatternsSettingItemPtrVec;
    tSettingItemsPtrVec mRegexUsageStatisticsDataItemPtrVec;

    bool mbInitialised;
};
