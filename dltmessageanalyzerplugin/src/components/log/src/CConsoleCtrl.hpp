#pragma once

#include <QMap>

#include "QPlainTextEdit"
#include "QTabWidget"
#include "QTimer"

namespace NDLTMessageAnalyzer
{
    namespace NConsole
    {
        enum class eMessageType
        {
            eMsg = 0,
            eWrn,
            eErr
        };

        struct tMessageSettings
        {
            bool bCustomColor = false;
            QColor color;
            eMessageType messageType = eMessageType::eMsg;
            bool clear = false;
        };

        struct tConsoleConfig
        {
            // size of log ring buffer, which is represente to the user. Measured in number of dedicated messages.
            unsigned int logSize = 1000;
            // Maximum size of single message. Measures in number of UTF-8 characters.
            unsigned int maxMsgSize = 1024;
            // consoleConfig - tab widget, which contains all tabs, including our console
            QTabWidget* pTabWidget = nullptr;
            // pConsoleTab - widget, which is a tab within pTabWidget
            QWidget* pConsoleTab = nullptr;
            // pConsoleTextEdit - the line edit, which is part of the pConsoleTab
            QPlainTextEdit* pConsoleTextEdit = nullptr;
        };

        /**
         * @brief The CConsoleCtrl class - this purely statis class is used to send messages from any Qt-based thread to a GUI thread and place them into provided
         * text edit.
         * Used to implement debug console within the DLTMessageAnalyzer plugin.
         */
        class CConsoleCtrl : public QObject
        {
            Q_OBJECT

        public:

            /**
             * @brief createInstance - creates singletone instance
             * In case if it already exists - will do nothing
             * Method is thread-safe.
             * Still it should be called ONLY!!! from the same QT-based thread, in which console config widgets are located.
             * @param consoleConfig - console configuration, which is used to create a signletone instance of console controller.
             */
            static void createInstance( const tConsoleConfig& consoleConfig );

            /**
             * @brief destroyInstance - destroys singletone instance, if it exists.
             * Otherwise does nothing.
             * Method is NOT thread-safe.
             * Should be called from the same thread, from which "createInstance" was previously called.
             */
            static void destroyInstance();

            /**
             * @brief isExist - tells, whether created instance exists
             * Method is thread-safe.
             * @return - true, if instance exist. False otherwise.
             */
            static bool isExist();

            /**
             * @brief sendMessage - sends a message to console, if created instance exists.
             * Otherwise does nothing.
             * Method is thread-safe.
             * @param message - message to be added to the log
             * @param messageSettings - set of the message settings, which should be considered.
             */
            static void sendMessage( const QString& message,
                                     const tMessageSettings& messageSettings );

private slots:
            /**
             * @brief addMessage - when event form the client reaches thread, in which signletone instance is located -
             * sends a message to the corresponding text edit.
             * Depending on type of the message, highlights
             * @param message - string of the message, to be added to console
             * @param messageSettings - set of the message settings, which should be considered.
             */
            void addMessage( const QString& message, const tMessageSettings& messageSettings );

        private: // methods
            /**
             * @brief CConsoleCtrl - constructor
             * @param consoleConfig - console configuration, which is used to create a signletone instance of console controller.
             */
            CConsoleCtrl( const tConsoleConfig& consoleConfig );

        private: // fields
            tConsoleConfig mConsoleConfig;
            typedef QMap<eMessageType, unsigned int> tMessageCounters;
            tMessageCounters mMessageCounters;
            eMessageType mCountedMessageType;
            QStringList mBufferedMessages;
            QTimer mFlushBufferedMessagsTimer;
        };
    }
}
