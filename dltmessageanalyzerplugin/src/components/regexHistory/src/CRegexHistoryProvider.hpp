#pragma once

#include <map>

#include "QLineEdit"
#include "QListView"
#include "QTimer"
#include "QCompleter"

#include "components/regexHistory/api/IRegexHistoryProvider.hpp"
#include "components/settings/api/CSettingsManagerClient.hpp"
#include "components/patternsView/api/CPatternsView.hpp"
#include "components/analyzer/api/IDLTMessageAnalyzerController.hpp"

class CRegexHistoryTextEdit;

class CExtendedCompleter : public QCompleter
{
    Q_OBJECT
public:
    CExtendedCompleter(QObject *parent,
                      CRegexHistoryTextEdit* pRegexTextEdit);
    void initFinished();

signals:
    void loseFocus();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    bool mbStartProcessEventFilter;
    CRegexHistoryTextEdit* mpRegexTextEdit;
};

class CRegexHistoryProvider : public IRegexHistoryProvider,
                              public CSettingsManagerClient
{
    Q_OBJECT

public:


    /**
     * @brief IRegexHistoryProvider - contructor
     * @param pSettingsManager - instance of the settings manager
     * @param pLineEdit - instance of the taraget line edit
     */
    CRegexHistoryProvider(const tSettingsManagerPtr& pSettingsManager,
                          CRegexHistoryTextEdit* pRegexTextEdit, CPatternsView* pPatternsView,
                          const tDLTMessageAnalyzerControllerPtr& pDLTMessageAnalyzerController);

    ~CRegexHistoryProvider();

public:
    int updateSuggestions(const QString& input) override;
    void clearSuggestions() override;
    bool getSuggestionActive() override;
    void setSuggestionActive(bool val) override;


protected:
    void handleSettingsManagerChange() override;

    typedef int64_t tTimeSinceEpochMs;

    struct DescendingOrder
    {
        bool operator()(const tTimeSinceEpochMs &a, const tTimeSinceEpochMs &b) const
        {
            return a > b; // Return true if 'a' should go before 'b'
        }
    };

    typedef QString tSuggestionKey;
    struct tSuggestionData
    {
        ISettingsManager::eRegexUsageStatisticsItemType regexUsageStatisticsItemType;
        tSuggestionKey suggestionKey;
    };

    typedef std::multimap<tTimeSinceEpochMs, tSuggestionData, DescendingOrder> tSuggestionsMap;

    tSuggestionsMap getSuggestions(const QString& input);

private:
    void incrementPatternsCounter(const QStringList& aliases);
    void incrementRegexTextCounter(const QString& regex);
    void handleAliasesChange(const ISettingsManager::tAliasItemMap& aliases);

private:
    struct tCompletionData
    {
        CExtendedCompleter* pCompleter = nullptr;
        QListView* pPopUp = nullptr;
        bool bAppendMode = false;
    };

    tCompletionData mCompletionData;

    CRegexHistoryTextEdit* mpRegexTextEdit;
    CPatternsView* mpPatternsView;
    tDLTMessageAnalyzerControllerPtr mpDLTMessageAnalyzerController;
    bool mbSuggestionActive;
};
