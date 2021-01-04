#pragma once

#include "map"
#include "string"
#include "functional"
#include "deque"

#include <QLineEdit>

#include "components/settings/api/CSettingsManagerClient.hpp"

class CConsoleInputProcessor : public QObject,
                               public CSettingsManagerClient
{
    Q_OBJECT

public:

    typedef QString tScenarioTag;
    typedef QString tParamName;
    typedef QString tParamValue;

    typedef std::map<tParamName, tParamValue> tParamMap;

    typedef std::function<void(const tParamMap&)> tScenarioHandler;

    struct tScenarioData
    {
        tScenarioData() = default;
        tScenarioData( const tScenarioHandler& scenarioHandler_,
                       const QString& comment_ );
        tScenarioHandler scenarioHandler;
        QString comment;
    };

    typedef std::map<tScenarioTag, tScenarioData> tScenariosMap;

    CConsoleInputProcessor( QLineEdit* pTargetLineEdit,
                            const tSettingsManagerPtr& pSettingsManager );
    virtual ~CConsoleInputProcessor();

protected:
    bool eventFilter(QObject* pObj, QEvent* pEvent) override;

private:
    CConsoleInputProcessor::tScenariosMap createScenariosMap();
    void printHelp(const QString& command);

private:

    QLineEdit* mpTargetLineEdit;
    tScenariosMap mScenariosMap;
    std::deque<QString> mHistory;
    int mCurrentHistoryItem;

    enum eHistoryBorderStatus
    {
        Reached_Both = 0,
        Reached_First,
        Reached_Last,
        Not_Near_Border
    };

    eHistoryBorderStatus mbBorderReached;
};
