#include <map>
#include <set>

#include "QCompleter"
#include "QStandardItemModel"
#include "QStandardItem"
#include "QIcon"
#include "QVariant"
#include "QEvent"
#include "QMouseEvent"

#include "CRegexHistoryProvider.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/regexHistory/api/CRegexHistoryTextEdit.hpp"

#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

const static QRegularExpression sFindLastPipeRegex("(.*)(?<!\\\\)(\\|)(?!.*(?<!\\\\)\\1)(.*)");
const static int sMaximumRegexHistorySize = 100;
const static int sSuggestionTypeRole = 123;

// CExtendedCompleter implementation

CExtendedCompleter::CExtendedCompleter(QObject* parent,
                                     CRegexHistoryTextEdit* pRegexTextEdit):
QCompleter(parent),
mbStartProcessEventFilter(false),
mpRegexTextEdit(pRegexTextEdit)
{}

void CExtendedCompleter::initFinished()
{
    mbStartProcessEventFilter = true;
}

bool CExtendedCompleter::eventFilter(QObject *o, QEvent *e)
{
    // Handle mouse button press events (clicks)
    if (e->type() == QEvent::MouseButtonPress)
    {
        // Get the popup and check if the mouse click occurred outside it
        if (popup() && popup()->isVisible() && !popup()->underMouse()) {
            // If clicked outside the popup, hide it and emit the loseFocus signal
            emit loseFocus();
        }
    }
    else if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
        {
            // Handle the key press, prevent further propagation
            if (popup()->isVisible())
            {
                mpRegexTextEdit->setIgnoreReturnKeyEvent(true);
                bool bResult = QCompleter::eventFilter(o, e);
                mpRegexTextEdit->setIgnoreReturnKeyEvent(false);
                return bResult;
            }
        }
    }

    // Default behavior for other events
    return QCompleter::eventFilter(o, e);
}

// CExtendedCompleter implementation end

static bool reachedTheSizeLimit(ISettingsManager::tRegexUsageStatisticsItemData& map)
{
    return map.size() >= sMaximumRegexHistorySize;
}

typedef int64_t tSuggestionRanking;

struct tSuggestionRankingValue
{
    tSuggestionRanking usageCounterRanking;
    tSuggestionRanking updateTimeRanking;
    ISettingsManager::tRegexUsageStatisticsItemData::iterator it;
    bool operator< (const tSuggestionRankingValue& val)
    {
        return (usageCounterRanking + updateTimeRanking) <
               (val.usageCounterRanking + val.updateTimeRanking);
    }
};

typedef std::shared_ptr<tSuggestionRankingValue> tSuggestionRankingValuePtr;

struct tSuggestionRankingValuePtrWrapper
{
public:
    tSuggestionRankingValuePtr pSuggestionRankingValue;

    bool operator< (const tSuggestionRankingValuePtrWrapper& val) const
    {
        bool bResult = false;

        if(pSuggestionRankingValue && val.pSuggestionRankingValue)
        {
            return *pSuggestionRankingValue < *val.pSuggestionRankingValue;
        }
        else if(!pSuggestionRankingValue && val.pSuggestionRankingValue)
        {
            bResult = true;
        }
        else if(pSuggestionRankingValue && !val.pSuggestionRankingValue)
        {
            bResult = true;
        }
        else if(!pSuggestionRankingValue && !val.pSuggestionRankingValue)
        {
            bResult = false;
        }

        return bResult;
    }
};

typedef std::multimap<tSuggestionRanking, tSuggestionRankingValuePtr> tSuggestionPreparationRankingMap;
typedef std::multiset<tSuggestionRankingValuePtrWrapper> tSuggestionRankingSet;

static void deleteNonRelevantElements(ISettingsManager::tRegexUsageStatisticsItemData& map)
{
    // Find the element with the oldest updateDateTime
    QDateTime oldestDateTime = QDateTime::currentDateTime(); // Initialize to current time for comparison

    tSuggestionPreparationRankingMap usageCounterMap;
    tSuggestionPreparationRankingMap updateTimeMap;

    for (auto it = map.begin(); it != map.end(); ++it)
    {
        tSuggestionRankingValuePtr pSuggestionRankValue = std::make_shared<tSuggestionRankingValue>();
        pSuggestionRankValue->it = it;
        usageCounterMap.insert(std::make_pair(it.value().usageCounter, pSuggestionRankValue));
        updateTimeMap.insert(std::make_pair(it.value().updateDateTime.toMSecsSinceEpoch(), pSuggestionRankValue));
    }

    int rankingCounter = 0;
    for(auto& item : usageCounterMap)
    {
        item.second->usageCounterRanking = rankingCounter;
        ++rankingCounter;
    }

    rankingCounter = 0;
    for(auto& item : updateTimeMap)
    {
        item.second->updateTimeRanking = rankingCounter;
        ++rankingCounter;
    }

    tSuggestionRankingSet suggestionRankingSet;

    for(const auto& item : updateTimeMap)
    {
        tSuggestionRankingValuePtrWrapper suggestionRankingValuePtrWrapper;
        suggestionRankingValuePtrWrapper.pSuggestionRankingValue = item.second;
        suggestionRankingSet.insert(suggestionRankingValuePtrWrapper);
    }

    for(const auto& item : suggestionRankingSet)
    {
        if(map.size() > sMaximumRegexHistorySize)
        {
            map.erase(item.pSuggestionRankingValue->it);
        }
        else
        {
            break;
        }
    }
}

CRegexHistoryProvider::CRegexHistoryProvider(const tSettingsManagerPtr& pSettingsManager,
                      CRegexHistoryTextEdit* pRegexTextEdit, CPatternsView* pPatternsView,
                      const tDLTMessageAnalyzerControllerPtr& pDLTMessageAnalyzerController):
CSettingsManagerClient(pSettingsManager),
mpRegexTextEdit(pRegexTextEdit),
mpPatternsView(pPatternsView),
mpDLTMessageAnalyzerController(pDLTMessageAnalyzerController),
mbSuggestionActive(false)
{
    if(nullptr != mpRegexTextEdit)
    {
        CExtendedCompleter* pCompleter = new CExtendedCompleter(this, mpRegexTextEdit);

        QStandardItemModel* pModel = new QStandardItemModel(pCompleter);
        pCompleter->setModel(pModel);
        pCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        mCompletionData.pCompleter = pCompleter;
        auto* pExtendedListView = new QListView(mpRegexTextEdit);

        mCompletionData.pPopUp = pExtendedListView;
        mCompletionData.pPopUp->setIconSize(QSize(50, 50));
        mCompletionData.pPopUp->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mCompletionData.pPopUp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mCompletionData.pPopUp->setSelectionBehavior(QAbstractItemView::SelectRows);
        mCompletionData.pPopUp->setSelectionMode(QAbstractItemView::SingleSelection);
        mCompletionData.pCompleter->setPopup(mCompletionData.pPopUp);

        mpRegexTextEdit->setCompleter(mCompletionData.pCompleter);

        connect(pCompleter, &CExtendedCompleter::loseFocus,
                this, [this]()
        {
            setSuggestionActive(false);
        });

        connect(mCompletionData.pCompleter, QOverload<const QModelIndex&>::of(&QCompleter::activated),
                this, [this](const QModelIndex& index)
        {
            auto resolveText = [this, &index]()->QString
            {
                QString result;

                if(nullptr != mCompletionData.pPopUp)
                {
                    if(index.isValid())
                    {
                        // Retrieve the custom data using the QModelIndex and the TypeRole
                        QVariant typeData = index.data(sSuggestionTypeRole);

                        switch(static_cast<ISettingsManager::eRegexUsageStatisticsItemType>(typeData.toInt()))
                        {
                            case ISettingsManager::eRegexUsageStatisticsItemType::TEXT:
                            {
                                result = index.data(Qt::DisplayRole).toString();
                                incrementRegexTextCounter(result);
                            }
                            break;
                            case ISettingsManager::eRegexUsageStatisticsItemType::STORED_REGEX_PATTERN:
                            {
                                QString aliasName = index.data(Qt::DisplayRole).toString();

                                const auto& aliases = getSettingsManager()->getAliases();

                                auto foundAlias = aliases.find(aliasName);

                                if(foundAlias != aliases.end())
                                {
                                    result = foundAlias->regex;
                                    QStringList aliases;
                                    aliases.append(foundAlias->alias);
                                    incrementPatternsCounter(aliases);
                                }
                            }
                        }
                    }
                }

                return result;
            };

            auto insertText = [this](const QString& text)
            {
                auto* pCompleter = mpRegexTextEdit->completer();

                if(nullptr != pCompleter)
                {
                    clearSuggestions();
                    mpRegexTextEdit->selectAll();
                    mpRegexTextEdit->insertPlainText(text);
                }
            };

            if(true == mCompletionData.bAppendMode)
            {
                QString previousText = mpRegexTextEdit->toPlainText();

                if(true == sFindLastPipeRegex.isValid())
                {
                    auto foundMatch = sFindLastPipeRegex.match(previousText);

                    if(foundMatch.hasMatch())
                    {
                        if(foundMatch.lastCapturedIndex() >= 3)
                        {
                            insertText(foundMatch.captured(1) + "|" + resolveText());
                        }
                    }
                    else
                    {
                        insertText(resolveText());
                    }
                }
            }
            else
            {
                insertText(resolveText());
            }
        });

        pCompleter->initFinished();
    }

    connect(mpDLTMessageAnalyzerController.get(), &IDLTMessageAnalyzerController::analysisStarted,
            this, [this](const tRequestId&, const QString& usedRegex, const QStringList& selectedAliases)
    {
        setSuggestionActive(false);

        if(true == selectedAliases.empty())
        {
            incrementRegexTextCounter(usedRegex);
        }

        incrementPatternsCounter(selectedAliases);
    });
}

CRegexHistoryProvider::~CRegexHistoryProvider()
{
}

void CRegexHistoryProvider::handleSettingsManagerChange()
{
    if(nullptr != getSettingsManager())
    {
        disconnect(getSettingsManager().get(), nullptr, this, nullptr);
        connect(getSettingsManager().get(), &ISettingsManager::aliasesChanged,
                this, [this]( const ISettingsManager::tAliasItemMap& aliases )
        {
            handleAliasesChange(aliases);
        });
    }

    auto regexUsageStatistics = getSettingsManager()->getRegexUsageStatistics();
    deleteNonRelevantElements(regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::TEXT]);
    deleteNonRelevantElements(regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::STORED_REGEX_PATTERN]);
    getSettingsManager()->setRegexUsageStatistics(regexUsageStatistics);
}

CRegexHistoryProvider::tSuggestionsMap CRegexHistoryProvider::getSuggestions(const QString& input)
{
    tSuggestionsMap result;

    auto normalizedInput = input;

    if(true == sFindLastPipeRegex.isValid())
    {
        auto foundMatch = sFindLastPipeRegex.match(input);

        if(foundMatch.hasMatch())
        {
            mCompletionData.bAppendMode = true;

            if(foundMatch.lastCapturedIndex() >= 3)
            {
                normalizedInput = foundMatch.captured(3);
            }
        }
        else
        {
            mCompletionData.bAppendMode = false;
        }
    }

    const auto& regexUsageStatistics = getSettingsManager()->getRegexUsageStatistics();

    auto caseSensitiveOption = getSettingsManager()->getRegexCompletion_CaseSensitive() ?
                Qt::CaseSensitive :
                Qt::CaseInsensitive;

    for (auto it = regexUsageStatistics.keyValueBegin(); it != regexUsageStatistics.keyValueEnd(); ++it)
    {
        for(auto jt = it->second.keyValueBegin(); jt != it->second.keyValueEnd(); ++jt)
        {
            bool bStringFound = false;

            if(false == getSettingsManager()->getRegexCompletion_SearchPolicy())
            {
                bStringFound = jt->first.startsWith(normalizedInput, caseSensitiveOption);
            }
            else
            {
                bStringFound = jt->first.contains(normalizedInput, caseSensitiveOption);
            }

            if(true == bStringFound)
            {
                tSuggestionData suggestionData;
                suggestionData.regexUsageStatisticsItemType = it->first;
                suggestionData.suggestionKey = jt->first;
                result.insert(std::make_pair(jt->second.updateDateTime.toMSecsSinceEpoch(), suggestionData));
            }
        }
    }

    return result;
}

int CRegexHistoryProvider::updateSuggestions(const QString& input)
{
    int result = 0;

    if(nullptr != mCompletionData.pCompleter
       && nullptr != mpRegexTextEdit)
    {
        QStandardItemModel* pModel = static_cast<QStandardItemModel*>(mCompletionData.pCompleter->model());
        pModel->clear();

        auto suggestions = getSuggestions(input);

        auto numberOfAssignedSuggestions = 0;
        for(const auto& suggestion_pair : suggestions)
        {
            QStandardItem* pStandardItem = nullptr;

            switch(suggestion_pair.second.regexUsageStatisticsItemType)
            {
                case ISettingsManager::eRegexUsageStatisticsItemType::STORED_REGEX_PATTERN:
                {
                    pStandardItem = new QStandardItem(QIcon(":/dltmessageanalyzer/png/alias_sug.png"),
                                                      suggestion_pair.second.suggestionKey);
                }
                break;
                case ISettingsManager::eRegexUsageStatisticsItemType::TEXT:
                {
                    pStandardItem = new QStandardItem(QIcon(":/dltmessageanalyzer/png/regex_sug.png"),
                                                      suggestion_pair.second.suggestionKey);
                }
                break;
            }

            QVariant additionalData = static_cast<int>(suggestion_pair.second.regexUsageStatisticsItemType);
            pStandardItem->setData(additionalData, sSuggestionTypeRole);
            pModel->appendRow(pStandardItem);

            ++numberOfAssignedSuggestions;
        }

        result = numberOfAssignedSuggestions;

        mCompletionData.pCompleter->complete();
    }

    return result;
}

void CRegexHistoryProvider::clearSuggestions()
{
    if(nullptr != mCompletionData.pCompleter)
    {
        QStandardItemModel* pModel = static_cast<QStandardItemModel*>(mCompletionData.pCompleter->model());
        pModel->clear();
        mCompletionData.pCompleter->complete();
    }
}

void CRegexHistoryProvider::incrementPatternsCounter(const QStringList& aliases)
{
    auto regexUsageStatistics = getSettingsManager()->getRegexUsageStatistics();

    for(const auto& item : aliases)
    {
        auto& updateItem = regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::STORED_REGEX_PATTERN][item];
        ++updateItem.usageCounter;
        updateItem.updateDateTime = QDateTime::currentDateTime();
    }

    auto& mapToCheck = regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::STORED_REGEX_PATTERN];
    if(true == reachedTheSizeLimit(mapToCheck))
    {
        deleteNonRelevantElements(mapToCheck);
    }

    getSettingsManager()->setRegexUsageStatistics(regexUsageStatistics);
}

void CRegexHistoryProvider::incrementRegexTextCounter(const QString& regex)
{
    auto regexUsageStatistics = getSettingsManager()->getRegexUsageStatistics();

    auto& updateItem = regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::TEXT][regex];
    ++updateItem.usageCounter;
    updateItem.updateDateTime = QDateTime::currentDateTime();

    auto& mapToCheck = regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::TEXT];
    if(true == reachedTheSizeLimit(mapToCheck))
    {
        deleteNonRelevantElements(mapToCheck);
    }

    getSettingsManager()->setRegexUsageStatistics(regexUsageStatistics);
}

void CRegexHistoryProvider::handleAliasesChange(const ISettingsManager::tAliasItemMap& aliases)
{
    auto regexUsageStatistics = getSettingsManager()->getRegexUsageStatistics();
    auto& aliasesUsageStatistics = regexUsageStatistics[ISettingsManager::eRegexUsageStatisticsItemType::STORED_REGEX_PATTERN];
    QSet<QString> aliasesSet;

    for(const auto& item : aliases)
    {
        aliasesSet.insert(item.alias);
    }

    QList<QString> keysToRemove;

    // Iterate over the QMap
    for (auto it = aliasesUsageStatistics.begin(); it != aliasesUsageStatistics.end(); ++it) {
        if (!aliasesSet.contains(it.key())) {
            keysToRemove.append(it.key());  // Collect keys that are not in QSet
        }
    }

    // Remove the collected keys from the QMap
    for (const QString& key : keysToRemove) {
        aliasesUsageStatistics.remove(key);
    }

    getSettingsManager()->setRegexUsageStatistics(regexUsageStatistics);
}

bool CRegexHistoryProvider::getSuggestionActive()
{
    return mbSuggestionActive;
}

void CRegexHistoryProvider::setSuggestionActive(bool val)
{
    if(false == val)
    {
        clearSuggestions();
    }

    mbSuggestionActive = val;
}

PUML_PACKAGE_BEGIN(DMA_RegexHistory)
    PUML_CLASS_BEGIN_CHECKED(CExtendedCompleter)
        PUML_INHERITANCE_CHECKED(QCompleter, extends)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN_CHECKED(CRegexHistoryProvider)
        PUML_INHERITANCE_CHECKED(IRegexHistoryProvider, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CRegexHistoryTextEdit, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CPatternsView, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IDLTMessageAnalyzerController, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CExtendedCompleter, 1, 1, creates)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QListView, 1, 1, creates)
    PUML_CLASS_END()
PUML_PACKAGE_END()
