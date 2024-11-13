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
#include "QCoreApplication"
#include <QThread>
#include "QDebug"
#include <QStandardPaths>

#include "components/log/api/CLog.hpp"
#include "common/OSHelper.hpp"
#include "CSettingsManager.hpp"

#include "DMA_Plantuml.hpp"

static const QString sSettingsManager_Directory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator() + ".DLT-Message-Analyzer";
static const QString sSettingsManager_Regex_SubDirectory = "regexes";
static const QString sSettingsManager_RegexUsageStatistics_SubDirectory = "regex_usage_statistics";
static const QString sSettingsManager_User_SettingsFile = "user_settings.json";
static const QString sSettingsManager_Root_SettingsFile = "root_settings.json";

static const QString sSettingsManagerVersionKey = "settingsManagerVersion";
static const QString sAliasesKey = "aliases";
static const QString sRegexUsageStatisticsKey = "regexUsageStatistics";
static const QString sUsernameKey = "username";
static const QString sRegexInputFieldHeight = "regexInputFieldHeight";
static const QString sAliasKey = "alias";
static const QString sRegexKey = "regex";
static const QString sIsDefaultKey = "isDefault";
static const QString sUsageCounterKey = "usageCounter";
static const QString sUpdateDateTimeKey = "updateDateTimeKey";
static const QString sRegexUsageStatisticsItemTypeKey = "regexUsageStatisticsItemType";
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
static const QString sSearchResultColumnsSearchMapKey = "searchResultColumnsSearchMap";
static const QString sMarkTimeStampWithBold = "markTimeStampWithBold";
static const QString sPatternsColumnsVisibilityMapKey = "patternsColumnsVisibilityMap";
static const QString sPatternsColumnsCopyPasteMapKey = "patternsColumnsCopyPasteMap";
static const QString sRegexFiltersColumnsVisibilityMapKey = "RegexFiltersColumnsVisibilityMap";
static const QString sFilterVariablesKey = "FilterVariables";
static const QString sCaseSensitiveRegex = "caseSensitiveRegex";
static const QString sSelectedRegexFile = "selectedRegexFile";
static const QString sGroupedViewColumnsVisibilityMapKey = "GroupedViewColumnsVisibilityMap";
static const QString sGroupedViewColumnsCopyPasteMapKey = "GroupedViewColumnsCopyPasteMap";
static const QString sSubFilesHandlingStatusKey = "SubFilesHandlingStatus";
static const QString sFont_SearchView = "FontSearchView";

static const QString sUML_FeatureActiveKey = "UML_FeatureActive";
static const QString sUML_MaxNumberOfRowsInDiagramKey = "UML_MaxNumberOfRowsInDiagram";
static const QString sUML_ShowArgumentsKey = "UML_ShowArguments";
static const QString sUML_WrapOutputKey = "UML_WrapOutput";
static const QString sUML_AutonumberKey = "UML_Autonumber";

static const QString sPlotViewFeatureActiveKey = "PlotViewFeatureActive";

static const QString sFiltersCompletion_CaseSensitiveKey = "FiltersCompletion_CaseSensitive";
static const QString sFiltersCompletion_MaxNumberOfSuggestionsKey = "FiltersCompletion_MaxNumberOfSuggestions";
static const QString sFiltersCompletion_MaxCharactersInSuggestionKey = "FiltersCompletion_MaxCharactersInSuggestion";
static const QString sFiltersCompletion_CompletionPopUpWidthKey = "FiltersCompletion_CompletionPopUpWidth";
static const QString sFiltersCompletion_SearchPolicyKey = "FiltersCompletion_SearchPolicy";

static const QString sSearchViewLastColumnWidthStrategyKey = "SearchViewLastColumnWidthStrategy";
static const QString sPlantumlPathMode = "PlantumlPathMode";
static const QString sPlantumlPathEnvVar = "PlantumlPathEnvVar";
static const QString sPlantumlCustomPath = "PlantumlCustomPath";
static const QString sJavaPathMode = "JavaPathMode";
static const QString sJavaPathEnvVar = "JavaPathEnvVar";
static const QString sJavaCustomPath = "JavaCustomPath";

static const QString sGroupedViewFeatureActive = "GroupedViewFeatureActive";

static const QString sRegexCompletion_CaseSensitiveKey = "RegexCompletion_CaseSensitive";
static const QString sRegexCompletion_SearchPolicyKey = "RegexCompletion_SearchPolicy";

static const tSettingsManagerVersion sDefaultSettingsManagerVersion = static_cast<tSettingsManagerVersion>(-1);
static const tSettingsManagerVersion sCurrentSettingsManagerVersion = 2u; // current version of settings manager used by SW.

static QString getCurrentUserName()
{
#ifdef Q_OS_WIN
    // On Windows, use USERNAME
    QString username = QString::fromLocal8Bit(qgetenv("USERNAME"));
#else
    // On Linux/macOS, use USER
    QString username = QString::fromLocal8Bit(qgetenv("USER"));
#endif

    // Handle empty username (in case the environment variable is missing)
    if (username.isEmpty())
    {
        username = "UnknownUser";  // Fallback value
    }

    return username;
}

static tSearchResultColumnsVisibilityMap fillInDefaultSearchResultColumnsVisibilityMap()
{
    tSearchResultColumnsVisibilityMap result;

    // fields, which are visible by default
    result.insert(eSearchResultColumn::UML_Applicability, true);
    result.insert(eSearchResultColumn::PlotView_Applicability, true);
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

static tSearchResultColumnsVisibilityMap fillInDefaultSearchResultCopyPasteMap()
{
    tSearchResultColumnsVisibilityMap result;

    // fields, which are copied by default
    result.insert(eSearchResultColumn::Index, true);
    result.insert(eSearchResultColumn::Timestamp, true);
    result.insert(eSearchResultColumn::Ecuid, true);
    result.insert(eSearchResultColumn::Apid, true);
    result.insert(eSearchResultColumn::Ctid, true);
    result.insert(eSearchResultColumn::Payload, true);

    // fields, which are not copied by default
    result.insert(eSearchResultColumn::UML_Applicability, false);
    result.insert(eSearchResultColumn::PlotView_Applicability, false);
    result.insert(eSearchResultColumn::Time, false);
    result.insert(eSearchResultColumn::Count, false);
    result.insert(eSearchResultColumn::SessionId, false);
    result.insert(eSearchResultColumn::Type, false);
    result.insert(eSearchResultColumn::Subtype, false);
    result.insert(eSearchResultColumn::Mode, false);
    result.insert(eSearchResultColumn::Args, false);

    return result;
}

static const tSearchResultColumnsVisibilityMap sDefaultSearchResultColumnsCopyPasteMap
= fillInDefaultSearchResultCopyPasteMap();

static tSearchResultColumnsVisibilityMap fillInDefaultSearchResultSearchMap()
{
    tSearchResultColumnsVisibilityMap result;

    // fields, which are searched by default
    result.insert(eSearchResultColumn::Apid, true);
    result.insert(eSearchResultColumn::Ctid, true);
    result.insert(eSearchResultColumn::Payload, true);

    // fields, which are not searched by default
    result.insert(eSearchResultColumn::Index, false);
    result.insert(eSearchResultColumn::Timestamp, false);
    result.insert(eSearchResultColumn::Ecuid, false);
    result.insert(eSearchResultColumn::UML_Applicability, false);
    result.insert(eSearchResultColumn::PlotView_Applicability, false);
    result.insert(eSearchResultColumn::Time, false);
    result.insert(eSearchResultColumn::Count, false);
    result.insert(eSearchResultColumn::SessionId, false);
    result.insert(eSearchResultColumn::Type, false);
    result.insert(eSearchResultColumn::Subtype, false);
    result.insert(eSearchResultColumn::Mode, false);
    result.insert(eSearchResultColumn::Args, false);

    return result;
}

static const tSearchResultColumnsVisibilityMap sDefaultSearchResultColumnsSearchMap
    = fillInDefaultSearchResultSearchMap();

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
    mSetting_SettingsManagerVersion(createArithmeticSettingsItem<tSettingsManagerVersion>(sSettingsManagerVersionKey,
        [this](const tSettingsManagerVersion&,
               const tSettingsManagerVersion& data){settingsManagerVersionChanged(data);},
        [this](){tryStoreRootConfig();},
        sDefaultSettingsManagerVersion)),
    mSetting_Aliases(createAliasItemMapSettingsItem(sAliasesKey,
        [this](const tAliasItemMap&, const tAliasItemMap& data){ aliasesChanged(data); },
        [this]()
        {
            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + mSetting_SelectedRegexFile.getData();
            storeRegexConfigCustomPath(regexSettingsFilePath);
        },
                                                   tAliasItemMap())),
    mSetting_NumberOfThreads(createRangedArithmeticSettingsItem<int>(sNumberOfThreadsKey,
        [this](const int&, const int& data){numberOfThreadsChanged(data);},
        [this](){tryStoreSettingsConfig();},
        TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(1, QThread::idealThreadCount())),
        1)),
    mSetting_ContinuousSearch(createBooleanSettingsItem(sIsContinuousSearchKey,
        [this](const bool&, const bool& data){continuousSearchChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_CopySearchResultAsHTML(createBooleanSettingsItem(sCopySearchResultAsHTMLKey,
        [this](const bool&, const bool& data){copySearchResultAsHTMLChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_MinimizePatternsViewOnSelection(createBooleanSettingsItem(sMinimizePatternsViewOnSelectionKey,
        [this](const bool&, const bool& data){minimizePatternsViewOnSelectionChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_WriteSettingsOnEachUpdate(createBooleanSettingsItem(sWriteSettingsOnEachUpdateChangedKey,
        [this](const bool&, const bool& data){writeSettingsOnEachUpdateChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_CacheEnabled(createBooleanSettingsItem(sCacheEnabledKey,
        [this](const bool&, const bool& data){cacheEnabledChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_CacheMaxSizeMB(createRangedArithmeticSettingsItem<tCacheSizeMB>(sCacheMaxSizeMBKey,
        [this](const tCacheSizeMB&, const tCacheSizeMB& data){cacheMaxSizeMBChanged(data);},
        [this](){tryStoreSettingsConfig();},
        TRangedSettingItem<tCacheSizeMB>::tOptionalAllowedRange(TRangedSettingItem<tCacheSizeMB>::tAllowedRange(0, getRAMSizeUnchecked())),
        512)),
    mSetting_RDPMode(createBooleanSettingsItem(sRDPModeKey,
        [this](const bool&, const bool& data){RDPModeChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_RegexMonoHighlightingColor(createColorSettingsItem(sRegexMonoHighlightingColosKey,
        [this](const QColor&, const QColor& data){regexMonoHighlightingColorChanged(data);},
        [this](){tryStoreSettingsConfig();},
        QColor(150,0,0))),
    mSetting_HighlightActivePatterns(createBooleanSettingsItem(sHighlightActivePatternsKey,
        [this](const bool&, const bool& data){highlightActivePatternsChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_PatternsHighlightingColor(createColorSettingsItem(sPatternsHighlightingColorKey,
        [this](const QColor&, const QColor& data){patternsHighlightingColorChanged(data);},
        [this](){tryStoreSettingsConfig();},
        QColor(0,150,0))),
    mSetting_SearchResultMonoColorHighlighting(createBooleanSettingsItem(sSearchResultMonoColorHighlightingKey,
        [this](const bool&, const bool& data){searchResultMonoColorHighlightingChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSearchResultHighlightingGradientProtector(),
    mSetting_SearchResultHighlightingGradient(createHighlightingGradientSettingsItem(sSearchResultHighlightingGradientKey,
        [this](const tHighlightingGradient&,
               const tHighlightingGradient& data){searchResultHighlightingGradientChanged(data);},
        [this](){tryStoreSettingsConfig();},
        tHighlightingGradient(QColor(154,0,146), QColor(1,162,165), 3))),
    mSetting_SearchResultColumnsVisibilityMap(createSearchResultColumnsVisibilityMapSettingsItem(sSearchResultColumnsVisibilityMapKey,
        [this](const tSearchResultColumnsVisibilityMap&,
               const tSearchResultColumnsVisibilityMap& data){searchResultColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultSearchResultColumnsVisibilityMap)),
    mSetting_SearchResultColumnsCopyPasteMap(createSearchResultColumnsVisibilityMapSettingsItem(sSearchResultColumnsCopyPasteMapKey,
        [this](const tSearchResultColumnsVisibilityMap&,
               const tSearchResultColumnsVisibilityMap& data){searchResultColumnsCopyPasteMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultSearchResultColumnsCopyPasteMap)),
    mSetting_SearchResultColumnsSearchMap(createSearchResultColumnsVisibilityMapSettingsItem(sSearchResultColumnsSearchMapKey,
          [this](const tSearchResultColumnsVisibilityMap&,
                 const tSearchResultColumnsVisibilityMap& data){searchResultColumnsSearchMapChanged(data);},
          [this](){tryStoreSettingsConfig();},
          sDefaultSearchResultColumnsSearchMap)),
    mSetting_MarkTimeStampWithBold(createBooleanSettingsItem(sMarkTimeStampWithBold,
        [this](const bool&, const bool& data){markTimeStampWithBoldChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_PatternsColumnsVisibilityMap(createPatternsColumnsVisibilityMapSettingsItem(sPatternsColumnsVisibilityMapKey,
        [this](const tPatternsColumnsVisibilityMap&,
               const tPatternsColumnsVisibilityMap& data){patternsColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultPatternsColumnsVisibilityMap)),
    mSetting_PatternsColumnsCopyPasteMap(createPatternsColumnsVisibilityMapSettingsItem(sPatternsColumnsCopyPasteMapKey,
        [this](const tPatternsColumnsVisibilityMap&,
               const tPatternsColumnsVisibilityMap& data){patternsColumnsCopyPasteMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultPatternsColumnsCopyPasteMap)),
    mSetting_CaseSensitiveRegex(createBooleanSettingsItem(sCaseSensitiveRegex,
        [this](const bool&, const bool& data){caseSensitiveRegexChanged(data);},
        [this](){tryStoreSettingsConfig();},
        false)),
    mSetting_RegexFiltersColumnsVisibilityMap(createRegexFiltersColumnsVisibilityMapSettingsItem(sRegexFiltersColumnsVisibilityMapKey,
        [this](const tRegexFiltersColumnsVisibilityMap&,
               const tRegexFiltersColumnsVisibilityMap& data){regexFiltersColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultRegexFiltersColumnsVisibilityMap)),
    mSetting_FilterVariables(createBooleanSettingsItem(sFilterVariablesKey,
        [this](const bool&, const bool& data){filterVariablesChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_SelectedRegexFile(createStringSettingsItem(sSelectedRegexFile,
        [this](const QString& oldData, const QString& data)
        {
            QString regexSettingsFilePath = getRegexDirectory() + QDir::separator() + data;

            clearRegexConfig();
            loadRegexConfigCustomPath(regexSettingsFilePath);

            QString oldRegexUsageStatisticsFilePath = getRegexUsageStatisticsDirectory() + QDir::separator() + oldData;

            if(true == mbInitialised && false == oldData.isEmpty())
            {
                auto result = storeRegexUsageStatisticsDataCustomPath(oldRegexUsageStatisticsFilePath);

                if(false == result.bResult)
                {
                    SEND_ERR(QString("Was not able to store regex usage statistics due "
                                     "to the following error: %1").arg(result.err));
                }
            }

            clearRegexUsageStatisticsData();
            QString newRegexUsageStatisticsFilePath = getRegexUsageStatisticsDirectory() + QDir::separator() + data;
            loadRegexUsageStatisticsDataCustomPath(newRegexUsageStatisticsFilePath);

            selectedRegexFileChanged(data);
        },
        [this](){tryStoreSettingsConfig();},
        sDefaultRegexFileName)),
    mSetting_GroupedViewColumnsVisibilityMap(createGroupedViewColumnsVisibilityMapSettingsItem(sGroupedViewColumnsVisibilityMapKey,
        [this](const tGroupedViewColumnsVisibilityMap&,
               const tGroupedViewColumnsVisibilityMap& data){groupedViewColumnsVisibilityMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultGroupedViewColumnsVisibilityMap)),
    mSetting_GroupedViewColumnsCopyPasteMap(createGroupedViewColumnsVisibilityMapSettingsItem(sGroupedViewColumnsCopyPasteMapKey,
        [this](const tGroupedViewColumnsVisibilityMap&,
               const tGroupedViewColumnsVisibilityMap& data){groupedViewColumnsCopyPasteMapChanged(data);},
        [this](){tryStoreSettingsConfig();},
        sDefaultGroupedViewColumnsVisibilityMap)),
    mSetting_SubFilesHandlingStatus(createBooleanSettingsItem(sSubFilesHandlingStatusKey,
        [this](const bool&, const bool& data){subFilesHandlingStatusChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_Font_SearchView(createFontSettingsItem(sFont_SearchView,
        [this](const QFont&, const QFont& data){font_SearchViewChanged(data);},
        [this](){tryStoreSettingsConfig();},
        QFont("sans-serif", 9))),
    mUML_FeatureActiveProtector(),
    mSetting_UML_FeatureActive(createBooleanSettingsItem(sUML_FeatureActiveKey,
        [this](const bool&, const bool& data){UML_FeatureActiveChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_UML_MaxNumberOfRowsInDiagram(createArithmeticSettingsItem<int>(sUML_MaxNumberOfRowsInDiagramKey,
        [this](const int&, const int& data){UML_MaxNumberOfRowsInDiagramChanged(data);},
        [this](){tryStoreRootConfig();},
        1000)),
    mSetting_UML_ShowArguments(createBooleanSettingsItem(sUML_ShowArgumentsKey,
        [this](const bool&, const bool& data){UML_ShowArgumentsChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_UML_WrapOutput(createBooleanSettingsItem(sUML_WrapOutputKey,
        [this](const bool&, const bool& data){UML_WrapOutputChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_UML_Autonumber(createBooleanSettingsItem(sUML_AutonumberKey,
        [this](const bool&, const bool& data){UML_AutonumberChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mPlotViewFeatureActiveProtector(),
    mSetting_PlotViewFeatureActive(createBooleanSettingsItem(sPlotViewFeatureActiveKey,
        [this](const bool&, const bool& data){plotViewFeatureActiveChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_FiltersCompletion_CaseSensitive(createBooleanSettingsItem(sFiltersCompletion_CaseSensitiveKey,
       [this](const bool&, const bool& data){filtersCompletion_CaseSensitiveChanged(data);},
       [this](){tryStoreSettingsConfig();},
       false)),
    mSetting_FiltersCompletion_MaxNumberOfSuggestions(createRangedArithmeticSettingsItem<int>(sFiltersCompletion_MaxNumberOfSuggestionsKey,
       [this](const int&, const int& data){filtersCompletion_MaxNumberOfSuggestionsChanged(data);},
       [this](){tryStoreSettingsConfig();},
       TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(1, 1000)),
       200)),
    mSetting_FiltersCompletion_MaxCharactersInSuggestion(createRangedArithmeticSettingsItem<int>(sFiltersCompletion_MaxCharactersInSuggestionKey,
       [this](const int&, const int& data){filtersCompletion_MaxCharactersInSuggestionChanged(data);},
       [this](){tryStoreSettingsConfig();},
       TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(1, 1000)),
       200)),
    mSetting_FiltersCompletion_CompletionPopUpWidth(createRangedArithmeticSettingsItem<int>(sFiltersCompletion_CompletionPopUpWidthKey,
       [this](const int&, const int& data){filtersCompletion_CompletionPopUpWidthChanged(data);},
       [this](){tryStoreSettingsConfig();},
       TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(100, 1000)),
       400)),
    mSetting_FiltersCompletion_SearchPolicy(createBooleanSettingsItem(sFiltersCompletion_SearchPolicyKey,
       [this](const bool&, const bool& data){filtersCompletion_SearchPolicyChanged(data);},
       [this](){tryStoreSettingsConfig();},
       false)),
    mSetting_RegexCompletion_CaseSensitive(createBooleanSettingsItem(sRegexCompletion_CaseSensitiveKey,
       [this](const bool&, const bool& data){regexCompletion_CaseSensitiveChanged(data);},
       [this](){tryStoreSettingsConfig();},
       false)),
    mSetting_RegexCompletion_SearchPolicy(createBooleanSettingsItem(sRegexCompletion_SearchPolicyKey,
       [this](const bool&, const bool& data){regexCompletion_SearchPolicyChanged(data);},
       [this](){tryStoreSettingsConfig();},
       false)),
    mSetting_SearchViewLastColumnWidthStrategy(createRangedArithmeticSettingsItem<int>(sSearchViewLastColumnWidthStrategyKey,
        [this](const int&, const int& data){searchViewLastColumnWidthStrategyChanged(data);},
        [this](){tryStoreSettingsConfig();},
        TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(static_cast<int>(eSearchViewLastColumnWidthStrategy::eReset),
                                                                                              static_cast<int>(eSearchViewLastColumnWidthStrategy::eFitToContent))),
        static_cast<int>(eSearchViewLastColumnWidthStrategy::eFitToContent))),
    mSetting_PlantumlPathMode(createRangedArithmeticSettingsItem<int>(sPlantumlPathMode,
        [this](const int&, const int& data){plantumlPathModeChanged(data);},
        [this](){tryStoreSettingsConfig();},
        TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(static_cast<int>(ePathMode::eUseDefaultPath),
                                                                                              static_cast<int>(ePathMode::eLast) - 1)),
        static_cast<int>(ePathMode::eUseDefaultPath))),
    mSetting_PlantumlPathEnvVar(createStringSettingsItem(sPlantumlPathEnvVar,
        [this](const QString&, const QString& data){plantumlPathEnvVarChanged(data);},
        [this](){tryStoreSettingsConfig();},
        "")),
    mSetting_PlantumlCustomPath(createStringSettingsItem(sPlantumlCustomPath,
        [this](const QString&, const QString& data){plantumlCustomPathChanged(data);},
        [this](){tryStoreSettingsConfig();},
        "")),
    mSetting_JavaPathMode(createRangedArithmeticSettingsItem<int>(sJavaPathMode,
        [this](const int&, const int& data){javaPathModeChanged(data);},
        [this](){tryStoreSettingsConfig();},
        TRangedSettingItem<int>::tOptionalAllowedRange(TRangedSettingItem<int>::tAllowedRange(static_cast<int>(ePathMode::eUseDefaultPath),
                                                                                              static_cast<int>(ePathMode::eLast) - 1)),
        static_cast<int>(ePathMode::eUseDefaultPath))),
    mSetting_JavaPathEnvVar(createStringSettingsItem(sJavaPathEnvVar,
        [this](const QString&, const QString& data){javaPathEnvVarChanged(data);},
        [this](){tryStoreSettingsConfig();},
        "")),
    mSetting_JavaCustomPath(createStringSettingsItem(sJavaCustomPath,
        [this](const QString&, const QString& data){javaCustomPathChanged(data);},
        [this](){tryStoreSettingsConfig();},
        "")),
    mSetting_GroupedViewFeatureActive(createBooleanSettingsItem(sGroupedViewFeatureActive,
        [this](const bool&, const bool& data){groupedViewFeatureActiveChanged(data);},
        [this](){tryStoreSettingsConfig();},
        true)),
    mSetting_RegexUsageStatistics(createRegexUsageStatisticsItemMapSettingsItem(sRegexUsageStatisticsKey,
        [this](const tRegexUsageStatisticsItemMap&,
               const tRegexUsageStatisticsItemMap& data){ regexUsageStatisticsChanged(data); },
        []()
        {
            // this data is not critical and is stored to file ONLY during exit.
        },
        tRegexUsageStatisticsItemMap())),
    mSetting_Username(createStringSettingsItem(sUsernameKey,
        [this](const QString&,
               const QString& data){ usernameChanged(data); },
        []()
        {
            // this data is not critical and is stored to file ONLY during exit.
        },
        getCurrentUserName())),
    mSetting_RegexInputFieldHeight(createArithmeticSettingsItem<int>(sRegexInputFieldHeight,
        [this](const int&,
               const int& data){ regexInputFieldHeightChanged(data); },
        [this](){tryStoreSettingsConfig();},
        4)),
    mRootSettingItemPtrVec(),
    mUserSettingItemPtrVec(),
    mPatternsSettingItemPtrVec(),
    mRegexUsageStatisticsDataItemPtrVec(),
    mbInitialised(false)
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
    mUserSettingItemPtrVec.push_back(&mSetting_SearchResultColumnsSearchMap);
    mUserSettingItemPtrVec.push_back(&mSetting_MarkTimeStampWithBold);
    mUserSettingItemPtrVec.push_back(&mSetting_PatternsColumnsVisibilityMap);
    mUserSettingItemPtrVec.push_back(&mSetting_PatternsColumnsCopyPasteMap);
    mUserSettingItemPtrVec.push_back(&mSetting_CaseSensitiveRegex);
    mUserSettingItemPtrVec.push_back(&mSetting_RegexFiltersColumnsVisibilityMap);
    mUserSettingItemPtrVec.push_back(&mSetting_FilterVariables);
    mUserSettingItemPtrVec.push_back(&mSetting_SelectedRegexFile);
    mUserSettingItemPtrVec.push_back(&mSetting_GroupedViewColumnsVisibilityMap);
    mUserSettingItemPtrVec.push_back(&mSetting_GroupedViewColumnsCopyPasteMap);
    mUserSettingItemPtrVec.push_back(&mSetting_SubFilesHandlingStatus);
    mUserSettingItemPtrVec.push_back(&mSetting_Font_SearchView);
    mUserSettingItemPtrVec.push_back(&mSetting_UML_FeatureActive);
    mUserSettingItemPtrVec.push_back(&mSetting_UML_MaxNumberOfRowsInDiagram);
    mUserSettingItemPtrVec.push_back(&mSetting_UML_ShowArguments);
    mUserSettingItemPtrVec.push_back(&mSetting_UML_WrapOutput);
    mUserSettingItemPtrVec.push_back(&mSetting_UML_Autonumber);
    mUserSettingItemPtrVec.push_back(&mSetting_PlotViewFeatureActive);
    mUserSettingItemPtrVec.push_back(&mSetting_FiltersCompletion_CaseSensitive);
    mUserSettingItemPtrVec.push_back(&mSetting_FiltersCompletion_MaxNumberOfSuggestions);
    mUserSettingItemPtrVec.push_back(&mSetting_FiltersCompletion_MaxCharactersInSuggestion);
    mUserSettingItemPtrVec.push_back(&mSetting_FiltersCompletion_CompletionPopUpWidth);
    mUserSettingItemPtrVec.push_back(&mSetting_FiltersCompletion_SearchPolicy);
    mUserSettingItemPtrVec.push_back(&mSetting_RegexCompletion_CaseSensitive);
    mUserSettingItemPtrVec.push_back(&mSetting_RegexCompletion_SearchPolicy);
    mUserSettingItemPtrVec.push_back(&mSetting_SearchViewLastColumnWidthStrategy);
    mUserSettingItemPtrVec.push_back(&mSetting_PlantumlPathMode);
    mUserSettingItemPtrVec.push_back(&mSetting_PlantumlPathEnvVar);
    mUserSettingItemPtrVec.push_back(&mSetting_PlantumlCustomPath);
    mUserSettingItemPtrVec.push_back(&mSetting_JavaPathMode);
    mUserSettingItemPtrVec.push_back(&mSetting_JavaPathEnvVar);
    mUserSettingItemPtrVec.push_back(&mSetting_JavaCustomPath);
    mUserSettingItemPtrVec.push_back(&mSetting_GroupedViewFeatureActive);
    mUserSettingItemPtrVec.push_back(&mSetting_Username);
    mUserSettingItemPtrVec.push_back(&mSetting_RegexInputFieldHeight);

    /////////////// PATTERNS SETTINGS ///////////////
    mPatternsSettingItemPtrVec.push_back(&mSetting_Aliases);

    /////////////// REGEX USAGE STATISTICS //////////
    mRegexUsageStatisticsDataItemPtrVec.push_back(&mSetting_RegexUsageStatistics);
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
    SEND_MSG(QString("[CSettingsManager] Performing setting manager update."));

    auto result = backwardCompatibility_V0_V1();

    if(true == result.bResult)
    {
        result = loadRootConfig(); // load root config. It should be available starting version 0

        auto cachedSettingsMangerVersion = getSettingsManagerVersion();

        SEND_MSG(QString("[CSettingsManager] Persisted settings manager verison - %1.").arg(getSettingsManagerVersion()));
        SEND_MSG(QString("[CSettingsManager] Target settings manager verison - %1.").arg(sCurrentSettingsManagerVersion));

        if(sCurrentSettingsManagerVersion != cachedSettingsMangerVersion)
        {
            if(cachedSettingsMangerVersion == 1u && sCurrentSettingsManagerVersion == 2)
            {
                result = backwardCompatibility_V1_V2();
            }
        }

        if(true == result.bResult)
        {
            setSettingsManagerVersion( sCurrentSettingsManagerVersion ); // if backward compatibility was successful, we need to update the settings manager version
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

TSettingItem<CSettingsManager::tAliasItemMap> CSettingsManager::createAliasItemMapSettingsItem(const QString& key,
                                             const TSettingItem<tAliasItemMap>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tAliasItemMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tAliasItemMap& defaultValue) const
{
    auto writeFunc = [&key](const tAliasItemMap& value)->QJsonObject
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
                       tAliasItemMap& data,
                       const tAliasItemMap&)->bool
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

                            QString aliasStr = alias->toString();
                            data.insert(aliasStr, tAliasItem(bIsDefault, aliasStr, regex->toString()));
                        }
                    }
                }

                bResult = true;
            }
        }

        return bResult;
    };

    return TSettingItem<tAliasItemMap>(key,
                             defaultValue,
                             writeFunc,
                             readFunc,
                             updateDataFunc,
                             updateFileFunc);
}

TSettingItem<CSettingsManager::tRegexUsageStatisticsItemMap> CSettingsManager::createRegexUsageStatisticsItemMapSettingsItem(const QString& key,
                                             const TSettingItem<tRegexUsageStatisticsItemMap>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<tRegexUsageStatisticsItemMap>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const tRegexUsageStatisticsItemMap& defaultValue) const
{
    auto writeFunc = [&key](const tRegexUsageStatisticsItemMap& value)->QJsonObject
    {
        QJsonObject aliases;
        QJsonArray arrayRegexUsageSttistics;

        for (auto it = value.keyValueBegin(); it != value.keyValueEnd(); ++it)
        {
            for(auto jt = it->second.keyValueBegin(); jt != it->second.keyValueEnd(); ++jt)
            {
                QJsonObject obj;
                obj.insert( sRegexUsageStatisticsItemTypeKey, static_cast<int>(it->first) );
                obj.insert( sRegexKey, QJsonValue( jt->first ) );
                obj.insert( sUsageCounterKey, jt->second.usageCounter );
                obj.insert( sUpdateDateTimeKey, jt->second.updateDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz") );
                arrayRegexUsageSttistics.append( obj );
            }
        }

        QJsonObject result;
        result.insert( key, QJsonValue( arrayRegexUsageSttistics ) );

        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       tRegexUsageStatisticsItemMap& data,
                       const tRegexUsageStatisticsItemMap&)->bool
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

                    auto regexObj = obj.find(sRegexKey);

                    if(regexObj != obj.end() && regexObj->isString())
                    {
                        auto usageCounterObj = obj.find(sUsageCounterKey);

                        if(usageCounterObj != obj.end() && usageCounterObj->isDouble())
                        {
                            int usageCounter = static_cast<int>(usageCounterObj->toDouble());

                            auto itemTypeObj = obj.find(sRegexUsageStatisticsItemTypeKey);
                            if(itemTypeObj != obj.end() && itemTypeObj->isDouble())
                            {
                                eRegexUsageStatisticsItemType itemType =
                                        static_cast<eRegexUsageStatisticsItemType>(itemTypeObj->toDouble());

                                auto updateDateTimeObj = obj.find(sUpdateDateTimeKey);

                                if(updateDateTimeObj != obj.end() && updateDateTimeObj->isString())
                                {
                                    auto updateDateTime = QDateTime::fromString(updateDateTimeObj->toString(), "yyyy-MM-dd HH:mm:ss.zzz");

                                    if(true == updateDateTime.isValid())
                                    {
                                        auto& updteItem = data[itemType][regexObj->toString()];
                                        updteItem.usageCounter = usageCounter;
                                        updteItem.updateDateTime = updateDateTime;
                                    }
                                }
                            }
                        }
                    }
                }

                bResult = true;
            }
        }

        return bResult;
    };

    return TSettingItem<tRegexUsageStatisticsItemMap>(key,
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
                       const tSearchResultColumnsVisibilityMap& defaultValues)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isArray())
        {
            data.clear();

            auto defaultValuesCopy = defaultValues;

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

                                    auto foundDefaultValue = defaultValuesCopy.find(static_cast<eSearchResultColumn>(columnIdx));

                                    if(foundDefaultValue != defaultValuesCopy.end())
                                    {
                                        defaultValuesCopy.erase(foundDefaultValue);
                                    }
                                }
                            }
                        }

                        if(false == defaultValuesCopy.empty()) // if there are some non-existing elements in the file
                        {
                            for(auto it = defaultValuesCopy.begin(); it != defaultValuesCopy.end(); ++it) // let's fill them in with default values
                            {
                                data.insert(static_cast<eSearchResultColumn>(it.key()), it.value());
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

TSettingItem<QFont> CSettingsManager::createFontSettingsItem(const QString& key,
                                             const TSettingItem<QFont>::tUpdateDataFunc& updateDataFunc,
                                             const TSettingItem<QFont>::tUpdateSettingsFileFunc& updateFileFunc,
                                             const QFont& defaultValue) const
{
    auto writeFunc = [&key](const QFont& value)->QJsonObject
    {
        QJsonObject result;
        result.insert( key, QJsonValue( value.toString() ) );
        return result;
    };

    auto readFunc = [](const QJsonValueRef& JSONItem,
                       QFont& data,
                       const QFont& defaultValue_)->bool
    {
        bool bResult = false;

        if(true == JSONItem.isString())
        {
            bool bReadResult = data.fromString( JSONItem.toString() );

            if(false == bReadResult) // if parsing of the font data has failed
            {
                data = defaultValue_; // let's do a fallback to the default value
            }

            bResult = true;
        }

        return bResult;
    };

    return TSettingItem<QFont>(key,
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
    if(jsonFile.open(QFile::ReadWrite))
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

            if(true == result.bResult)
            {
                QString regexUsageStatisticsFilePath = getRegexUsageStatisticsDirectory() + QDir::separator() + mSetting_SelectedRegexFile.getData();
                result = storeRegexUsageStatisticsDataCustomPath(regexUsageStatisticsFilePath);
            }
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

                if( true == result.bResult )
                {
                    QString regexUsageStatisticsFilePath = getRegexUsageStatisticsDirectory() + QDir::separator() + mSetting_SelectedRegexFile.getData();
                    result = loadRegexUsageStatisticsDataCustomPath(regexUsageStatisticsFilePath); // load regex usage statistics
                }
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
        mbInitialised = true;
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::backwardCompatibility_V0_V1()
{
    CSettingsManager::tOperationResult result;
    result.bResult = true;

    const char* sFileName = "DLTMessageAnalyzer_plugin_config.json";

    QString V0_SettingsFilePath(QCoreApplication::applicationDirPath() + QDir::separator() + sFileName);
    QFile V0_SettingsFile( V0_SettingsFilePath );

    QString regexDirPath = getRegexDirectory();
    QDir regexDir( regexDirPath );

    // Let's check, whether we have V0.
    // That can be checked only indirectly, based on file-system-based conclusions
    if(false == regexDir.exists()) // regex dir does not exist. Most probably we've faced old system
    {
        SEND_MSG(QString("[CSettingsManager] Performing setting manager update from V0 to V1"));

        QDir dir;
        QString configDirPath = sSettingsManager_Directory;

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

        SEND_MSG(QString("[CSettingsManager] Setting manager update from V0 to V1 finished with result - %1").arg(true == result.bResult ? "SUCCESSFUL" : "FAILED"));
    }
    else
    {
        SEND_MSG(QString("[CSettingsManager] Setting manager update from V0 to V1 is not needed"));
    }

    return result;
}

CSettingsManager::tOperationResult CSettingsManager::backwardCompatibility_V1_V2()
{
    CSettingsManager::tOperationResult result;
    result.bResult = true;

    QString regexUsageStatisticsDirPath = getRegexUsageStatisticsDirectory();
    QDir dir;

    SEND_MSG(QString("[CSettingsManager] Performing setting manager update from V1 to V2"));

    // let's create regex usage statistics directory
    if(false == dir.mkpath(regexUsageStatisticsDirPath))
    {
        result.bResult = false;
        result.err = QString( "[%1] Was not able to create directory \"%2\"" ).arg(__FUNCTION__).arg(regexUsageStatisticsDirPath);
    }

    SEND_MSG(QString("[CSettingsManager] Setting manager update from V1 to V2 finished with result - %1").arg(true == result.bResult ? "SUCCESSFUL" : "FAILED"));

    return result;
}

void CSettingsManager::setSettingsManagerVersion(const tSettingsManagerVersion& val)
{
    mSetting_SettingsManagerVersion.setData(val);
}

void CSettingsManager::setAliases(const tAliasItemMap& val)
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

void CSettingsManager::setRegexUsageStatistics(const tRegexUsageStatisticsItemMap& val)
{
    mSetting_RegexUsageStatistics.setData(val);
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

void CSettingsManager::setSearchResultColumnsSearchMap(const tSearchResultColumnsVisibilityMap& val)
{
    mSetting_SearchResultColumnsSearchMap.setData(val);
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

void CSettingsManager::setGroupedViewColumnsCopyPasteMap(const tGroupedViewColumnsVisibilityMap& val)
{
    mSetting_GroupedViewColumnsCopyPasteMap.setData(val);
}

void CSettingsManager::setSubFilesHandlingStatus( const bool& val )
{
    mSetting_SubFilesHandlingStatus.setData(val);
}

void CSettingsManager::setFont_SearchView( const QFont& val )
{
    mSetting_Font_SearchView.setData(val);
}

void CSettingsManager::setUML_FeatureActive(const bool& val)
{
    std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mUML_FeatureActiveProtector));
    mSetting_UML_FeatureActive.setData(val);
}

void CSettingsManager::setUML_MaxNumberOfRowsInDiagram(const int& val)
{
    mSetting_UML_MaxNumberOfRowsInDiagram.setData(val);
}

void CSettingsManager::setUML_ShowArguments(const bool& val)
{
    mSetting_UML_ShowArguments.setData(val);
}

void CSettingsManager::setUML_WrapOutput(const bool& val)
{
    mSetting_UML_WrapOutput.setData(val);
}

void CSettingsManager::setUML_Autonumber(const bool& val)
{
    mSetting_UML_Autonumber.setData(val);
}

void CSettingsManager::setPlotViewFeatureActive(const bool& val)
{
    std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mPlotViewFeatureActiveProtector));
    mSetting_PlotViewFeatureActive.setData(val);
}

void CSettingsManager::setFiltersCompletion_CaseSensitive(const bool& val)
{
    mSetting_FiltersCompletion_CaseSensitive.setData(val);
}

void CSettingsManager::setFiltersCompletion_MaxNumberOfSuggestions(const int& val)
{
    mSetting_FiltersCompletion_MaxNumberOfSuggestions.setData(val);
}

void CSettingsManager::setFiltersCompletion_MaxCharactersInSuggestion(const int& val)
{
    mSetting_FiltersCompletion_MaxCharactersInSuggestion.setData(val);
}

void CSettingsManager::setFiltersCompletion_CompletionPopUpWidth(const int& val)
{
    mSetting_FiltersCompletion_CompletionPopUpWidth.setData(val);
}

void CSettingsManager::setFiltersCompletion_SearchPolicy(const bool& val)
{
    mSetting_FiltersCompletion_SearchPolicy.setData(val);
}

void CSettingsManager::setRegexCompletion_CaseSensitive(const bool& val)
{
    mSetting_RegexCompletion_CaseSensitive.setData(val);
}

void CSettingsManager::setRegexCompletion_SearchPolicy(const bool& val)
{
    mSetting_RegexCompletion_SearchPolicy.setData(val);
}

void CSettingsManager::setUserName(const QString& val)
{
    mSetting_Username.setData(val);
}

void CSettingsManager::setRegexInputFieldHeight(const int& linesNumber)
{
    mSetting_RegexInputFieldHeight.setData(linesNumber);
}

void CSettingsManager::setSearchViewLastColumnWidthStrategy(const int& val)
{
    mSetting_SearchViewLastColumnWidthStrategy.setData(val);
}

void CSettingsManager::setPlantumlPathMode(const int& val)
{
    mSetting_PlantumlPathMode.setData(val);
}

void CSettingsManager::setPlantumlPathEnvVar(const QString& val)
{
    mSetting_PlantumlPathEnvVar.setData(val);
}

void CSettingsManager::setPlantumlCustomPath(const QString& val)
{
    mSetting_PlantumlCustomPath.setData(val);
}

void CSettingsManager::setJavaPathMode(const int& val)
{
    mSetting_JavaPathMode.setData(val);
}

void CSettingsManager::setJavaPathEnvVar(const QString& val)
{
    mSetting_JavaPathEnvVar.setData(val);
}

void CSettingsManager::setJavaCustomPath(const QString& val)
{
    mSetting_JavaCustomPath.setData(val);
}

void CSettingsManager::setGroupedViewFeatureActive(bool val)
{
    mSetting_GroupedViewFeatureActive.setData(val);
}

void CSettingsManager::setSelectedRegexFile(const QString& val)
{
    mSetting_SelectedRegexFile.setData(val);
}

const tSettingsManagerVersion& CSettingsManager::getSettingsManagerVersion() const
{
    return mSetting_SettingsManagerVersion.getData();
}

const CSettingsManager::tAliasItemMap& CSettingsManager::getAliases() const
{
    return mSetting_Aliases.getData();
}

const CSettingsManager::tRegexUsageStatisticsItemMap& CSettingsManager::getRegexUsageStatistics() const
{
    return mSetting_RegexUsageStatistics.getData();
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

const tSearchResultColumnsVisibilityMap& CSettingsManager::getSearchResultColumnsSearchMap() const
{
    return mSetting_SearchResultColumnsSearchMap.getData();
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

const tGroupedViewColumnsVisibilityMap& CSettingsManager::getGroupedViewColumnsCopyPasteMap() const
{
    return mSetting_GroupedViewColumnsCopyPasteMap.getData();
}

bool CSettingsManager::getSubFilesHandlingStatus() const
{
    return mSetting_SubFilesHandlingStatus.getData();
}

const QFont& CSettingsManager::getFont_SearchView() const
{
    return mSetting_Font_SearchView.getData();
}

const bool& CSettingsManager::getUML_FeatureActive() const
{
    std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mUML_FeatureActiveProtector));
    return mSetting_UML_FeatureActive.getData();
}

const int& CSettingsManager::getUML_MaxNumberOfRowsInDiagram() const
{
    return mSetting_UML_MaxNumberOfRowsInDiagram.getData();
}

const bool& CSettingsManager::getUML_ShowArguments() const
{
    return mSetting_UML_ShowArguments.getData();
}

const bool& CSettingsManager::getUML_WrapOutput() const
{
    return mSetting_UML_WrapOutput.getData();
}

const bool& CSettingsManager::getUML_Autonumber() const
{
    return mSetting_UML_Autonumber.getData();
}

const bool& CSettingsManager::getPlotViewFeatureActive() const
{
    std::lock_guard<std::recursive_mutex> lock(*const_cast<std::recursive_mutex*>(&mPlotViewFeatureActiveProtector));
    return mSetting_PlotViewFeatureActive.getData();
}

const bool& CSettingsManager::getFiltersCompletion_CaseSensitive() const
{
    return mSetting_FiltersCompletion_CaseSensitive.getData();
}

const int& CSettingsManager::getFiltersCompletion_MaxNumberOfSuggestions() const
{
    return mSetting_FiltersCompletion_MaxNumberOfSuggestions.getData();
}

const int& CSettingsManager::getFiltersCompletion_MaxCharactersInSuggestion() const
{
    return mSetting_FiltersCompletion_MaxCharactersInSuggestion.getData();
}

const int& CSettingsManager::getFiltersCompletion_CompletionPopUpWidth() const
{
    return mSetting_FiltersCompletion_CompletionPopUpWidth.getData();
}

const bool& CSettingsManager::getFiltersCompletion_SearchPolicy() const
{
    return mSetting_FiltersCompletion_SearchPolicy.getData();
}

const bool& CSettingsManager::getRegexCompletion_CaseSensitive() const
{
    return mSetting_RegexCompletion_CaseSensitive.getData();
}

const bool& CSettingsManager::getRegexCompletion_SearchPolicy() const
{
    return mSetting_RegexCompletion_SearchPolicy.getData();
}

const QString& CSettingsManager::getUsername() const
{
    return mSetting_Username.getData();
}

const int& CSettingsManager::getRegexInputFieldHeight() const
{
    return mSetting_RegexInputFieldHeight.getData();
}

const int& CSettingsManager::getSearchViewLastColumnWidthStrategy() const
{
    return mSetting_SearchViewLastColumnWidthStrategy.getData();
}

const int& CSettingsManager::getPlantumlPathMode() const
{
    return mSetting_PlantumlPathMode.getData();
}

const QString& CSettingsManager::getPlantumlPathEnvVar() const
{
    return mSetting_PlantumlPathEnvVar.getData();
}

const QString& CSettingsManager::getPlantumlCustomPath() const
{
    return mSetting_PlantumlCustomPath.getData();
}

const int& CSettingsManager::getJavaPathMode() const
{
    return mSetting_JavaPathMode.getData();
}

const QString& CSettingsManager::getJavaPathEnvVar() const
{
    return mSetting_JavaPathEnvVar.getData();
}

const QString& CSettingsManager::getJavaCustomPath() const
{
    return mSetting_JavaCustomPath.getData();
}

const bool& CSettingsManager::getGroupedViewFeatureActive() const
{
    return mSetting_GroupedViewFeatureActive.getData();
}

QString CSettingsManager::getRegexDirectory() const
{
    return sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_Regex_SubDirectory;
}

QString CSettingsManager::getRegexUsageStatisticsDirectory() const
{
    return sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_RegexUsageStatistics_SubDirectory;
}

QString CSettingsManager::getSettingsFilepath() const
{
    return sSettingsManager_Directory;
}

QString CSettingsManager::getUserSettingsFilepath() const
{
    return sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_User_SettingsFile;
}

QString CSettingsManager::getRootSettingsFilepath() const
{
    return sSettingsManager_Directory + QDir::separator() +
           sSettingsManager_Root_SettingsFile;
}

QString CSettingsManager::getDefaultPlantumlPath() const
{
    return QCoreApplication::applicationDirPath() + QDir::separator() + "plugins/plantuml.jar";
}

QString CSettingsManager::getDefaultJavaPath() const
{
    return "java"; // we expect that it should be inside the path
}

void CSettingsManager::refreshRegexConfiguration()
{
    mSetting_SelectedRegexFile.setData(mSetting_SelectedRegexFile.getData(), true);
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

CSettingsManager::tOperationResult CSettingsManager::loadRegexConfigCustomPath(const QString &filePath)
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::ReadWrite))
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

void CSettingsManager::clearRegexConfig()
{
    mSetting_Aliases.setDataSilent(tAliasItemMap());
    aliasesChanged(mSetting_Aliases.getData());
}

CSettingsManager::tOperationResult
CSettingsManager::storeRegexUsageStatisticsDataCustomPath( const QString& filePath ) const
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filePath);

    if(jsonFile.open(QFile::WriteOnly))
    {
        QJsonArray settingsArray;

        for(auto* pSettingItem : mRegexUsageStatisticsDataItemPtrVec)
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


CSettingsManager::tOperationResult
CSettingsManager::loadRegexUsageStatisticsDataCustomPath( const QString& filePath )
{
    CSettingsManager::tOperationResult result;
    result.bResult = false;

    QFile jsonFile(filePath);

    if(jsonFile.open(QFile::ReadWrite))
    {
        auto jsonDoc =  QJsonDocument().fromJson(jsonFile.readAll());

        if( true == jsonDoc.isArray() )
        {
            QJsonArray arrayRows = jsonDoc.array();

            for(auto* pSettingItem : mRegexUsageStatisticsDataItemPtrVec)
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

void CSettingsManager::clearRegexUsageStatisticsData()
{
    mSetting_RegexUsageStatistics.setDataSilent(tRegexUsageStatisticsItemMap());
    regexUsageStatisticsChanged(mSetting_RegexUsageStatistics.getData());
}

CSettingsManager::tOperationResult CSettingsManager::loadSettingsConfig()
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
        // Uncomment this to see the content of the generated file
        //qDebug() << "doc - " << jsonDoc.toJson();
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
    if(jsonFile.open(QFile::ReadWrite))
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
    setSearchResultColumnsCopyPasteMap(sDefaultSearchResultColumnsCopyPasteMap);
}

void CSettingsManager::resetSearchResultColumnsSearchMap()
{
    setSearchResultColumnsSearchMap(sDefaultSearchResultColumnsSearchMap);
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

void CSettingsManager::resetGroupedViewColumnsCopyPasteMap()
{
    setGroupedViewColumnsCopyPasteMap(sDefaultGroupedViewColumnsVisibilityMap);
}

const TRangedSettingItem<int>::tOptionalAllowedRange& CSettingsManager::getSetting_NumberOfThreads_AllowedRange() const
{
    return mSetting_NumberOfThreads.getAllowedTange();
}

const TRangedSettingItem<tCacheSizeMB>::tOptionalAllowedRange& CSettingsManager::getSetting_CacheMaxSizeMB_AllowedRange() const
{
    return mSetting_CacheMaxSizeMB.getAllowedTange();
}

const TRangedSettingItem<int>::tOptionalAllowedRange& CSettingsManager::getFiltersCompletion_MaxNumberOfSuggestions_AllowedRange() const
{
    return mSetting_FiltersCompletion_MaxNumberOfSuggestions.getAllowedTange();
}

const TRangedSettingItem<int>::tOptionalAllowedRange& CSettingsManager::getFiltersCompletion_MaxCharactersInSuggestion_AllowedRange() const
{
    return mSetting_FiltersCompletion_MaxCharactersInSuggestion.getAllowedTange();
}

const TRangedSettingItem<int>::tOptionalAllowedRange& CSettingsManager::getFiltersCompletion_CompletionPopUpWidth_AllowedRange() const
{
    return mSetting_FiltersCompletion_CompletionPopUpWidth.getAllowedTange();
}

PUML_PACKAGE_BEGIN(DMA_Settings)
    PUML_CLASS_BEGIN_CHECKED(CSettingsManager)
        PUML_INHERITANCE_CHECKED(ISettingsManager, implements)
        PUML_COMPOSITION_DEPENDENCY(TSettingItem<T>, 1, *, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
