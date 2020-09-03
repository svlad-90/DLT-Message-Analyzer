#include "CLog.hpp"

#include "CConsoleCtrl.hpp"

namespace DMA_Log
{
    void sendMessage(const QString& str)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eMsg;
        NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
    }

    void sendWarning(const QString& str)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eWrn;
        NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
    }

    void sendError(const QString& str)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eErr;
        NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
    }

    void sendMessageColored(const QString& str, const QColor& color)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eWrn;
        messageSettings.bCustomColor = true;
        messageSettings.color = color;
        NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
    }

    void clearConsole()
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.clear = true;
        NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage("", messageSettings);
    }
}

