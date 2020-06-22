#ifndef CCONSOLEINPUTPROCESSOR_HPP
#define CCONSOLEINPUTPROCESSOR_HPP

#include "functional"

#include <QLineEdit>

class CConsoleInputProcessor : public QObject
{
    Q_OBJECT

public:

    typedef QString tScenarioTag;
    typedef std::function<void()> tScenarioHandler;
    typedef std::map<tScenarioTag, tScenarioHandler> tScenariosMap;

    CConsoleInputProcessor( QLineEdit* pTargetLineEdit );
    virtual ~CConsoleInputProcessor();

private:

    QLineEdit* mpTargetLineEdit;
    tScenariosMap mScenariosMap;
};

#endif // CCONSOLEINPUTPROCESSOR_HPP
