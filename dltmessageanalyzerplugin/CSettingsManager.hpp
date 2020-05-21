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

#include "Definitions.hpp"

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
    void resetPatternsColumnsVisibilityMap();
    void resetRegexFiltersColumnsVisibilityMap();
    QString getRegexDirectory() const;
    QString getRegexDirectoryFull() const;
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
    void setMarkTimeStampWithBold(bool val);
    void setPatternsColumnsVisibilityMap(const tPatternsColumnsVisibilityMap& val);
    void setCaseSensitiveRegex(bool val);
    void setRegexFiltersColumnsVisibilityMap(const tRegexFiltersColumnsVisibilityMap& val);
    void setFilterVariables(bool val);

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
    bool getMarkTimeStampWithBold() const;
    const tPatternsColumnsVisibilityMap& getPatternsColumnsVisibilityMap() const;
    bool getCaseSensitiveRegex() const;
    const tRegexFiltersColumnsVisibilityMap& getRegexFiltersColumnsVisibilityMap() const;
    bool getFilterVariables() const;
    QString getSelectedRegexFile() const;

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
    void markTimeStampWithBoldChanged(bool markTimeStampWithBold);
    void patternsColumnsVisibilityMapChanged(const tPatternsColumnsVisibilityMap& patternsColumnsVisibilityMap);
    void caseSensitiveRegexChanged(bool bCaseSensitiveRegex);
    void regexFiltersColumnsVisibilityMapChanged(const tRegexFiltersColumnsVisibilityMap& regexFiltersColumnsVisibilityMap);
    void filterVariablesChanged(bool filterVariables);
    void selectedRegexFileChanged( const QString& regexDirectory );

private: // methods

    /**
     * @brief CSettingsManager - default constructor.
     * Private, since this is a single-tone class.
     */
    CSettingsManager();

    /**
     * @brief setUp - this method is using other methods of this class in order to set it up for further usage.
     * Called from constructor of instance of this single-tone class
     */
    tOperationResult setUp();

    ////////////////BACKWARD_COMPATIBILITY//////////////////////////////////////////////

    /**
     * @brief backwardCompatibility - calls all versions of backward compatibility functions
     */
    tOperationResult backwardCompatibility();

    /**
     * @brief backwardCompatibility_V0_V1 - function, which handles backward compatibility between V0 and V1 of settings manager
     */
    tOperationResult backwardCompatibility_V0_V1();

    ////////////////ROOT_CONFIG//////////////////////////////////////////////

    /**
     * @brief loadRootConfig - loads root configuration, which contains version of used settings manager and most general values
     */
    tOperationResult loadRootConfig();

    /**
     * @brief storeRootConfig - stores root configuration, which contains version of used settings manager and most general values
     */
    tOperationResult storeRootConfig();

    ////////////////SETTINGS_CONFIG//////////////////////////////////////////////

    /**
     * @brief storeSettingsConfig - stores config to a predefined file ( path is hard-coded )
     * @param path - path to a config file
     * @return - true on success, false otherwise
     */
    tOperationResult storeSettingsConfig();

    /**
     * @brief loadSettingsConfig - loads general configuration from a file ( path is hard-coded )
     * @param path - path to a config file
     * @return - true on success, false otherwise
     */
    tOperationResult loadSettingsConfig();

    /**
     * @brief storeSettingsConfigCustomPath - stores config to a specified file
     * @param path - path to a config file
     * @return - true on success, false otherwise
     */
    tOperationResult storeSettingsConfigCustomPath(const QString& filepath);

    /**
     * @brief loadSettingsConfigCustomPath - loads general configuration from a specified file
     * @param path - path to a config file
     * @return - true on success, false otherwise
     */
    tOperationResult loadSettingsConfigCustomPath(const QString& filepath);

    ////////////////REGEX_CONFIG//////////////////////////////////////////////

    /**
     * @brief storeRegexConfigCustomPath - stores regex config to a specified config file
     * @return - true on success, false otherwise
     */
    tOperationResult storeRegexConfigCustomPath( const QString& filePath ) const;

    /**
     * @brief loadRegexConfigCustomPath - loads regex config from a specified file
     * @param filePath - path to a file with configuration
     * @return - true on success, false otherwise
     */
    tOperationResult loadRegexConfigCustomPath( const QString& filePath );

    /**
     * @brief clearRegexConfig - clears regex configuration
     */
    void clearRegexConfig();

private: // fields

    tSettingsManagerVersion mSettingsManagerVersion;
    tAliasItemVec mAliases;
    int mNumberOfThreads;
    bool mbContinuousSearch;
    bool mbCopySearchResultAsHTML;
    bool mbMinimizePatternsViewOnSelection;
    bool mbWriteSettingsOnEachUpdate;
    bool mbCacheEnabled;
    tCacheSizeMB mCacheMaxSizeMB;
    bool mbRDPMode;
    QColor mRegexMonoHighlightingColor;
    bool mbHighlightActivePatterns;
    QColor mPatternsHighlightingColor;
    bool mbSearchResultMonoColorHighlighting;
    tHighlightingGradient mSearchResultHighlightingGradient;
    std::mutex mSearchResultHighlightingGradientProtector;
    tSearchResultColumnsVisibilityMap mSearchResultColumnsVisibilityMap;
    bool mbMarkTimeStampWithBold;
    tPatternsColumnsVisibilityMap mPatternsColumnsVisibilityMap;
    bool mbCaseSensitiveRegex;
    tRegexFiltersColumnsVisibilityMap mRegexFiltersColumnsVisibilityMap;
    bool mbFilterVariables;
    bool mbRootConfigInitialised;
    QString mSelectedRegexFile; // name of the regex file to be used.
};

#endif // CSETTINGSMANAGER_HPP
