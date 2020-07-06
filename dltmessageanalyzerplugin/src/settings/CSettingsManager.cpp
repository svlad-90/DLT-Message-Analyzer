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

#include "QDebug"

#include "../log/CConsoleCtrl.hpp"

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
static const QString sCacheMaxSizeMBKey = "cacheMaxSizeMB";
static const QString sRDPModeKey = "RDPMode";
static const QString sRegexMonoHighlightingColosKey = "regexMonoHighlightingColor";
static const QString sRKey = "r";
static const QString sGKey = "g";
static const QString sBKey = "b";
static const QString sHighlightActivePatternsKey = "highlightActivePatterns";
static const QString sPatternsHighlightingColorKey = "patternsHighlightingColor";
static const QString sSearchResultMonoColorHighlightingKey = "searchResultMonoColorHighlighting";
static const QString sSearchResultHighlightingGradientKey = "searchResultHighlightingGradient";
static const QString sNumberOfColorsKey = "numberOfColors";
static const QString sSearchResultColumnsVisibilityMapKey = "searchResultColumnsVisibilityMap";
static const QString sSearchResultColumnsCopyPasteMapKey = "searchResultColumnsCopyPasteMap";
static const QString sMarkTimeStampWithBold = "markTimeStampWithBold";
static const QString sPatternsColumnsVisibilityMapKey = "patternsColumnsVisibilityMap";
static const QString sPatternsColumnsCopyPasteMapKey = "patternsColumnsCopyPasteMap";
static const QString sRegexFiltersColumnsVisibilityMapKey = "RegexFiltersColumnsVisibilityMap";
static const QString sFilterVariablesKey = "FilterVariables";
static const QString sCaseSensitiveRegex = "caseSensitiveRegex";
static const QString sSelectedRegexFile = "selectedRegexFile";
static const QString sGroupedViewColumnsVisibilityMapKey = "GroupedViewColumnsVisibilityMap";

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

static tPatternsColumnsVisibilityMap fillInDefaultPatternsColumnsCopyPasteMap()
{
    tPatternsColumnsVisibilityMap result;

    // fields, which are visible by default
    result.insert(ePatternsColumn::AliasTreeLevel, true);
    result.insert(ePatternsColumn::Regex, true);

    // fields, which are not visible by default
    result.insert(ePatternsColumn::Default, false);
    result.insert(ePatternsColumn::Combine, false);

    return result;
}

static const tPatternsColumnsVisibilityMap sDefaultPatternsColumnsCopyPasteMap
= fillInDefaultPatternsColumnsCopyPasteMap();

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

static tGroupedViewColumnsVisibilityMap fillInDefaultGroupedViewColumnsVisibilityMap()
{
    tGroupedViewColumnsVisibilityMap result;

    // fields, which are visible by default
    result.insert(eGroupedViewColumn::SubString, true);
    result.insert(eGroupedViewColumn::Messages, true);
    result.insert(eGroupedViewColumn::MessagesPercantage, true);
    result.insert(eGroupedViewColumn::MessagesPerSecondAverage, true);
    result.insert(eGroupedViewColumn::Payload, true);
    result.insert(eGroupedViewColumn::PayloadPercantage, true);
    result.insert(eGroupedViewColumn::PayloadPerSecondAverage, true);

    return result;
}

static const tGroupedViewColumnsVisibilityMap sDefaultGroupedViewColumnsVisibilityMap
= fillInDefaultGroupedViewColumnsVisibilityMap();

CSettingsManager::CSettingsManager():
    mSetting_SettingsManagerVersion(createIntegralSettingsItem<tSettingsManagerVersion>(sSettingsManagerVersionKey,
        [this](const tSettingsManagerVersion& data){settingsManagerVersionChanged(data);},
        [this](){tryStoreRootConfig();},
        sDefaultSettingsManagerVersion)),
    mSetting_Aliases(createAliasItemVecSettingsItem(sAliasesKey,
        [this](const tAliasItemVec& data){ aliasesChanged(data); },
        [this]()
        {
            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSetting_SelectedRegexFile.getData();
            storeRegexConfigCustomPath(regexSettingsFilePath);
        },
                                                   tAliasItemVec())),
    mSetting_NumberOfThreads(createIntegralSettingsItem<int>(sNumberOfThreadsKey,
        [this](const int& data){numberOfThreadsChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_ContinuousSearch(createBooleanSettingsItem(sIsContinuousSearchKey,
        [this](const bool& data){continuousSearchChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_CopySearchResultAsHTML(createBooleanSettingsItem(sCopySearchResultAsHTMLKey,
        [this](const bool& data){copySearchResultAsHTMLChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_MinimizePatternsViewOnSelection(createBooleanSettingsItem(sMinimizePatternsViewOnSelectionKey,
        [this](const bool& data){minimizePatternsViewOnSelectionChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_WriteSettingsOnEachUpdate(createBooleanSettingsItem(sWriteSettingsOnEachUpdateChangedKey,
        [this](const bool& data){writeSettingsOnEachUpdateChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_CacheEnabled(createBooleanSettingsItem(sCacheEnabledKey,
        [this](const bool& data){cacheEnabledChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_CacheMaxSizeMB(createIntegralSettingsItem<tCacheSizeMB>(sCacheMaxSizeMBKey,
        [this](const tCacheSizeMB& data){cacheMaxSizeMBChanged(data);},
        [this](){tryStoreSettingsConfig();},
        500)),
    mSetting_RDPMode(createBooleanSettingsItem(sRDPModeKey,
        [this](const bool& data){RDPModeChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_RegexMonoHighlightingColor(createColorSettingsItem(sRegexMonoHighlightingColosKey,
        [this](const QColor& data){regexMonoHighlightingColorChanged(data);},
        [this](){tryStoreSettingsConfig();},
        QColor(150,0,0))),
    mSetting_HighlightActivePatterns(createBooleanSettingsItem(sHighlightActivePatternsKey,
        [this](const bool& data){highlightActivePatternsChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_PatternsHighlightingColor(createColorSettingsItem(sPatternsHighlightingColorKey,
        [this](const QColor& data){patternsHighlightingColorChanged(data);},
        [this](){tryStoreSettingsConfig();},
        QColor(0,150,0))),
    mSetting_SearchResultMonoColorHighlighting(createBooleanSettingsItem(sSearchResultMonoColorHighlightingKey,
        [this](const bool& data){searchResultMonoColorHighlightingChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSearchResultHighlightingGradientProtector(),
    mSetting_SearchResultHighlightingGradient(createHighlightingGradientSettingsItem(sSearchResultHighlightingGradientKey,
        [this](const tHighlightingGradient& data){searchResultHighlightingGradientChanged(data);},
        [this](){tryStoreSettingsConfig();},
        tHighlightingGradient(QColor(154,0,146), QColor(1,162,165), 3))),
    mSetting_SearchResultColumnsVisibilityMap(createSearchResultColumnsVisibilityMapSettingsItem(sSearchResultColumnsVisibilityMapKey,
        [this](const tSearchResultColumnsVisibilityMap& data){searchResultColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultSearchResultColumnsVisibilityMap)),
    mSetting_SearchResultColumnsCopyPasteMap(createSearchResultColumnsVisibilityMapSettingsItem(sSearchResultColumnsCopyPasteMapKey,
        [this](const tSearchResultColumnsVisibilityMap& data){searchResultColumnsCopyPasteMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultSearchResultColumnsVisibilityMap)),
    mSetting_MarkTimeStampWithBold(createBooleanSettingsItem(sMarkTimeStampWithBold,
        [this](const bool& data){markTimeStampWithBoldChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_PatternsColumnsVisibilityMap(createPatternsColumnsVisibilityMapSettingsItem(sPatternsColumnsVisibilityMapKey,
        [this](const tPatternsColumnsVisibilityMap& data){patternsColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultPatternsColumnsVisibilityMap)),
    mSetting_PatternsColumnsCopyPasteMap(createPatternsColumnsVisibilityMapSettingsItem(sPatternsColumnsCopyPasteMapKey,
        [this](const tPatternsColumnsVisibilityMap& data){patternsColumnsCopyPasteMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultPatternsColumnsCopyPasteMap)),
    mSetting_CaseSensitiveRegex(createBooleanSettingsItem(sCaseSensitiveRegex,
        [this](const bool& data){caseSensitiveRegexChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_RegexFiltersColumnsVisibilityMap(createRegexFiltersColumnsVisibilityMapSettingsItem(sRegexFiltersColumnsVisibilityMapKey,
        [this](const tRegexFiltersColumnsVisibilityMap& data){regexFiltersColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultRegexFiltersColumnsVisibilityMap)),
    mSetting_FilterVariables(createBooleanSettingsItem(sFilterVariablesKey,
        [this](const bool& data){filterVariablesChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_SelectedRegexFile(createStringSettingsItem(sSelectedRegexFile,
        [this](const QString& data)
        {
            clearRegexConfig();

            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + data;
            loadRegexConfigCustomPath(regexSettingsFilePath);

            selectedRegexFileChanged(data);
        },
        [this](){tryStoreSettingsConfig();},
        sDefaultRegexFileName)),
    mSetting_GroupedViewColumnsVisibilityMap(createGroupedViewColumnsVisibilityMapSettingsItem(sGroupedViewColumnsVisibilityMapKey,
        [this](const tGroupedViewColumnsVisibilityMap& data){groupedViewColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultGroupedViewColumnsVisibilityMap)),
    mRootSettingItemPtrVec(),
    mUserSettingItemPtrVec(),
    mPatternsSettingItemPtrVec(),
    mbRootConfigInitialised(false)
{
    /////////////// ROOT SETTINGS ///////////////
    mRootSettingItemPtrVec.push_back(&mSetting_SettingsManagerVersion);

    /////////////// USER SETTINGS ///////////////
    mUserSettingItemPtrVec.push_back(&mSetting_NumberOfThreads);
    mUserSettingItemPtrVec.push_back(&mSetting_ContinuousSearch);
    mUserSettingItemPtrVec.push_back(&mSetting_CopySearchResultAsHTML);
    mUserSettingItemPtrVec.push_back(&mSetting_MinimizePatternsViewOnSelection);
    mUserSettingItemPtrVec.push_back(&mSetting_WriteSettingsOnEachUpdate);
    mUserSettingItemPtrVec.push_back(&mSetting_CacheEnabled);
    mUserSettingItemPtrVec.push_back(&mSetting_CacheMaxSizeMB);
    mUserSettingItemPtrVec.push_back(&mSetting_RDPMode);
    mUserSettingItemPtrVec.push_back(&mSetting_RegexMonoHighlightingColor);
    mUserSettingItemPtrVec.push_back(&mSetting_HighlightActivePatterns);
    mUserSettingItemPtrVec.push_back(&mSetting_PatternsHighlightingColor);
    mUserSettingItemPtrVec.push_back(&mSetting_SearchResultMonoColorHighlighting);
    mUserSettingItemPtrVec.push_back(&mSetting_SearchResultHighlightingGradient);
    mUserSettingItemPtrVec.push_back(&mSetting_SearchResultColumnsVisibilityMap);
    mUserSettingItemPtrVec.push_back(&mSetting_SearchResultColumnsCopyPasteMap);
    mUserSettingItemPtrVec.push_back(&mSetting_MarkTimeStampWithBold);
    mUserSettingItemPtrVec.push_back(&mSetting_PatternsColumnsVisibilityMap);
    mUserSettingItemPtrVec.push_back(&mSetting_PatternsColumnsCopyPasteMap);
    mUserSettingItemPtrVec.push_back(&mSetting_CaseSensitiveRegex);
    mUserSettingItemPtrVec.push_back(&mSetting_RegexFiltersColumnsVisibilityMap);
    mUserSettingItemPtrVec.push_back(&mSetting_FilterVariables);
    mUserSettingItemPtrVec.push_back(&mSetting_SelectedRegexFile);
    mUserSettingItemPtrVec.push_back(&mSetting_GroupedViewColumnsVisibilityMap);

    /////////////// PATTERNS SETTINGS ///////////////
    mPatternsSettingItemPtrVec.push_back(&mSetting_Aliases);

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

void CSettingsManager::tryStoreSettingsConfig()
{
    if(true == getWriteSettingsOnEachUpdate())
    {
        storeSettingsConfig();
    }
}

void CSettingsManager::tryStoreRootConfig()
{
    if(true == getWriteSettingsOnEachUpdate())
    {
        storeRootConfig();
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

TSettingItem<bool> CSettingsManager::createBooleanSettingsItem(const QString& key,
                                                               const TSettingItem<bool>::tUpdateDataFunc& updateDataFunc,
                                                               const TSettingItem<bool>::tUpdateSettingsFileFunc& updateFileFunc,
                                                               const bool& defaultValue) const
{
    auto writeFunc = [&key](const bool& value)->QJsonObject
    {
        QJsonObject result;
        result.insert( key, QJsonValue( value ) );
        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem, bool& data, const bool&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isBool())
        {
            data = JSONItem.toBool();
            bResult = true;
        }

        return bResult;
    };

    return TSettingItem<bool>(key,
                              defaultValue,
                              writeFunc,
                              readFunc,
                              updateDataFunc,
                              updateFileFunc);
}

TSettingItem<QColor> CSettingsManager::createColorSettingsItem(const QString& key,
                                             const TSettingItem<QColor>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<QColor>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const QColor& defaultValue) const
{
    auto writeFunc = [&key](const QColor& value)->QJsonObject
    {
        QJsonObject JSONColor;

        JSONColor.insert(sRKey, value.red());
        JSONColor.insert(sGKey, value.green());
        JSONColor.insert(sBKey, value.blue());

        QJsonObject result;
        result.insert( key, QJsonValue( JSONColor ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem, QColor& data, const QColor&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isObject())
        {
            QJsonObject obj = JSONItem.toObject();

            auto r = obj.find( sRKey );
            auto g = obj.find( sGKey );
            auto b = obj.find( sBKey );

            if( r != obj.end() && r->isDouble() &&
                g != obj.end() && g->isDouble() &&
                b != obj.end() && b->isDouble())
            {
                data = QColor(static_cast<int>(r->toDouble()),
                              static_cast<int>(g->toDouble()),
                              static_cast<int>(b->toDouble()));

                bResult = true;
            }
        }

        return bResult;
    };

    return TSettingItem<QColor>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<tHighlightingGradient> CSettingsManager::createHighlightingGradientSettingsItem(const QString& key,
                                             const TSettingItem<tHighlightingGradient>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tHighlightingGradient>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tHighlightingGradient& defaultValue) const
{
    auto writeFunc = [&key](const tHighlightingGradient& value)->QJsonObject
    {
        QJsonObject highlightGradientJSONObject;

        highlightGradientJSONObject.insert(sRKey + "_1", value.from.red());
        highlightGradientJSONObject.insert(sGKey + "_1", value.from.green());
        highlightGradientJSONObject.insert(sBKey + "_1", value.from.blue());
        highlightGradientJSONObject.insert(sRKey + "_2", value.to.red());
        highlightGradientJSONObject.insert(sGKey + "_2", value.to.green());
        highlightGradientJSONObject.insert(sBKey + "_2", value.to.blue());

        highlightGradientJSONObject.insert(sNumberOfColorsKey, value.numberOfColors);

        QJsonObject result;
        result.insert( key, QJsonValue( highlightGradientJSONObject ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tHighlightingGradient& data,
                       const tHighlightingGradient&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isObject())
        {
            QJsonObject obj = JSONItem.toObject();

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
                data = tHighlightingGradient(
                            QColor( static_cast<int>(r_1->toDouble()),
                                    static_cast<int>(g_1->toDouble()),
                                    static_cast<int>(b_1->toDouble()) ),
                            QColor( static_cast<int>(r_2->toDouble()),
                                    static_cast<int>(g_2->toDouble()),
                                    static_cast<int>(b_2->toDouble()) ),
                            static_cast<int>(numberOfColors->toDouble()));
            }
        }

        return bResult;
    };

    return TSettingItem<tHighlightingGradient>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<CSettingsManager::tAliasItemVec> CSettingsManager::createAliasItemVecSettingsItem(const QString& key,
                                             const TSettingItem<tAliasItemVec>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tAliasItemVec>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tAliasItemVec& defaultValue) const
{
    auto writeFunc = [&key](const tAliasItemVec& value)->QJsonObject
    {
        QJsonObject aliases;
        QJsonArray arrayAliases;
        for(const auto& item : value)
        {
            QJsonObject obj;
            obj.insert( sIsDefaultKey, QJsonValue( item.isDefault ) );
            obj.insert( sAliasKey, QJsonValue( item.alias ) );
            obj.insert( sRegexKey, QJsonValue( item.regex ) );
            arrayAliases.append( obj );
        }

        QJsonObject result;
        result.insert( key, QJsonValue( arrayAliases ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tAliasItemVec& data,
                       const tAliasItemVec&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isArray())
        {
            data.clear();

            auto aliasesArray = JSONItem.toArray();

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

                            data.push_back(tAliasItem(bIsDefault, alias->toString(), regex->toString()));
                        }
                    }
                }

                bResult = true;
            }
        }

        return bResult;
    };

    return TSettingItem<tAliasItemVec>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<tSearchResultColumnsVisibilityMap> CSettingsManager::createSearchResultColumnsVisibilityMapSettingsItem(const QString& key,
                                             const TSettingItem<tSearchResultColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tSearchResultColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tSearchResultColumnsVisibilityMap& defaultValue) const
{
    auto writeFunc = [&key](const tSearchResultColumnsVisibilityMap& value)->QJsonObject
    {
        QJsonArray arraySearchResultColumnsVisibilityItems;
        for(auto it = value.begin(); it != value.end(); ++it)
        {
            QJsonObject obj;
            obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
            arraySearchResultColumnsVisibilityItems.append( obj );
        }

        QJsonObject result;
        result.insert( key, QJsonValue( arraySearchResultColumnsVisibilityItems ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tSearchResultColumnsVisibilityMap& data,
                       const tSearchResultColumnsVisibilityMap&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isArray())
        {
            data.clear();

            auto searchResultColumnsVisibilityArray = JSONItem.toArray();
            for( const auto searchResultColumnsVisibilityObj : searchResultColumnsVisibilityArray )
            {
                if(true == searchResultColumnsVisibilityObj.isObject())
                {
                    QJsonObject visibilityObj = searchResultColumnsVisibilityObj.toObject();

                    if(false == visibilityObj.empty())
                    {
                        auto visibilityKeys = visibilityObj.keys();

                        for(const auto& visibilityKey : visibilityKeys)
                        {
                            auto foundVisibilityValue = visibilityObj.find(visibilityKey);

                            if(foundVisibilityValue != visibilityObj.end())
                            {
                                if(foundVisibilityValue->isBool())
                                {
                                    bool bConvertionStatus = false;

                                    int columnIdx = visibilityKey.toInt(&bConvertionStatus);

                                    bool bIsVisible = foundVisibilityValue->toBool();
                                    data.insert(static_cast<eSearchResultColumn>(columnIdx), bIsVisible);
                                }
                            }
                        }
                    }

                    bResult = true;
                }
            }
        }

        return bResult;
    };

    return TSettingItem<tSearchResultColumnsVisibilityMap>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<tPatternsColumnsVisibilityMap> CSettingsManager::createPatternsColumnsVisibilityMapSettingsItem(const QString& key,
                                             const TSettingItem<tPatternsColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tPatternsColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tPatternsColumnsVisibilityMap& defaultValue) const
{
    auto writeFunc = [&key](const tPatternsColumnsVisibilityMap& value)->QJsonObject
    {
        QJsonArray arrayPatternsColumnsVisibilityItems;
        for(auto it = value.begin(); it != value.end(); ++it)
        {
            QJsonObject obj;
            obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
            arrayPatternsColumnsVisibilityItems.append( obj );
        }

        QJsonObject result;
        result.insert( key, QJsonValue( arrayPatternsColumnsVisibilityItems ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tPatternsColumnsVisibilityMap& data,
                       const tPatternsColumnsVisibilityMap&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isArray())
        {
            data.clear();

            auto patternsColumnsVisibilityArray = JSONItem.toArray();
            for( const auto patternsColumnsVisibilityObj : patternsColumnsVisibilityArray )
            {
                if(true == patternsColumnsVisibilityObj.isObject())
                {
                    QJsonObject visibilityObj = patternsColumnsVisibilityObj.toObject();

                    if(false == visibilityObj.empty())
                    {
                        auto visibilityKeys = visibilityObj.keys();

                        for(const auto& visibilityKey : visibilityKeys)
                        {
                            auto foundVisibilityValue = visibilityObj.find(visibilityKey);

                            if(foundVisibilityValue != visibilityObj.end())
                            {
                                if(foundVisibilityValue->isBool())
                                {
                                    bool bConvertionStatus = false;

                                    int columnIdx = visibilityKey.toInt(&bConvertionStatus);

                                    bool bIsVisible = foundVisibilityValue->toBool();
                                    data.insert(static_cast<ePatternsColumn>(columnIdx), bIsVisible);
                                }
                            }
                        }
                    }
                }
            }

            bResult = true;
        }

        return bResult;
    };

    return TSettingItem<tPatternsColumnsVisibilityMap>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<tGroupedViewColumnsVisibilityMap> CSettingsManager::createGroupedViewColumnsVisibilityMapSettingsItem(const QString& key,
                                             const TSettingItem<tGroupedViewColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tGroupedViewColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tGroupedViewColumnsVisibilityMap& defaultValue) const
{
    auto writeFunc = [&key](const tGroupedViewColumnsVisibilityMap& value)->QJsonObject
    {
        QJsonArray arrayGroupedViewColumnsVisibilityItems;
        for(auto it = value.begin(); it != value.end(); ++it)
        {
            QJsonObject obj;
            obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
            arrayGroupedViewColumnsVisibilityItems.append( obj );
        }

        QJsonObject result;
        result.insert( key, QJsonValue( arrayGroupedViewColumnsVisibilityItems ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tGroupedViewColumnsVisibilityMap& data,
                       const tGroupedViewColumnsVisibilityMap&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isArray())
        {
            data.clear();

            auto groupedViewColumnsVisibilityArray = JSONItem.toArray();
            for( const auto groupedViewColumnsVisibilityObj : groupedViewColumnsVisibilityArray )
            {
                if(true == groupedViewColumnsVisibilityObj.isObject())
                {
                    QJsonObject visibilityObj = groupedViewColumnsVisibilityObj.toObject();

                    if(false == visibilityObj.empty())
                    {
                        auto visibilityKeys = visibilityObj.keys();

                        for(const auto& visibilityKey : visibilityKeys)
                        {
                            auto foundVisibilityValue = visibilityObj.find(visibilityKey);

                            if(foundVisibilityValue != visibilityObj.end())
                            {
                                if(foundVisibilityValue->isBool())
                                {
                                    bool bConvertionStatus = false;

                                    int columnIdx = visibilityKey.toInt(&bConvertionStatus);

                                    bool bIsVisible = foundVisibilityValue->toBool();
                                    data.insert(static_cast<eGroupedViewColumn>(columnIdx), bIsVisible);
                                }
                            }
                        }
                    }
                }
            }

            bResult = true;
        }

        return bResult;
    };

    return TSettingItem<tGroupedViewColumnsVisibilityMap>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<tRegexFiltersColumnsVisibilityMap> CSettingsManager::createRegexFiltersColumnsVisibilityMapSettingsItem(const QString& key,
                                             const TSettingItem<tRegexFiltersColumnsVisibilityMap>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tRegexFiltersColumnsVisibilityMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tRegexFiltersColumnsVisibilityMap& defaultValue) const
{
    auto writeFunc = [&key](const tRegexFiltersColumnsVisibilityMap& value)->QJsonObject
    {
        QJsonObject regexFiltersColumnsVisibilityMap;
        QJsonArray arrayRegexFiltersColumnsVisibilityItems;
        for(auto it = value.begin(); it != value.end(); ++it)
        {
            QJsonObject obj;
            obj.insert( QString::number( static_cast<int>(it.key()) ), QJsonValue( it.value() ) );
            arrayRegexFiltersColumnsVisibilityItems.append( obj );
        }

        QJsonObject result;
        result.insert( key, QJsonValue( arrayRegexFiltersColumnsVisibilityItems ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tRegexFiltersColumnsVisibilityMap& data,
                       const tRegexFiltersColumnsVisibilityMap&)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isArray())
        {
            data.clear();

            auto regexFiltersColumnsVisibilityArray = JSONItem.toArray();
            for( const auto regexFiltersColumnsVisibilityObj : regexFiltersColumnsVisibilityArray )
            {
                if(true == regexFiltersColumnsVisibilityObj.isObject())
                {
                    QJsonObject visibilityObj = regexFiltersColumnsVisibilityObj.toObject();

                    if(false == visibilityObj.empty())
                    {
                        auto visibilityKeys = visibilityObj.keys();

                        for(const auto& visibilityKey : visibilityKeys)
                        {
                            auto foundVisibilityValue = visibilityObj.find(visibilityKey);

                            if(foundVisibilityValue != visibilityObj.end())
                            {
                                if(foundVisibilityValue->isBool())
                                {
                                    bool bConvertionStatus = false;

                                    int columnIdx = visibilityKey.toInt(&bConvertionStatus);

                                    bool bIsVisible = foundVisibilityValue->toBool();
                                    data.insert(static_cast<eRegexFiltersColumn>(columnIdx), bIsVisible);
                                }
                            }
                        }
                    }
                }
            }

            bResult = true;
        }

        return bResult;
    };

    return TSettingItem<tRegexFiltersColumnsVisibilityMap>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<QString> CSettingsManager::createStringSettingsItem(const QString& key,
                                             const TSettingItem<QString>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<QString>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const QString& defaultValue) const
{
    auto writeFunc = [&key](const QString& value)->QJsonObject
    {
        QJsonObject result;
        result.insert( key, QJsonValue( value ) );
        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       QString& data,
                       const QString& defaultValue_)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isString())
        {
            data = JSONItem.toString();

            if(true == data.isEmpty()) // if stored filepath is empty
            {
                data = defaultValue_; // let's do a fallback to the default value
            }

            bResult = true;
        }

        return bResult;
    };

    return TSettingItem<QString>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
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

            for(auto* pSetting : mRootSettingItemPtrVec)
            {
                pSetting->readDataFromArray(arrayRows);
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

        for(auto* pSetting : mRootSettingItemPtrVec)
        {
            settingsArray.append(pSetting->writeData());
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
            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSetting_SelectedRegexFile.getData();
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
            if(false == mSetting_SelectedRegexFile.getData().isEmpty())
            {
                QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSetting_SelectedRegexFile.getData();
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
                    mSetting_SettingsManagerVersion.setDataSilent(1);
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
                                        mSetting_SelectedRegexFile.setDataSilent( sDefaultRegexFileName );

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
    mSetting_SettingsManagerVersion.setData(val);
}

void CSettingsManager::setAliases(const tAliasItemVec& val)
{
    mSetting_Aliases.setData(val);
}

void CSettingsManager::setAliasIsDefault(const QString& alias, bool isDefault)
{
    auto aliases = mSetting_Aliases.getData();

    auto foundAlias = std::find_if( aliases.begin(), aliases.end(), [&alias](const tAliasItem& aliasItem)->bool
    {
        return aliasItem.alias == alias;
    });

    if(foundAlias != aliases.end())
    {
        foundAlias->isDefault = isDefault;
    }

    mSetting_Aliases.setData(aliases);
}

void CSettingsManager::setNumberOfThreads(const int& val)
{
    mSetting_NumberOfThreads.setData(val);
}

void CSettingsManager::setContinuousSearch(bool val)
{
    mSetting_ContinuousSearch.setData(val);
}

void CSettingsManager::setCopySearchResultAsHTML(bool val)
{
    mSetting_CopySearchResultAsHTML.setData(val);
}

void CSettingsManager::setMinimizePatternsViewOnSelection(bool val)
{
    mSetting_MinimizePatternsViewOnSelection.setData(val);
}

void CSettingsManager::setWriteSettingsOnEachUpdate(bool val)
{
    mSetting_WriteSettingsOnEachUpdate.setData(val);
}

void CSettingsManager::setCacheEnabled(bool val)
{
    mSetting_CacheEnabled.setData(val);
}

void CSettingsManager::setCacheMaxSizeMB(tCacheSizeMB val)
{
    mSetting_CacheMaxSizeMB.setData(val);
}

void CSettingsManager::setRDPMode(bool val)
{
    mSetting_RDPMode.setData(val);
}

void CSettingsManager::setRegexMonoHighlightingColor(const QColor& val)
{
    mSetting_RegexMonoHighlightingColor.setData(val);
}

void CSettingsManager::setHighlightActivePatterns(bool val)
{
    mSetting_HighlightActivePatterns.setData(val);
}

void CSettingsManager::setPatternsHighlightingColor(const QColor& val)
{
    mSetting_PatternsHighlightingColor.setData(val);
}

void CSettingsManager::setSearchResultMonoColorHighlighting(bool val)
{
    mSetting_SearchResultMonoColorHighlighting.setData(val);
}

void CSettingsManager::setSearchResultHighlightingGradient(const tHighlightingGradient& val)
{
    std::lock_guard<std::recursive_mutex> lock(mSearchResultHighlightingGradientProtector);
    mSetting_SearchResultHighlightingGradient.setData(val);
}

void CSettingsManager::setSearchResultColumnsVisibilityMap(const tSearchResultColumnsVisibilityMap& val)
{
    mSetting_SearchResultColumnsVisibilityMap.setData(val);
}

void CSettingsManager::setSearchResultColumnsCopyPasteMap(const tSearchResultColumnsVisibilityMap& val)
{
    mSetting_SearchResultColumnsCopyPasteMap.setData(val);
}

void CSettingsManager::setMarkTimeStampWithBold(bool val)
{
    mSetting_MarkTimeStampWithBold.setData(val);
}

void CSettingsManager::setPatternsColumnsVisibilityMap(const tPatternsColumnsVisibilityMap& val)
{
    mSetting_PatternsColumnsVisibilityMap.setData(val);
}

void CSettingsManager::setPatternsColumnsCopyPasteMap(const tPatternsColumnsVisibilityMap& val)
{
    mSetting_PatternsColumnsCopyPasteMap.setData(val);
}

void CSettingsManager::setCaseSensitiveRegex(bool val)
{
    mSetting_CaseSensitiveRegex.setData(val);
}

void CSettingsManager::setRegexFiltersColumnsVisibilityMap(const tRegexFiltersColumnsVisibilityMap& val)
{
    mSetting_RegexFiltersColumnsVisibilityMap.setData(val);
}

void CSettingsManager::setFilterVariables(bool val)
{
    mSetting_FilterVariables.setData(val);
}

void CSettingsManager::setGroupedViewColumnsVisibilityMap(const tGroupedViewColumnsVisibilityMap& val)
{
    mSetting_GroupedViewColumnsVisibilityMap.setData(val);
}

void CSettingsManager::setSelectedRegexFile(const QString& val)
{
    mSetting_SelectedRegexFile.setData(val);
}

const tSettingsManagerVersion& CSettingsManager::getSettingsManagerVersion() const
{
    return mSetting_SettingsManagerVersion.getData();
}

const CSettingsManager::tAliasItemVec& CSettingsManager::getAliases() const
{
    return mSetting_Aliases.getData();
}

const int& CSettingsManager::getNumberOfThreads() const
{
    return mSetting_NumberOfThreads.getData();
}

bool CSettingsManager::getContinuousSearch() const
{
    return mSetting_ContinuousSearch.getData();
}

bool CSettingsManager::getCopySearchResultAsHTML() const
{
    return mSetting_CopySearchResultAsHTML.getData();
}

bool CSettingsManager::getMinimizePatternsViewOnSelection() const
{
    return mSetting_MinimizePatternsViewOnSelection.getData();
}

bool CSettingsManager::getWriteSettingsOnEachUpdate()const
{
    return mSetting_WriteSettingsOnEachUpdate.getData();
}

bool CSettingsManager::getCacheEnabled() const
{
    return mSetting_CacheEnabled.getData();
}

const tCacheSizeMB& CSettingsManager::getCacheMaxSizeMB() const
{
    return mSetting_CacheMaxSizeMB.getData();
}

bool CSettingsManager::getRDPMode() const
{
    return mSetting_RDPMode.getData();
}

const QColor& CSettingsManager::getRegexMonoHighlightingColor() const
{
    return mSetting_RegexMonoHighlightingColor.getData();
}

bool CSettingsManager::getHighlightActivePatterns() const
{
    return mSetting_HighlightActivePatterns.getData();
}

const QColor& CSettingsManager::getPatternsHighlightingColor() const
{
    return mSetting_PatternsHighlightingColor.getData();
}

bool CSettingsManager::getSearchResultMonoColorHighlighting() const
{
    return mSetting_SearchResultMonoColorHighlighting.getData();
}

tHighlightingGradient CSettingsManager::getSearchResultHighlightingGradient() const
{
   std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mSearchResultHighlightingGradientProtector));
   return mSetting_SearchResultHighlightingGradient.getData();
}

const tSearchResultColumnsVisibilityMap& CSettingsManager::getSearchResultColumnsVisibilityMap() const
{
    return mSetting_SearchResultColumnsVisibilityMap.getData();
}

const tSearchResultColumnsVisibilityMap& CSettingsManager::getSearchResultColumnsCopyPasteMap() const
{
    return mSetting_SearchResultColumnsCopyPasteMap.getData();
}

bool CSettingsManager::getMarkTimeStampWithBold() const
{
    return mSetting_MarkTimeStampWithBold.getData();
}

const tPatternsColumnsVisibilityMap& CSettingsManager::getPatternsColumnsVisibilityMap() const
{
    return mSetting_PatternsColumnsVisibilityMap.getData();
}

const tPatternsColumnsVisibilityMap& CSettingsManager::getPatternsColumnsCopyPasteMap() const
{
    return mSetting_PatternsColumnsCopyPasteMap.getData();
}

bool CSettingsManager::getCaseSensitiveRegex() const
{
    return mSetting_CaseSensitiveRegex.getData();
}

const tRegexFiltersColumnsVisibilityMap& CSettingsManager::getRegexFiltersColumnsVisibilityMap() const
{
    return mSetting_RegexFiltersColumnsVisibilityMap.getData();
}

bool CSettingsManager::getFilterVariables() const
{
    return mSetting_FilterVariables.getData();
}

QString CSettingsManager::getSelectedRegexFile() const
{
    return mSetting_SelectedRegexFile.getData();
}

 const tGroupedViewColumnsVisibilityMap& CSettingsManager::getGroupedViewColumnsVisibilityMap() const
 {
     return mSetting_GroupedViewColumnsVisibilityMap.getData();
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
    mSetting_Aliases.setDataSilent(tAliasItemVec());
    aliasesChanged(mSetting_Aliases.getData());
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

            for(auto* pSettingItem : mPatternsSettingItemPtrVec)
            {
                pSettingItem->readDataFromArray(arrayRows);
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

        for(auto* pSettingItem : mPatternsSettingItemPtrVec)
        {
            settingsArray.append(pSettingItem->writeData());
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

        for( auto* pSetting : mUserSettingItemPtrVec )
        {
            if(pSetting == &mSetting_SearchResultHighlightingGradient)
            {
                std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mSearchResultHighlightingGradientProtector));
                settingsArray.append(pSetting->writeData());
            }
            else
            {
                settingsArray.append(pSetting->writeData());
            }
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

            for( auto* pSetting : mUserSettingItemPtrVec )
            {
                if(pSetting == &mSetting_SearchResultHighlightingGradient)
                {
                    std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mSearchResultHighlightingGradientProtector));
                    pSetting->readDataFromArray(arrayRows);
                }
                else
                {
                    pSetting->readDataFromArray(arrayRows);
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

    const auto& aliases = mSetting_Aliases.getData();

    for( const auto& alias : aliases )
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

void CSettingsManager::resetSearchResultColumnsCopyPasteMap()
{
    setSearchResultColumnsCopyPasteMap(sDefaultSearchResultColumnsVisibilityMap);
}

void CSettingsManager::resetPatternsColumnsVisibilityMap()
{
    setPatternsColumnsVisibilityMap(sDefaultPatternsColumnsVisibilityMap);
}

void CSettingsManager::resetPatternsColumnsCopyPasteMap()
{
    setPatternsColumnsCopyPasteMap(sDefaultPatternsColumnsCopyPasteMap);
}

void CSettingsManager::resetRegexFiltersColumnsVisibilityMap()
{
    setRegexFiltersColumnsVisibilityMap(sDefaultRegexFiltersColumnsVisibilityMap);
}

void CSettingsManager::resetGroupedViewColumnsVisibilityMap()
{
    setGroupedViewColumnsVisibilityMap(sDefaultGroupedViewColumnsVisibilityMap);
}
