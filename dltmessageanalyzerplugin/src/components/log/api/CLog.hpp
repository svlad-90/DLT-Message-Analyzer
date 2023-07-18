#pragma once

#include <QString>
#include <QColor>

namespace DMA_Log
{
    extern "C" void sendMessage(const QString& str);
    extern "C" void sendWarning(const QString& str);
    extern "C" void sendError(const QString& str);
    extern "C" void sendMessageColored(const QString& str, const QColor& color);
    extern "C" void clearConsole();
}

/**
 * Sends debug message to console. Takes string as an argument.
 */
#define SEND_MSG(STRING)\
do\
{\
    DMA_Log::sendMessage(STRING);\
}while(false)

/**
 * Sends warning message to console. Takes string as an argument.
 */
#define SEND_WRN(STRING)\
do\
{\
    DMA_Log::sendWarning(STRING);\
}while(false)

/**
 * Sends error message to console. Takes string as an argument.
 */
#define SEND_ERR(STRING)\
do\
{\
    DMA_Log::sendError(STRING);\
}while(false)

/**
 * Sends debug message of specified color to console. Takes string and color as arguments.
 */
#define SEND_MSG_COLORED(STRING, COLOR)\
do\
{\
    DMA_Log::sendMessageColored(STRING, COLOR);\
}while(false)

#define CLEAR_CONSOLE_VIEW()\
do\
{\
    DMA_Log::clearConsole();\
}while(false)
