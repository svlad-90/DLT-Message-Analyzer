/**
 * @file    CSettingsManager.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSettingsManager class
 * Class is not thread-safe!
 */
#ifndef CSETTINGSMANAGER_HPP
#define CSETTINGSMANAGER_HPP

#include "memory"
#include <mutex>

#include "QObject"
#include "QVector"
#include "QPair"
#include "QColor"
#include "QVariant"
#include "QFont"

#include "../common/Definitions.hpp"
#include "../common/cpp_extensions.hpp"
#include "TSettingItem.hpp"

class CSettingsManager;
typedef std::shared_ptr<CSettingsManager> tSettingsManagerPtr;

class CSettingsManager : public QObject
{
    Q_OBJECT

public:

    static tSettingsManagerPtr getInstance();

    struct tOperationResult
    {
        bool bResult = false;
        QString err;
    };

    /**
     * @brief storeConfigs - stores all types of configuration ( root, settings, regex-es ) from memory to JSON files
     */
    tOperationResult storeConfigs();

    /**
     * @brief loadConfigs - loads all types of configuration ( root, settings, regex-es ) from JSON files to memory
     */
    tOperationResult loadConfigs();

    struct tAliasItem
    {
        tAliasItem();
        tAliasItem(bool isDefault_, const QString& alias_, const QString& regex_);
        bool operator==(const tAliasItem&) const;
        bool isDefault;
        QString alias;
        QString regex;
    };
    typedef QVector<tAliasItem> tAliasItemVec;

    //helpers
    bool areAnyDefaultAliasesAvailable() const;
    void resetSearchResultColumnsVisibilityMap();
    void resetSearchResultColumnsCopyPasteMap();
    void resetPatternsColumnsVisibilityMap();
    void resetPatternsColumnsCopyPasteMap();
    void resetRegexFiltersColumnsVisibilityMap();
    void resetGroupedViewColumnsVisibilityMap();
    void resetGroupedViewColumnsCopyPasteMap();
    QString getRegexDirectory() const;
    QString getRegexDirectoryFull() const;
    QString getSettingsFilepath() const;
    QString getUserSettingsFilepath() const;
    QString getRootSettingsFilepath() const;

////////////////////////SETTERS/////////////////////////////

    // root settings
    void setSettingsManagerVersion(const tSettingsManagerVersion& val);

    // regex settings
    void setAliases(const tAliasItemVec& val);
    void setAliasIsDefault(const QString& alias, bool isDefault);

    // general settings
    void setNumberOfThreads(const int& val);
    void setContinuousSearch(bool val);
    void setCopySearchResultAsHTML(bool val);
    void setMinimizePatternsViewOnSelection(bool val);
    void setWriteSettingsOnEachUpdate(bool val);
    void setCacheEnabled(bool val);
    void setCacheMaxSizeMB(tCacheSizeMB val);
    void setRDPMode(bool val);
    void setRegexMonoHighlightingColor(const QColor& val);
    void setHighlightActivePatterns(bool val);
    void setPatternsHighlightingColor(const QColor& val);
    void setSearchResultMonoColorHighlighting(bool val);
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    void setSearchResultHighlightingGradient(const tHighlightingGradient& val);
    void setSearchResultColumnsVisibilityMap(const tSearchResultColumnsVisibilityMap& val);
    void setSearchResultColumnsCopyPasteMap(const tSearchResultColumnsVisibilityMap& val);
    void setMarkTimeStampWithBold(bool val);
    void setPatternsColumnsVisibilityMap(const tPatternsColumnsVisibilityMap& val);
    void setPatternsColumnsCopyPasteMap(const tPatternsColumnsVisibilityMap& val);
    void setCaseSensitiveRegex(bool val);
    void setRegexFiltersColumnsVisibilityMap(const tRegexFiltersColumnsVisibilityMap& val);
    void setFilterVariables(bool val);
    void setGroupedViewColumnsVisibilityMap(const tGroupedViewColumnsVisibilityMap& val);
    void setGroupedViewColumnsCopyPasteMap(const tGroupedViewColumnsVisibilityMap& val);
    void setSubFilesHandlingStatus( const bool& val );
    void setFont_SearchView( const QFont& val );
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    void setUML_FeatureActive(const bool& val);
    void setUML_MaxNumberOfRowsInDiagram(const int& val);
    void setUML_ShowArguments(const bool& val);
    void setUML_WrapOutput(const bool& val);
    void setUML_Autonumber(const bool& val);

    /**
     * @brief setSelectedRegexFile - updates selected regex file
     * @param val - file name ( with extension ) of the file, which is located in the regex dir.
     * Path to regex dir is hard-coded at can't be changed as of now.
     */
    void setSelectedRegexFile(const QString& val);

////////////////////////GETTERS/////////////////////////////

    // root settings
    const tSettingsManagerVersion& getSettingsManagerVersion() const;

    // regex settings
    const tAliasItemVec& getAliases() const;

    // general settings
    const int& getNumberOfThreads() const;
    bool getContinuousSearch() const;
    bool getCopySearchResultAsHTML() const;
    bool getMinimizePatternsViewOnSelection()const;
    bool getWriteSettingsOnEachUpdate()const;
    bool getCacheEnabled() const;
    const tCacheSizeMB& getCacheMaxSizeMB() const;
    bool getRDPMode() const;
    const QColor& getRegexMonoHighlightingColor() const;
    bool getHighlightActivePatterns() const;
    const QColor& getPatternsHighlightingColor() const;
    bool getSearchResultMonoColorHighlighting() const;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    tHighlightingGradient getSearchResultHighlightingGradient() const;
    const tSearchResultColumnsVisibilityMap& getSearchResultColumnsVisibilityMap() const;
    const tSearchResultColumnsVisibilityMap& getSearchResultColumnsCopyPasteMap() const;
    bool getMarkTimeStampWithBold() const;
    const tPatternsColumnsVisibilityMap& getPatternsColumnsVisibilityMap() const;
    const tPatternsColumnsVisibilityMap& getPatternsColumnsCopyPasteMap() const;
    bool getCaseSensitiveRegex() const;
    const tRegexFiltersColumnsVisibilityMap& getRegexFiltersColumnsVisibilityMap() const;
    bool getFilterVariables() const;
    QString getSelectedRegexFile() const;
    const tGroupedViewColumnsVisibilityMap& getGroupedViewColumnsVisibilityMap() const;
    const tGroupedViewColumnsVisibilityMap& getGroupedViewColumnsCopyPasteMap() const;
    bool getSubFilesHandlingStatus() const;
    const QFont& getFont_SearchView() const;
    // this method can be called from context of multiple threads, thus it is designed as thread-safe
    const bool& getUML_FeatureActive() const;
    const int& getUML_MaxNumberOfRowsInDiagram() const;
    const bool& getUML_ShowArguments() const;
    const bool& getUML_WrapOutput() const;
    const bool& getUML_Autonumber() const;

////////////////////////NOTIFICATIONS/////////////////////////////

signals:

    // root settings
    void settingsManagerVersionChanged( const tSettingsManagerVersion& settingsManagerVersion ) const;

    // regex settings
    void aliasesChanged( const tAliasItemVec& aliases );

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

private: // methods

    /**
     * @brief CSettingsManager - default constructor.
     * Private, since this is a single-tone class.
     */
    CSettingsManager();

    /**
     * @brief setUp - this method is using other methods of this class in order to set it up for further usage.
     * Called from constructor of instance of this single-tone class
     * @return - result of the operation
     */
    tOperationResult setUp();

    ////////////////BACKWARD_COMPATIBILITY//////////////////////////////////////////////

    /**
     * @brief backwardCompatibility - calls all versions of backward compatibility functions
     * @return - result of the operation
     */
    tOperationResult backwardCompatibility();

    /**
     * @brief backwardCompatibility_V0_V1 - function, which handles backward compatibility between V0 and V1 of settings manager
     * @return - result of the operation
     */
    tOperationResult backwardCompatibility_V0_V1();

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
     */
    void clearRegexConfig();

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

    TSettingItem<tAliasItemVec> createAliasItemVecSettingsItem(const QString& key,
                                                 const TSettingItem<tAliasItemVec>::tUpdateDataFunc& updateDataFunc,
                                                 const TSettingItem<tAliasItemVec>::tUpdateSettingsFileFunc& updateFileFunc,
                                                 const tAliasItemVec& defaultValue) const;

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
    TSettingItem<tAliasItemVec> mSetting_Aliases;
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

    typedef ISettingItem* tSettingItemPtr;
    typedef std::vector<tSettingItemPtr> tSettingItemsPtrVec;

    tSettingItemsPtrVec mRootSettingItemPtrVec;
    tSettingItemsPtrVec mUserSettingItemPtrVec;
    tSettingItemsPtrVec mPatternsSettingItemPtrVec;

    bool mbRootConfigInitialised;
};

#endif // CSETTINGSMANAGER_HPP
