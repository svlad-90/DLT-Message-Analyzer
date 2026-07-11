#include "../api/CLog.hpp"

#include "CConsoleCtrl.hpp"

#include <cstdio>

namespace DMA_Log
{
    static void printToConsole(const QString& str, FILE* stream)
    {
        const QByteArray data = str.toLocal8Bit();
        std::fprintf(stream, "%s\n", data.constData());
        std::fflush(stream);
    }

    void sendMessage(const QString& str)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eMsg;
        if(NDLTMessageAnalyzer::NConsole::CConsoleCtrl::isExist())
        {
            NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
        }
        else
        {
            printToConsole(str, stdout);
        }
    }

    void sendWarning(const QString& str)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eWrn;
        if(NDLTMessageAnalyzer::NConsole::CConsoleCtrl::isExist())
        {
            NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
        }
        else
        {
            printToConsole(str, stderr);
        }
    }

    void sendError(const QString& str)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eErr;
        if(NDLTMessageAnalyzer::NConsole::CConsoleCtrl::isExist())
        {
            NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
        }
        else
        {
            printToConsole(str, stderr);
        }
    }

    void sendMessageColored(const QString& str, const QColor& color)
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.messageType = NDLTMessageAnalyzer::NConsole::eMessageType::eWrn;
        messageSettings.bCustomColor = true;
        messageSettings.color = color;
        if(NDLTMessageAnalyzer::NConsole::CConsoleCtrl::isExist())
        {
            NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage(str, messageSettings);
        }
        else
        {
            printToConsole(str, stdout);
        }
    }

    void clearConsole()
    {
        NDLTMessageAnalyzer::NConsole::tMessageSettings messageSettings;
        messageSettings.clear = true;
        if(NDLTMessageAnalyzer::NConsole::CConsoleCtrl::isExist())
        {
            NDLTMessageAnalyzer::NConsole::CConsoleCtrl::sendMessage("", messageSettings);
        }
    }
}
