/**
 * @file    CSettingsManager.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSettingsManager class
 */

#include "QFile"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>

#include "CConsoleCtrl.hpp"

#include "CSettingsManager.hpp"

static const int sSettingsManager_Version = 1;
static const QString sSettingsManager_Directory = QString("plugins") + QDir::separator() + "DLTMessageAnalyzerConfig";
static const QString sSettingsManager_Regex_SubDirectory = "regexes";
static const QString sSettingsManager_User_SettingsFile = "user_settings.json";
static const QString sSettingsManager_Root_SettingsFile = "root_settings.json";

static const QString sSettingsManagerVersionKey = "settingsManagerVersion";
static const QString sAliasesKey = "aliases";
static const QString sAliasKey = "alias";
static const QString sRegexKey = "regex";
static const QString sIsDefaultKey = "isDefault";
static const QString sNumberOfThreadsKey = "numberOfThreads";
static const QString sIsContinuousSearchKey = "isContinuousSearch";
static const QString sCopySearchResultAsHTMLKey = "copySearchResultAsHTML";
static const QString sMinimizePatternsViewOnSelectionKey = "minimizePatternsViewOnSelection";
static const QString sWriteSettingsOnEachUpdateChangedKey = "writeSettingsOnEachUpdateChanged";
static const QString sCacheEnabledKey = "cacheEnabled";
static const QString sCacheMaxSizeMsBKey = "cacheMaxSizeMB";
static const QString sRDPModeKey = "RDPMode";
static const QString sRegexMonoHighlightingColosRKey = "regexMonoHighlightingColor";
static const QString sRKey = "r";
static const QString sGKey = "g";
static const QString sBKey = "b";
static const QString sHighlightActivePatternsKey = "highlightActivePatterns";
static const QString sPatternsHighlightingColorKey = "patternsHighlightingColor";
static const QString sSearchResultMonoColorHighlightingKey = "searchResultMonoColorHighlighting";
static const QString sSearchResultHighlightingGradientKey = "searchResultHighlightingGradient";
static const QString sNumberOfColorsKey = "numberOfColors";
static const QString sSearchResultColumnsVisibilityMapKey = "searchResultColumnsVisibilityMap";
static const QString sMarkTimeStampWithBold = "markTimeStampWithBold";
static const QString sPatternsColumnsVisibilityMapKey = "patternsColumnsVisibilityMap";
static const QString sRegexFiltersColumnsVisibilityMapKey = "RegexFiltersColumnsVisibilityMap";
static const QString sFilterVariablesKey = "FilterVariables";
static const QString sCaseSensitiveRegex = "caseSensitiveRegex";
static const QString sSelectedRegexFile = "selectedRegexFile";

static const tSettingsManagerVersion sDefaultSettingsManagerVersion = static_cast<tSettingsManagerVersion>(-1);
static const tSettingsManagerVersion sCurrentSettingsManagerVersion = 1u; // current version of settings manager used by SW.

static tSearchResultColumnsVisibilityMap fillInDefaultSearchResultColumnsVisibilityMap()
{
    tSearchResultColumnsVisibilityMap result;

    // fields, which are visible by default
    result.insert(eSearchResultColumn::Index, true);
    result.insert(eSearchResultColumn::Time, true);
    result.insert(eSearchResultColumn::Timestamp, true);
    result.insert(eSearchResultColumn::Ecuid, true);
    result.insert(eSearchResultColumn::Apid, true);
    result.insert(eSearchResultColumn::Ctid, true);
    result.insert(eSearchResultColumn::Payload, true);

    // fields, which are not visible by default
    result.insert(eSearchResultColumn::Count, false);
    result.insert(eSearchResultColumn::SessionId, false);
    result.insert(eSearchResultColumn::Type, false);
    result.insert(eSearchResultColumn::Subtype, false);
    result.insert(eSearchResultColumn::Mode, false);
    result.insert(eSearchResultColumn::Args, false);

    return result;
}

static const tSearchResultColumnsVisibilityMap sDefaultSearchResultColumnsVisibilityMap
= fillInDefaultSearchResultColumnsVisibilityMap();

static tPatternsColumnsVisibilityMap fillInDefaultPatternsColumnsVisibilityMap()
{
    tPatternsColumnsVisibilityMap result;

    // fields, which are visible by default
    result.insert(ePatternsColumn::AliasTreeLevel, true);
    result.insert(ePatternsColumn::Default, true);
    result.insert(ePatternsColumn::Combine, true);

    // fields, which are not visible by default
    result.insert(ePatternsColumn::Regex, false);

    return result;
}

static const tPatternsColumnsVisibilityMap sDefaultPatternsColumnsVisibilityMap
= fillInDefaultPatternsColumnsVisibilityMap();

static tRegexFiltersColumnsVisibilityMap fillInDefaultRegexFiltersColumnsVisibilityMap()
{
    tRegexFiltersColumnsVisibilityMap result;

    // fields, which are visible by default
    result.insert(eRegexFiltersColumn::Index, true);
    result.insert(eRegexFiltersColumn::ItemType, true);
    result.insert(eRegexFiltersColumn::Value, true);

    return result;
}


static const tRegexFiltersColumnsVisibilityMap sDefaultRegexFiltersColumnsVisibilityMap
= fillInDefaultRegexFiltersColumnsVisibilityMap();

CSettingsManager::CSettingsManager():
    mSettingsManagerVersion(sDefaultSettingsManagerVersion),
    mAliases(),
    mNumberOfThreads(1),
    mbContinuousSearch(true),
    mbCopySearchResultAsHTML(true),
    mbMinimizePatternsViewOnSelection(false),
    mbWriteSettingsOnEachUpdate(true),
    mbCacheEnabled(true),
    mCacheMaxSizeMB(500),
    mbRDPMode(false),
    mRegexMonoHighlightingColor(150,0,0),
    mbHighlightActivePatterns(true),
    mPatternsHighlightingColor(0,150,0),
    mbSearchResultMonoColorHighlighting(false),
    mSearchResultHighlightingGradient( QColor(154,0,146), QColor(1,162,165), 3 ),
    mSearchResultHighlightingGradientProtector(),
    mSearchResultColumnsVisibilityMap(sDefaultSearchResultColumnsVisibilityMap),
    mbMarkTimeStampWithBold(true),
    mPatternsColumnsVisibilityMap(sDefaultPatternsColumnsVisibilityMap),
    mbCaseSensitiveRegex(false),
    mRegexFiltersColumnsVisibilityMap(sDefaultRegexFiltersColumnsVisibilityMap),
    mbFilterVariables(true),
    mbRootConfigInitialised(false),
    mSelectedRegexFile(sDefaultRegexFileName)
{
    CSettingsManager::tOperationResult result = setUp();

    if(false == result.bResult)
    {
        SEND_ERR(QString("[CSettingsManager][setUp] Error - %1").arg(result.err));
    }
    else
    {
        SEND_MSG(QString("[CSettingsManager][setUp] set up is successful"));
    }
}

CSettingsManager::tOperationResult CSettingsManager::backwardCompatibility()
{
    SEND_MSG(QString("[CSettingsManager] Performing backward compatibility check."));

    auto result = backwardCompatibility_V0_V1();

    if(true == result.bResult)
    {
        result = loadRootConfig(); // load root config. It should be available starting version 0

        if(true == result.bResult)
        {
            SEND_MSG(QString("[CSettingsManager] Persisted settings manager verison - %1.").arg(getSettingsManagerVersion()));
            SEND_MSG(QString("[CSettingsManager] Target settings manager verison - %1.").arg(sSettingsManager_Version));

            setSettingsManagerVersion( sSettingsManager_Version ); // if backward compatibility was successful, we need to update the settings manager version
        }
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::loadRootConfig()
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QString rootConfigPath = getRootSettingsFilepath();

    QFile jsonFile(rootConfigPath);
    if(jsonFile.open(QFile::ReadOnly))
    {
        auto jsonDoc =  QJsonDocument().fromJson(jsonFile.readAll());

        if( true == jsonDoc.isArray() )
        {
            QJsonArray arrayRows = jsonDoc.array();

            for(auto item : arrayRows) // no references returned
            {
                if(true == item.isObject())
                {
                    auto object = item.toObject();

                    {
                        auto foundSettingsManagerVersion = object.find(sSettingsManagerVersionKey);

                        if(foundSettingsManagerVersion != object.end())
                        {
                            if(true == foundSettingsManagerVersion->isDouble())
                            {
                                mSettingsManagerVersion = static_cast<tSettingsManagerVersion>(foundSettingsManagerVersion->toDouble());
                            }
                        }

                        settingsManagerVersionChanged(mSettingsManagerVersion);
                    }
                }
            }
        }

        result.bResult = true;
    }
    else
    {
        result.err = QString("[%1] Was not able to open file - \"%2\"").arg(__FUNCTION__).arg(rootConfigPath);
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::storeRootConfig()
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QString rootConfigPath = getRootSettingsFilepath();

    QFile jsonFile(rootConfigPath);

    if(jsonFile.open(QFile::WriteOnly))
    {
        QJsonArray settingsArray;

        {
            QJsonObject settingsManagerVersion;
            settingsManagerVersion.insert( sSettingsManagerVersionKey, QJsonValue( static_cast<int>(mSettingsManagerVersion) ) );
            settingsArray.append(settingsManagerVersion);
        }

        QJsonDocument jsonDoc( settingsArray );
        jsonFile.write( jsonDoc.toJson() );

        result.bResult = true;
    }
    else
    {
        result.err = QString("[%1] Was not able to open file - \"%2\"").arg(__FUNCTION__).arg(rootConfigPath);
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::storeConfigs()
{
    CSettingsManager::tOperationResult  result = storeRootConfig();

    if( true == result.bResult )
    {
        result = storeSettingsConfig();

        if(true == result.bResult)
        {
            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSelectedRegexFile;
            result = storeRegexConfigCustomPath(regexSettingsFilePath);
        }
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::loadConfigs()
{
    CSettingsManager::tOperationResult  result = loadRootConfig(); // load root config

    if( true == result.bResult )
    {
        result = loadSettingsConfig(); // load settings

        if( true == result.bResult )
        {
            if(false == mSelectedRegexFile.isEmpty())
            {
                QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSelectedRegexFile;
                result = loadRegexConfigCustomPath(regexSettingsFilePath); // load regexes
            }
        }
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::setUp()
{
    CSettingsManager::tOperationResult result = backwardCompatibility(); // ensure that our file structure is up to date

    if(true == result.bResult)
    {
        result = loadConfigs();
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::backwardCompatibility_V0_V1()
{
    CSettingsManager::tOperationResult result;
    result.bResult = true;

    const char* sFileName = "DLTMessageAnalyzer_plugin_config.json";

    QString V0_SettingsFilePath(QString(".") + QDir::separator() + sFileName);
    QFile V0_SettingsFile( V0_SettingsFilePath );

    QString regexDirPath = getRegexDirectory();
    QDir regexDir( regexDirPath );

    // Let's check, whether we have V0.
    // That can be checked only indirectly, based on file-system-based conclusions
    if(false == regexDir.exists()) // regex dir does not exist. Most probably we've faced old system
    {
        SEND_MSG(QString("[CSettingsManager] Performing backward compatibility iteration from V0 to V1"));

        QDir dir;
        QString configDirPath = QString(".") + QDir::separator() + sSettingsManager_Directory;

        // let's create config dir
        if(true == dir.mkpath(configDirPath))
        {
            // let's create regex dir
            if(true == dir.mkpath( regexDirPath ))
            {
                // folders are created.
                // Let's fill them in.

                // create root settings
                {
                    mSettingsManagerVersion = 1;
                    result = storeRootConfig();
                }

                if(true == result.bResult)
                {
                    if(true == V0_SettingsFile.exists()) // the V0 file exist.
                    {
                        // re-save general settings to new folder structure
                        result = loadSettingsConfigCustomPath(V0_SettingsFilePath);

                        if(true == result.bResult)
                        {
                            result = storeSettingsConfigCustomPath(getUserSettingsFilepath());

                            if(true == result.bResult)
                            {
                                result = loadRegexConfigCustomPath( V0_SettingsFilePath ); // re-save regex settings to new folder structure
                                if(true == result.bResult)
                                {
                                    QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + sDefaultRegexFileName;
                                    result = storeRegexConfigCustomPath( regexSettingsFilePath );

                                    if(true == result.bResult)
                                    {
                                        mSelectedRegexFile = sDefaultRegexFileName;

                                        // delete old-style JSON file
                                        if(false == V0_SettingsFile.remove())
                                        {
                                            result.bResult = false;
                                            result.err = QString( "[%1] Was not able to delete file \"%2\"" ).arg(__FUNCTION__).arg(V0_SettingsFilePath);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else // if there is no old-style config, let's simply create default setting files
                    {
                        result = storeSettingsConfig();

                        if(true == result.bResult)
                        {
                            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + sDefaultRegexFileName;
                            result = storeRegexConfigCustomPath( regexSettingsFilePath );
                        }
                    }
                }
            }
            else
            {
                result.bResult = false;
                result.err = QString( "[%1] Was not able to create directory \"%2\"" ).arg(__FUNCTION__).arg(regexDirPath);
            }
        }
        else
        {
            result.bResult = false;
            result.err = QString( "[%1] Was not able to create directory \"%2\"" ).arg(__FUNCTION__).arg(configDirPath);
        }

        SEND_MSG(QString("[CSettingsManager] BackwardCompatibility_V0_V1 finished with result - %1").arg(true == result.bResult ? "SUCCESSFUL" : "FAILED"));
    }
    else
    {
        SEND_MSG(QString("[CSettingsManager] Backward compatibility iteration from V0 to V1 not needed"));
    }

    return result;
}

tSettingsManagerPtr CSettingsManager::getInstance()
{
    static tSettingsManagerPtr sInstance = std::shared_ptr<CSettingsManager>( new CSettingsManager() );
    return sInstance;
}

void CSettingsManager::setSettingsManagerVersion(const tSettingsManagerVersion& val)
{
    bool bUpdate = mSettingsManagerVersion != val;

    if(true == bUpdate)
    {
        mSettingsManagerVersion = val;
        settingsManagerVersionChanged(mSettingsManagerVersion);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeRootConfig();
    }
}

void CSettingsManager::setAliases(const tAliasItemVec& val)
{
    bool bUpdate = mAliases != val;

    if(true == bUpdate)
    {
        mAliases = val;
        aliasesChanged(mAliases);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSelectedRegexFile;
        storeRegexConfigCustomPath(regexSettingsFilePath);
    }
}

void CSettingsManager::setAliasIsDefault(const QString& alias, bool isDefault)
{
    auto foundAlias = std::find_if( mAliases.begin(), mAliases.end(), [&alias](const tAliasItem& aliasItem)->bool
    {
        return aliasItem.alias == alias;
    });

    if(foundAlias != mAliases.end())
    {
        foundAlias->isDefault = isDefault;
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSelectedRegexFile;
        storeRegexConfigCustomPath(regexSettingsFilePath);
    }

    aliasesChanged(mAliases);
}

void CSettingsManager::setNumberOfThreads(const int& val)
{
    bool bUpdate = mNumberOfThreads != val;

    if(true == bUpdate)
    {
        mNumberOfThreads = val;
        numberOfThreadsChanged(mNumberOfThreads);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setContinuousSearch(bool val)
{
    bool bUpdate = mbContinuousSearch != val;

    if(true == bUpdate)
    {
        mbContinuousSearch = val;
        continuousSearchChanged(mbContinuousSearch);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setCopySearchResultAsHTML(bool val)
{
    bool bUpdate = mbCopySearchResultAsHTML != val;

    if(true == bUpdate)
    {
        mbCopySearchResultAsHTML = val;
        copySearchResultAsHTMLChanged(mbCopySearchResultAsHTML);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setMinimizePatternsViewOnSelection(bool val)
{
    bool bUpdate = mbMinimizePatternsViewOnSelection != val;

    if(true == bUpdate)
    {
        mbMinimizePatternsViewOnSelection = val;
        minimizePatternsViewOnSelectionChanged(mbMinimizePatternsViewOnSelection);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setWriteSettingsOnEachUpdate(bool val)
{
    bool bUpdate = mbWriteSettingsOnEachUpdate != val;

    if(true == bUpdate)
    {
        mbWriteSettingsOnEachUpdate = val;
        writeSettingsOnEachUpdateChanged(mbWriteSettingsOnEachUpdate);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setCacheEnabled(bool val)
{
    bool bUpdate = mbCacheEnabled != val;

    if(true == bUpdate)
    {
        mbCacheEnabled = val;
        cacheEnabledChanged(mbCacheEnabled);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setCacheMaxSizeMB(tCacheSizeMB val)
{
    bool bUpdate = mCacheMaxSizeMB != val;

    if(true == bUpdate)
    {
        mCacheMaxSizeMB = val;
        cacheMaxSizeMBChanged(mCacheMaxSizeMB);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setRDPMode(bool val)
{
    bool bUpdate = mbRDPMode != val;

    if(true == bUpdate)
    {
        mbRDPMode = val;
        RDPModeChanged(mbRDPMode);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setRegexMonoHighlightingColor(const QColor& val)
{
    bool bUpdate = mRegexMonoHighlightingColor != val;

    if(true == bUpdate)
    {
        mRegexMonoHighlightingColor = val;
        regexMonoHighlightingColorChanged(mRegexMonoHighlightingColor);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setHighlightActivePatterns(bool val)
{
    bool bUpdate = mbHighlightActivePatterns != val;

    if(true == bUpdate)
    {
        mbHighlightActivePatterns = val;
        highlightActivePatternsChanged(mbHighlightActivePatterns);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setPatternsHighlightingColor(const QColor& val)
{
    bool bUpdate = mPatternsHighlightingColor != val;

    if(true == bUpdate)
    {
        mPatternsHighlightingColor = val;
        patternsHighlightingColorChanged(mPatternsHighlightingColor);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setSearchResultMonoColorHighlighting(bool val)
{
    bool bUpdate = mbSearchResultMonoColorHighlighting != val;

    if(true == bUpdate)
    {
        mbSearchResultMonoColorHighlighting = val;
        searchResultMonoColorHighlightingChanged(mbSearchResultMonoColorHighlighting);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setSearchResultHighlightingGradient(const tHighlightingGradient& val)
{
    std::lock_guard<std::mutex> lock(mSearchResultHighlightingGradientProtector);

    bool bUpdate = mSearchResultHighlightingGradient != val;

    if(true == bUpdate)
    {
        mSearchResultHighlightingGradient = val;
        searchResultHighlightingGradientChanged(mSearchResultHighlightingGradient);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setSearchResultColumnsVisibilityMap(const tSearchResultColumnsVisibilityMap& val)
{
    bool bUpdate = mSearchResultColumnsVisibilityMap != val;

    if(true == bUpdate)
    {
        mSearchResultColumnsVisibilityMap = val;
        searchResultColumnsVisibilityMapChanged(mSearchResultColumnsVisibilityMap);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setMarkTimeStampWithBold(bool val)
{
    bool bUpdate = mbMarkTimeStampWithBold != val;

    if(true == bUpdate)
    {
        mbMarkTimeStampWithBold = val;
        markTimeStampWithBoldChanged(mbMarkTimeStampWithBold);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setPatternsColumnsVisibilityMap(const tPatternsColumnsVisibilityMap& val)
{
    bool bUpdate = mPatternsColumnsVisibilityMap != val;

    if(true == bUpdate)
    {
        mPatternsColumnsVisibilityMap = val;
        patternsColumnsVisibilityMapChanged(mPatternsColumnsVisibilityMap);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setCaseSensitiveRegex(bool val)
{
    bool bUpdate = mbCaseSensitiveRegex != val;

    if(true == bUpdate)
    {
        mbCaseSensitiveRegex = val;
        caseSensitiveRegexChanged(mbCaseSensitiveRegex);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setRegexFiltersColumnsVisibilityMap(const tRegexFiltersColumnsVisibilityMap& val)
{
    bool bUpdate = mRegexFiltersColumnsVisibilityMap != val;

    if(true == bUpdate)
    {
        mRegexFiltersColumnsVisibilityMap = val;
        regexFiltersColumnsVisibilityMapChanged(mRegexFiltersColumnsVisibilityMap);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setFilterVariables(bool val)
{
    bool bUpdate = mbFilterVariables != val;

    if(true == bUpdate)
    {
        mbFilterVariables = val;
        filterVariablesChanged(mbFilterVariables);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::setSelectedRegexFile(const QString& val)
{
    bool bUpdate = mSelectedRegexFile != val;

    if(true == bUpdate)
    {
        mSelectedRegexFile = val;

        clearRegexConfig();

        QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + val;
        loadRegexConfigCustomPath(regexSettingsFilePath);

        selectedRegexFileChanged(mSelectedRegexFile);
    }

    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

const tSettingsManagerVersion& CSettingsManager::getSettingsManagerVersion() const
{
    return mSettingsManagerVersion;
}

const CSettingsManager::tAliasItemVec& CSettingsManager::getAliases() const
{
    return mAliases;
}

const int& CSettingsManager::getNumberOfThreads() const
{
    return mNumberOfThreads;
}

bool CSettingsManager::getContinuousSearch() const
{
    return mbContinuousSearch;
}

bool CSettingsManager::getCopySearchResultAsHTML() const
{
    return mbCopySearchResultAsHTML;
}

bool CSettingsManager::getMinimizePatternsViewOnSelection() const
{
    return mbMinimizePatternsViewOnSelection;
}

bool CSettingsManager::getWriteSettingsOnEachUpdate()const
{
    return mbWriteSettingsOnEachUpdate;
}

bool CSettingsManager::getCacheEnabled() const
{
    return mbCacheEnabled;
}

const tCacheSizeMB& CSettingsManager::getCacheMaxSizeMB() const
{
    return mCacheMaxSizeMB;
}

bool CSettingsManager::getRDPMode() const
{
    return mbRDPMode;
}

const QColor& CSettingsManager::getRegexMonoHighlightingColor() const
{
    return mRegexMonoHighlightingColor;
}

bool CSettingsManager::getHighlightActivePatterns() const
{
    return mbHighlightActivePatterns;
}

const QColor& CSettingsManager::getPatternsHighlightingColor() const
{
    return mPatternsHighlightingColor;
}

bool CSettingsManager::getSearchResultMonoColorHighlighting() const
{
    return mbSearchResultMonoColorHighlighting;
}

tHighlightingGradient CSettingsManager::getSearchResultHighlightingGradient() const
{
   std::lock_guard<std::mutex> lock(*const_cast<std::mutex*>(&mSearchResultHighlightingGradientProtector));
   return mSearchResultHighlightingGradient;
}

const tSearchResultColumnsVisibilityMap& CSettingsManager::getSearchResultColumnsVisibilityMap() const
{
    return mSearchResultColumnsVisibilityMap;
}

bool CSettingsManager::getMarkTimeStampWithBold() const
{
    return mbMarkTimeStampWithBold;
}

const tPatternsColumnsVisibilityMap& CSettingsManager::getPatternsColumnsVisibilityMap() const
{
    return mPatternsColumnsVisibilityMap;
}

bool CSettingsManager::getCaseSensitiveRegex() const
{
    return mbCaseSensitiveRegex;
}

const tRegexFiltersColumnsVisibilityMap& CSettingsManager::getRegexFiltersColumnsVisibilityMap() const
{
    return mRegexFiltersColumnsVisibilityMap;
}

bool CSettingsManager::getFilterVariables() const
{
    return mbFilterVariables;
}

QString CSettingsManager::getSelectedRegexFile() const
{
    return mSelectedRegexFile;
}

QString CSettingsManager::getRegexDirectory() const
{
    return QString(".") + QDir::separator() +
           sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_Regex_SubDirectory;
}

QString CSettingsManager::getRegexDirectoryFull() const
{
    return QDir::toNativeSeparators( QDir::currentPath() ) + QDir::separator() +
           sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_Regex_SubDirectory;
}

QString CSettingsManager::getUserSettingsFilepath() const
{
    return QString(".") + QDir::separator() +
           sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_User_SettingsFile;
}

QString CSettingsManager::getRootSettingsFilepath() const
{
    return QString(".") + QDir::separator() +
           sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_Root_SettingsFile;
}

void CSettingsManager::clearRegexConfig()
{
    mAliases.clear();
    aliasesChanged(mAliases);
}

CSettingsManager::tOperationResult CSettingsManager::loadRegexConfigCustomPath(const QString &filePath)
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::ReadOnly))
    {
        auto jsonDoc =  QJsonDocument().fromJson(jsonFile.readAll());

        if( true == jsonDoc.isArray() )
        {
            QJsonArray arrayRows = jsonDoc.array();

            for(auto item : arrayRows) // no references returned
            {
                if(true == item.isObject())
                {
                    auto object = item.toObject();

                    {
                        auto foundAliases = object.find(sAliasesKey);
                        if(foundAliases != object.end())
                        {
                            auto aliases = foundAliases.value();

                            if(true == aliases.isArray())
                            {
                                mAliases.clear();

                                auto aliasesArray = aliases.toArray();
                                for( const auto aliasObj : aliasesArray)
                                {
                                    if(true == aliasObj.isObject())
                                    {
                                        QJsonObject obj = aliasObj.toObject();

                                        auto alias = obj.find( sAliasKey );

                                        if(alias != obj.end() && alias->isString())
                                        {
                                            auto regex = obj.find(sRegexKey);

                                            if(regex != obj.end() && regex->isString())
                                            {
                                                bool bIsDefault = false;

                                                auto isDefault = obj.find(sIsDefaultKey);

                                                if(isDefault != obj.end() && isDefault->isBool())
                                                {
                                                    bIsDefault = isDefault->toBool();
                                                }

                                                mAliases.push_back(tAliasItem(bIsDefault, alias->toString(), regex->toString()));
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        aliasesChanged(mAliases);
                    }
                }
            }
        }

        result.bResult = true;
    }
    else
    {
        result.bResult = false;
        result.err = QString("[%1] Failed to open file - \"%2\"").arg(__FUNCTION__).arg(filePath);
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::storeRegexConfigCustomPath(const QString &filePath) const
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filePath);

    if(jsonFile.open(QFile::WriteOnly))
    {
        QJsonArray settingsArray;

        {
            QJsonObject aliases;
            QJsonArray arrayAliases;
            for(const auto& item : mAliases)
            {
                QJsonObject obj;
                obj.insert( sIsDefaultKey, QJsonValue( item.isDefault ) );
                obj.insert( sAliasKey, QJsonValue( item.alias ) );
                obj.insert( sRegexKey, QJsonValue( item.regex ) );
                arrayAliases.append( obj );
            }

            QJsonObject setting;
            setting.insert( sAliasesKey, QJsonValue( arrayAliases ) );

            settingsArray.append(setting);
        }

        QJsonDocument jsonDoc( settingsArray );
        jsonFile.write( jsonDoc.toJson() );

        result.bResult = true;
    }
    else
    {
        result.bResult = false;
        result.err = QString("[%1] Failed to open file - \"%2\"").arg(__FUNCTION__).arg(filePath);
    }

    return result;
}

CSettingsManager::tOperationResult  CSettingsManager::loadSettingsConfig()
{
    return loadSettingsConfigCustomPath(getUserSettingsFilepath());
}

CSettingsManager::tOperationResult  CSettingsManager::storeSettingsConfig()
{
    return storeSettingsConfigCustomPath(getUserSettingsFilepath());
}

CSettingsManager::tOperationResult CSettingsManager::storeSettingsConfigCustomPath(const QString& filepath)
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filepath);

    if(jsonFile.open(QFile::WriteOnly))
    {
        QJsonArray settingsArray;

        {
            QJsonObject numberOfThreads;
            numberOfThreads.insert( sNumberOfThreadsKey, QJsonValue( mNumberOfThreads ) );
            settingsArray.append(numberOfThreads);
        }

        {
            QJsonObject isContinuousSearch;
            isContinuousSearch.insert( sIsContinuousSearchKey, QJsonValue( mbContinuousSearch ) );
            settingsArray.append(isContinuousSearch);
        }

        {
            QJsonObject copySearchResultAsHTML;
            copySearchResultAsHTML.insert( sCopySearchResultAsHTMLKey, QJsonValue( mbCopySearchResultAsHTML ) );
            settingsArray.append(copySearchResultAsHTML);
        }

        {
            QJsonObject minimizePatternsViewOnSelection;
            minimizePatternsViewOnSelection.insert( sMinimizePatternsViewOnSelectionKey, QJsonValue( mbMinimizePatternsViewOnSelection ) );
            settingsArray.append(minimizePatternsViewOnSelection);
        }

        {
            QJsonObject writeSettingsOnEachUpdateChanged;
            writeSettingsOnEachUpdateChanged.insert( sWriteSettingsOnEachUpdateChangedKey, QJsonValue( mbWriteSettingsOnEachUpdate ) );
            settingsArray.append(writeSettingsOnEachUpdateChanged);
        }

        {
            QJsonObject cacheEnabled;
            cacheEnabled.insert( sCacheEnabledKey, QJsonValue( mbCacheEnabled ) );
            settingsArray.append(cacheEnabled);
        }

        {
            QJsonObject cacheMaxSizeMB;
            cacheMaxSizeMB.insert( sCacheMaxSizeMsBKey, QJsonValue( static_cast<int>(mCacheMaxSizeMB) ) );
            settingsArray.append(cacheMaxSizeMB);
        }

        {
            QJsonObject RDPMode;
            RDPMode.insert( sRDPModeKey, QJsonValue( mbRDPMode ) );
            settingsArray.append(RDPMode);
        }

        {
            QJsonObject regexMonoHighlightingColor;

            regexMonoHighlightingColor.insert(sRKey, mRegexMonoHighlightingColor.red());
            regexMonoHighlightingColor.insert(sGKey, mRegexMonoHighlightingColor.green());
            regexMonoHighlightingColor.insert(sBKey, mRegexMonoHighlightingColor.blue());

            QJsonObject setting;
            setting.insert( sRegexMonoHighlightingColosRKey, QJsonValue( regexMonoHighlightingColor ) );
            settingsArray.append(setting);
        }

        {
            QJsonObject highlightActivePatterns;
            highlightActivePatterns.insert( sHighlightActivePatternsKey, QJsonValue( mbHighlightActivePatterns ) );
            settingsArray.append(highlightActivePatterns);
        }

        {
            QJsonObject patternsHighlightingColor;

            patternsHighlightingColor.insert(sRKey, mPatternsHighlightingColor.red());
            patternsHighlightingColor.insert(sGKey, mPatternsHighlightingColor.green());
            patternsHighlightingColor.insert(sBKey, mPatternsHighlightingColor.blue());

            QJsonObject setting;
            setting.insert( sPatternsHighlightingColorKey, QJsonValue( patternsHighlightingColor ) );
            settingsArray.append(setting);
        }

        {
            QJsonObject searchResultMonoColorHighlighting;
            searchResultMonoColorHighlighting.insert( sSearchResultMonoColorHighlightingKey, QJsonValue( mbSearchResultMonoColorHighlighting ) );
            settingsArray.append(searchResultMonoColorHighlighting);
        }

        {
            QJsonObject searchResultHighlightingGradient;

            searchResultHighlightingGradient.insert(sRKey + "_1", mSearchResultHighlightingGradient.from.red());
            searchResultHighlightingGradient.insert(sGKey + "_1", mSearchResultHighlightingGradient.from.green());
            searchResultHighlightingGradient.insert(sBKey + "_1", mSearchResultHighlightingGradient.from.blue());
            searchResultHighlightingGradient.insert(sRKey + "_2", mSearchResultHighlightingGradient.to.red());
            searchResultHighlightingGradient.insert(sGKey + "_2", mSearchResultHighlightingGradient.to.green());
            searchResultHighlightingGradient.insert(sBKey + "_2", mSearchResultHighlightingGradient.to.blue());

            searchResultHighlightingGradient.insert(sNumberOfColorsKey, mSearchResultHighlightingGradient.numberOfColors);

            QJsonObject setting;
            setting.insert( sSearchResultHighlightingGradientKey, QJsonValue( searchResultHighlightingGradient ) );
            settingsArray.append(setting);
        }

        {
            QJsonObject searchResultColumnsVisibilityMap;
            QJsonArray arraySearchResultColumnsVisibilityItems;
            for(auto it = mSearchResultColumnsVisibilityMap.begin(); it != mSearchResultColumnsVisibilityMap.end(); ++it)
            {
                QJsonObject obj;
                obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
                arraySearchResultColumnsVisibilityItems.append( obj );
            }

            QJsonObject setting;
            setting.insert( sSearchResultColumnsVisibilityMapKey, QJsonValue( arraySearchResultColumnsVisibilityItems ) );
            settingsArray.append(setting);
        }

        {
            QJsonObject markTimeStampWithBold;
            markTimeStampWithBold.insert( sMarkTimeStampWithBold, QJsonValue( mbMarkTimeStampWithBold ) );
            settingsArray.append(markTimeStampWithBold);
        }

        {
            QJsonObject patternsColumnsVisibilityMap;
            QJsonArray arrayPatternsColumnsVisibilityItems;
            for(auto it = mPatternsColumnsVisibilityMap.begin(); it != mPatternsColumnsVisibilityMap.end(); ++it)
            {
                QJsonObject obj;
                obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
                arrayPatternsColumnsVisibilityItems.append( obj );
            }

            QJsonObject setting;
            setting.insert( sPatternsColumnsVisibilityMapKey, QJsonValue( arrayPatternsColumnsVisibilityItems ) );
            settingsArray.append(setting);
        }

        {
            QJsonObject caseSensitiveRegex;
            caseSensitiveRegex.insert( sCaseSensitiveRegex, QJsonValue( mbCaseSensitiveRegex ) );
            settingsArray.append(caseSensitiveRegex);
        }

        {
            QJsonObject regexFiltersColumnsVisibilityMap;
            QJsonArray arrayRegexFiltersColumnsVisibilityItems;
            for(auto it = mRegexFiltersColumnsVisibilityMap.begin(); it != mRegexFiltersColumnsVisibilityMap.end(); ++it)
            {
                QJsonObject obj;
                obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
                arrayRegexFiltersColumnsVisibilityItems.append( obj );
            }

            QJsonObject setting;
            setting.insert( sRegexFiltersColumnsVisibilityMapKey, QJsonValue( arrayRegexFiltersColumnsVisibilityItems ) );
            settingsArray.append(setting);
        }

        {
            QJsonObject filterVariables;
            filterVariables.insert( sFilterVariablesKey, QJsonValue( mbFilterVariables ) );
            settingsArray.append(filterVariables);
        }

        {
            QJsonObject selectedRegexFile;
            selectedRegexFile.insert( sSelectedRegexFile, QJsonValue( mSelectedRegexFile ) );
            settingsArray.append(selectedRegexFile);
        }

        QJsonDocument jsonDoc( settingsArray );
        jsonFile.write( jsonDoc.toJson() );

        result.bResult = true;
    }
    else
    {
        result.bResult = false;
        result.err = QString("[%1] Failed to open file - \"%2\"").arg(__FUNCTION__).arg(filepath);
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::loadSettingsConfigCustomPath(const QString& filepath)
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filepath);
    if(jsonFile.open(QFile::ReadOnly))
    {
        auto jsonDoc =  QJsonDocument().fromJson(jsonFile.readAll());

        if( true == jsonDoc.isArray() )
        {
            QJsonArray arrayRows = jsonDoc.array();

            for(auto item : arrayRows) // no references returned
            {
                if(true == item.isObject())
                {
                    auto object = item.toObject();

                    {
                        auto foundNumberOfThreads = object.find(sNumberOfThreadsKey);
                        if(foundNumberOfThreads != object.end())
                        {
                            if(true == foundNumberOfThreads->isDouble())
                            {
                                mNumberOfThreads = static_cast<int>(foundNumberOfThreads->toDouble());
                            }
                        }

                        numberOfThreadsChanged(mNumberOfThreads);
                    }

                    {
                        auto foundIsContinuousSearch = object.find(sIsContinuousSearchKey);

                        if(foundIsContinuousSearch != object.end())
                        {
                            if(true == foundIsContinuousSearch->isBool())
                            {
                                mbContinuousSearch = foundIsContinuousSearch->toBool();
                            }
                        }

                        continuousSearchChanged(mbContinuousSearch);
                    }

                    {
                        auto foundCopySearchResultAsHTML = object.find(sCopySearchResultAsHTMLKey);

                        if(foundCopySearchResultAsHTML != object.end())
                        {
                            if(true == foundCopySearchResultAsHTML->isBool())
                            {
                                mbCopySearchResultAsHTML = foundCopySearchResultAsHTML->toBool();
                            }
                        }

                        copySearchResultAsHTMLChanged(mbCopySearchResultAsHTML);
                    }

                    {
                        auto foundMinimizePatternsViewOnSelection = object.find(sMinimizePatternsViewOnSelectionKey);

                        if(foundMinimizePatternsViewOnSelection != object.end())
                        {
                            if(true == foundMinimizePatternsViewOnSelection->isBool())
                            {
                                mbMinimizePatternsViewOnSelection = foundMinimizePatternsViewOnSelection->toBool();
                            }
                        }

                        minimizePatternsViewOnSelectionChanged(mbMinimizePatternsViewOnSelection);
                    }

                    {
                        auto foundWriteSettingsOnEachUpdateChanged = object.find(sWriteSettingsOnEachUpdateChangedKey);

                        if(foundWriteSettingsOnEachUpdateChanged != object.end())
                        {
                            if(true == foundWriteSettingsOnEachUpdateChanged->isBool())
                            {
                                mbWriteSettingsOnEachUpdate = foundWriteSettingsOnEachUpdateChanged->toBool();
                            }
                        }

                        writeSettingsOnEachUpdateChanged(mbMinimizePatternsViewOnSelection);
                    }

                    {
                        auto foundCacheEnabled = object.find(sCacheEnabledKey);

                        if(foundCacheEnabled != object.end())
                        {
                            if(true == foundCacheEnabled->isBool())
                            {
                                mbCacheEnabled = foundCacheEnabled->toBool();
                            }
                        }

                        cacheEnabledChanged(mbCacheEnabled);
                    }

                    {
                        auto foundCacheMaxSizeMB = object.find(sCacheMaxSizeMsBKey);

                        if(foundCacheMaxSizeMB != object.end())
                        {
                            if(true == foundCacheMaxSizeMB->isDouble())
                            {
                                mCacheMaxSizeMB = static_cast<unsigned int>(foundCacheMaxSizeMB->toDouble());
                            }
                        }

                        cacheMaxSizeMBChanged(mCacheMaxSizeMB);
                    }

                    {
                        auto foundRDPMode = object.find(sRDPModeKey);

                        if(foundRDPMode != object.end())
                        {
                            if(true == foundRDPMode->isBool())
                            {
                                mbRDPMode = foundRDPMode->toBool();
                            }
                        }

                        RDPModeChanged(mbRDPMode);
                    }

                    {
                        auto foundRegexMonoHighlightingColor = object.find(sRegexMonoHighlightingColosRKey);

                        if(foundRegexMonoHighlightingColor != object.end())
                        {
                            if(true == foundRegexMonoHighlightingColor->isObject())
                            {
                                QJsonObject obj = foundRegexMonoHighlightingColor->toObject();

                                auto r = obj.find( sRKey );
                                auto g = obj.find( sGKey );
                                auto b = obj.find( sBKey );

                                if( r != obj.end() && r->isDouble() &&
                                        g != obj.end() && g->isDouble() &&
                                        b != obj.end() && b->isDouble())
                                {
                                    mRegexMonoHighlightingColor = QColor(static_cast<int>(r->toDouble()),
                                                                         static_cast<int>(g->toDouble()),
                                                                         static_cast<int>(b->toDouble()));
                                }
                            }
                        }

                        regexMonoHighlightingColorChanged(mRegexMonoHighlightingColor);
                    }

                    {
                        auto foundHighlightActivePatterns = object.find(sHighlightActivePatternsKey);

                        if(foundHighlightActivePatterns != object.end())
                        {
                            if(true == foundHighlightActivePatterns->isBool())
                            {
                                mbHighlightActivePatterns = foundHighlightActivePatterns->toBool();
                            }
                        }

                        highlightActivePatternsChanged(mbHighlightActivePatterns);
                    }

                    {
                        auto foundPatternsHighlightingColor = object.find(sPatternsHighlightingColorKey);

                        if(foundPatternsHighlightingColor != object.end())
                        {
                            if(true == foundPatternsHighlightingColor->isObject())
                            {
                                QJsonObject obj = foundPatternsHighlightingColor->toObject();

                                auto r = obj.find( sRKey );
                                auto g = obj.find( sGKey );
                                auto b = obj.find( sBKey );

                                if( r != obj.end() && r->isDouble() &&
                                        g != obj.end() && g->isDouble() &&
                                        b != obj.end() && b->isDouble())
                                {
                                    mPatternsHighlightingColor = QColor(static_cast<int>(r->toDouble()),
                                                                        static_cast<int>(g->toDouble()),
                                                                        static_cast<int>(b->toDouble()));
                                }
                            }
                        }

                        patternsHighlightingColorChanged(mPatternsHighlightingColor);
                    }

                    {
                        auto foundSearchResultMonoColorHighlighting = object.find(sSearchResultMonoColorHighlightingKey);

                        if(foundSearchResultMonoColorHighlighting != object.end())
                        {
                            if(true == foundSearchResultMonoColorHighlighting->isBool())
                            {
                                mbSearchResultMonoColorHighlighting = foundSearchResultMonoColorHighlighting->toBool();
                            }
                        }

                        searchResultMonoColorHighlightingChanged(mbSearchResultMonoColorHighlighting);
                    }

                    {
                        auto foundSearchResultHighlightingGradient = object.find(sSearchResultHighlightingGradientKey);

                        if(foundSearchResultHighlightingGradient != object.end())
                        {
                            if(true == foundSearchResultHighlightingGradient->isObject())
                            {
                                QJsonObject obj = foundSearchResultHighlightingGradient->toObject();

                                auto r_1 = obj.find( sRKey + "_1" );
                                auto g_1 = obj.find( sGKey + "_1" );
                                auto b_1 = obj.find( sBKey + "_1" );

                                auto r_2 = obj.find( sRKey + "_2" );
                                auto g_2 = obj.find( sGKey + "_2" );
                                auto b_2 = obj.find( sBKey + "_2" );

                                auto numberOfColors = obj.find( sNumberOfColorsKey );

                                if( r_1 != obj.end() && r_1->isDouble() &&
                                    g_1 != obj.end() && g_1->isDouble() &&
                                    b_1 != obj.end() && b_1->isDouble() &&
                                    r_2 != obj.end() && r_2->isDouble() &&
                                    g_2 != obj.end() && g_2->isDouble() &&
                                    b_2 != obj.end() && b_2->isDouble() &&
                                    numberOfColors->isDouble() )
                                {
                                    mSearchResultHighlightingGradient = tHighlightingGradient(
                                                QColor( static_cast<int>(r_1->toDouble()),
                                                        static_cast<int>(g_1->toDouble()),
                                                        static_cast<int>(b_1->toDouble()) ),
                                                QColor( static_cast<int>(r_2->toDouble()),
                                                        static_cast<int>(g_2->toDouble()),
                                                        static_cast<int>(b_2->toDouble()) ),
                                                static_cast<int>(numberOfColors->toDouble()));
                                }
                            }
                        }

                        searchResultHighlightingGradientChanged(mSearchResultHighlightingGradient);
                    }

                    {
                        auto foundSearchResultColumnsVisibilityMap = object.find(sSearchResultColumnsVisibilityMapKey);
                        if(foundSearchResultColumnsVisibilityMap != object.end())
                        {
                            auto searchResultColumnsVisibilityArrayObj = foundSearchResultColumnsVisibilityMap.value();

                            if(true == searchResultColumnsVisibilityArrayObj.isArray())
                            {
                                mSearchResultColumnsVisibilityMap.clear();

                                auto searchResultColumnsVisibilityArray = searchResultColumnsVisibilityArrayObj.toArray();
                                for( const auto searchResultColumnsVisibilityObj : searchResultColumnsVisibilityArray )
                                {
                                    if(true == searchResultColumnsVisibilityObj.isObject())
                                    {
                                        QJsonObject visibilityObj = searchResultColumnsVisibilityObj.toObject();

                                        if(false == visibilityObj.empty())
                                        {
                                            auto keys = visibilityObj.keys();

                                            for(const auto& key : keys)
                                            {
                                                auto foundVisibilityValue = visibilityObj.find(key);

                                                if(foundVisibilityValue != visibilityObj.end())
                                                {
                                                    if(foundVisibilityValue->isBool())
                                                    {
                                                        bool bConvertionStatus = false;

                                                        int columnIdx = key.toInt(&bConvertionStatus);

                                                        bool bIsVisible = foundVisibilityValue->toBool();
                                                        mSearchResultColumnsVisibilityMap.insert(static_cast<eSearchResultColumn>(columnIdx), bIsVisible);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        searchResultColumnsVisibilityMapChanged(mSearchResultColumnsVisibilityMap);
                    }

                    {
                        auto foundMarkTimeStampWithBold = object.find(sMarkTimeStampWithBold);

                        if(foundMarkTimeStampWithBold != object.end())
                        {
                            if(true == foundMarkTimeStampWithBold->isBool())
                            {
                                mbMarkTimeStampWithBold = foundMarkTimeStampWithBold->toBool();
                            }
                        }

                        markTimeStampWithBoldChanged(mbMarkTimeStampWithBold);
                    }

                    {
                        auto foundPatternsColumnsVisibilityMap = object.find(sPatternsColumnsVisibilityMapKey);
                        if(foundPatternsColumnsVisibilityMap != object.end())
                        {
                            auto patternsColumnsVisibilityArrayObj = foundPatternsColumnsVisibilityMap.value();

                            if(true == patternsColumnsVisibilityArrayObj.isArray())
                            {
                                mPatternsColumnsVisibilityMap.clear();

                                auto patternsColumnsVisibilityArray = patternsColumnsVisibilityArrayObj.toArray();
                                for( const auto patternsColumnsVisibilityObj : patternsColumnsVisibilityArray )
                                {
                                    if(true == patternsColumnsVisibilityObj.isObject())
                                    {
                                        QJsonObject visibilityObj = patternsColumnsVisibilityObj.toObject();

                                        if(false == visibilityObj.empty())
                                        {
                                            auto keys = visibilityObj.keys();

                                            for(const auto& key : keys)
                                            {
                                                auto foundVisibilityValue = visibilityObj.find(key);

                                                if(foundVisibilityValue != visibilityObj.end())
                                                {
                                                    if(foundVisibilityValue->isBool())
                                                    {
                                                        bool bConvertionStatus = false;

                                                        int columnIdx = key.toInt(&bConvertionStatus);

                                                        bool bIsVisible = foundVisibilityValue->toBool();
                                                        mPatternsColumnsVisibilityMap.insert(static_cast<ePatternsColumn>(columnIdx), bIsVisible);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        patternsColumnsVisibilityMapChanged(mPatternsColumnsVisibilityMap);
                    }

                    {
                        auto foundCaseSensitiveRegex = object.find(sCaseSensitiveRegex);

                        if(foundCaseSensitiveRegex != object.end())
                        {
                            if(true == foundCaseSensitiveRegex->isBool())
                            {
                                mbCaseSensitiveRegex = foundCaseSensitiveRegex->toBool();
                            }
                        }

                        caseSensitiveRegexChanged(mbCaseSensitiveRegex);
                    }

                    {
                        auto foundRegexFiltersColumnsVisibilityMap = object.find(sRegexFiltersColumnsVisibilityMapKey);
                        if(foundRegexFiltersColumnsVisibilityMap != object.end())
                        {
                            auto regexFiltersColumnsVisibilityArrayObj = foundRegexFiltersColumnsVisibilityMap.value();

                            if(true == regexFiltersColumnsVisibilityArrayObj.isArray())
                            {
                                mRegexFiltersColumnsVisibilityMap.clear();

                                auto regexFiltersColumnsVisibilityArray = regexFiltersColumnsVisibilityArrayObj.toArray();
                                for( const auto regexFiltersColumnsVisibilityObj : regexFiltersColumnsVisibilityArray )
                                {
                                    if(true == regexFiltersColumnsVisibilityObj.isObject())
                                    {
                                        QJsonObject visibilityObj = regexFiltersColumnsVisibilityObj.toObject();

                                        if(false == visibilityObj.empty())
                                        {
                                            auto keys = visibilityObj.keys();

                                            for(const auto& key : keys)
                                            {
                                                auto foundVisibilityValue = visibilityObj.find(key);

                                                if(foundVisibilityValue != visibilityObj.end())
                                                {
                                                    if(foundVisibilityValue->isBool())
                                                    {
                                                        bool bConvertionStatus = false;

                                                        int columnIdx = key.toInt(&bConvertionStatus);

                                                        bool bIsVisible = foundVisibilityValue->toBool();
                                                        mRegexFiltersColumnsVisibilityMap.insert(static_cast<eRegexFiltersColumn>(columnIdx), bIsVisible);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        regexFiltersColumnsVisibilityMapChanged(mRegexFiltersColumnsVisibilityMap);
                    }

                    {
                        auto foundFilterVariables = object.find(sFilterVariablesKey);

                        if(foundFilterVariables != object.end())
                        {
                            if(true == foundFilterVariables->isBool())
                            {
                                mbFilterVariables = foundFilterVariables->toBool();
                            }
                        }

                        filterVariablesChanged(mbFilterVariables);
                    }

                    {
                        auto foundSelectedRegexFile = object.find(sSelectedRegexFile);
                        if(foundSelectedRegexFile != object.end())
                        {
                            mSelectedRegexFile = foundSelectedRegexFile->toString();

                            if(true == mSelectedRegexFile.isEmpty()) // if stored filepath is empty
                            {
                                mSelectedRegexFile = sDefaultRegexFileName; // let's do a fallback to the default value
                            }
                        }

                        selectedRegexFileChanged(mSelectedRegexFile);
                    }
                }
            }
        }

        result.bResult = true;
    }
    else
    {
        result.bResult = false;
        result.err = QString("[%1] Failed to open file - \"%2\"").arg(__FUNCTION__).arg(filepath);
    }

    return result;
}

// tAliasItem
CSettingsManager::tAliasItem::tAliasItem():
    isDefault(false),
    alias(),
    regex()
{}

CSettingsManager::tAliasItem::tAliasItem(bool isDefault_, const QString& alias_, const QString& regex_):
    isDefault(isDefault_),
    alias(alias_),
    regex(regex_)
{}

// tAliasItem
bool CSettingsManager::tAliasItem::operator==(const tAliasItem& val) const
{
    return isDefault == val.isDefault &&
            alias == val.alias &&
            regex == val.regex;
}

bool CSettingsManager::areAnyDefaultAliasesAvailable() const
{
    bool bResult = false;


    for( const auto& alias : mAliases )
    {
        if( true == alias.isDefault )
        {
            bResult = true;
            break;
        }
    }

    return bResult;
}

void CSettingsManager::resetSearchResultColumnsVisibilityMap()
{
    setSearchResultColumnsVisibilityMap(sDefaultSearchResultColumnsVisibilityMap);
}

void CSettingsManager::resetPatternsColumnsVisibilityMap()
{
    setPatternsColumnsVisibilityMap(sDefaultPatternsColumnsVisibilityMap);
}

void CSettingsManager::resetRegexFiltersColumnsVisibilityMap()
{
    setRegexFiltersColumnsVisibilityMap(sDefaultRegexFiltersColumnsVisibilityMap);
}
