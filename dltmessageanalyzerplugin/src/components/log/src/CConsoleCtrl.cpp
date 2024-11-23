#include <atomic>
#include <mutex>
#include <memory>

#include <QApplication>
#include <QDateTime>
#include <QTabBar>
#include <QThread>

#include "common/Definitions.hpp"
#include "CConsoleCtrl.hpp"

#include "DMA_Plantuml.hpp"

Q_DECLARE_METATYPE(NDLTMessageAnalyzer::NConsole::tMessageSettings)

using namespace NDLTMessageAnalyzer::NConsole;

static const char* g_TabName()
{
    return "Console view";
}

/////////////////////////MESSAGE_BUFFER ACCESS/////////////////////////
static std::atomic<bool>& g_IsBufferApplied()
{
    static std::atomic<bool> sIsBufferApplied;
    return sIsBufferApplied;
}

static std::mutex& g_MessageBufferProtector()
{
    static std::mutex sMessageBufferProtector;
    return sMessageBufferProtector;
}

struct tMessageBufferItem
{
    tMessageSettings messageSettings;
    QString message;
};

typedef QList<tMessageBufferItem> tMessageBufferList;

static tMessageBufferList& g_MessageBuffer()
{
    static tMessageBufferList sMessageBuffer;
    return sMessageBuffer;
}

/////////////////////////INSTANCE_ACCESS/////////////////////////
static std::atomic<bool>& g_IsExist()
{
    static std::atomic<bool> sIsExist;
    return sIsExist;
}

static std::mutex& g_InstanceProtector()
{
    static std::mutex sInstanceProtector;
    return sInstanceProtector;
}

static std::unique_ptr<CConsoleCtrl>& g_InstancePtr()
{
    static std::unique_ptr<CConsoleCtrl> spInstance(nullptr);
    return spInstance;
}

/////////////////////////OTHER/////////////////////////
void CConsoleCtrl::createInstance( const tConsoleConfig& consoleConfig )
{
    if(false == g_IsExist())
    {
        const std::lock_guard<std::mutex> guard(g_InstanceProtector());
        g_InstancePtr() = std::unique_ptr<CConsoleCtrl>(new CConsoleCtrl(consoleConfig));
        g_IsExist() = true;
    }
}

void CConsoleCtrl::destroyInstance()
{
    if(true == g_IsExist())
    {
        const std::lock_guard<std::mutex> guard(g_InstanceProtector());
        g_InstancePtr().reset();
        g_IsExist() = false;
    }
}

bool CConsoleCtrl::isExist()
{
    return g_IsExist();
}

void CConsoleCtrl::sendMessage( const QString& message,
                                const tMessageSettings& messageSettings )
{
    qRegisterMetaType<tMessageSettings>("tMessageSettings");

    if( true == g_IsExist() )
    {
        auto& isBufferApplied = g_IsBufferApplied();

        if(false == isBufferApplied)
        {
            const std::lock_guard<std::mutex> guard(g_MessageBufferProtector());

            if(false == isBufferApplied) // let's double check, to prevent duplicated messages.
            {
                auto& messageBuffer = g_MessageBuffer();

                for(const auto& messageBufferItem : messageBuffer)
                {
                    QMetaObject::invokeMethod(g_InstancePtr().get(), "addMessage", Qt::QueuedConnection,
                                              Q_ARG(QString, messageBufferItem.message),
                                              Q_ARG(tMessageSettings, messageBufferItem.messageSettings));
                }

                messageBuffer.clear();
                isBufferApplied = true;
            }
        }

        QString TID = QString("0x%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()),
                            QT_POINTER_SIZE * 2, 16, QChar('0'));
        QString messageExtended( QString("[TID::%1] : %2")
                                 .arg(TID)
                                 .arg(message));

        QMetaObject::invokeMethod(g_InstancePtr().get(), "addMessage", Qt::QueuedConnection,
                                  Q_ARG(QString, messageExtended),
                                  Q_ARG(tMessageSettings, messageSettings));
    }
    else
    {
        const std::lock_guard<std::mutex> guard(g_MessageBufferProtector());

        tMessageBufferItem bufferItem;
        bufferItem.message = message;
        bufferItem.messageSettings = messageSettings;
        g_MessageBuffer().push_back(bufferItem);
    }
}

CConsoleCtrl::CConsoleCtrl( const tConsoleConfig& consoleConfig ):
mConsoleConfig(consoleConfig),
mMessageCounters(),
mCountedMessageType(eMessageType::eMsg),
mBufferedMessages(),
mFlushBufferedMessagsTimer()
{
    if(nullptr != mConsoleConfig.pConsoleTextEdit)
    {
        consoleConfig.pConsoleTextEdit->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse|
                                                                Qt::TextInteractionFlag::TextSelectableByKeyboard);

        QFont font("Monospace", 9);
        font.setStyleHint(QFont::TypeWriter);
        consoleConfig.pConsoleTextEdit->setFont(font);
        consoleConfig.pConsoleTextEdit->setMaximumBlockCount(static_cast<int>(mConsoleConfig.logSize));

        auto* pTabWidget = mConsoleConfig.pTabWidget;
        auto* pConsoleTab = mConsoleConfig.pConsoleTab;
        auto* pTabBar = pTabWidget->tabBar();

        if(nullptr != pTabWidget && nullptr != pConsoleTab && nullptr != pTabBar)
        {
            auto indexOfConsoleView = pTabWidget->indexOf(pConsoleTab);
            pTabBar->setTabText(indexOfConsoleView, g_TabName());
        }

        if(nullptr != pTabWidget)
        {
            connect( pTabWidget, &QTabWidget::tabBarClicked, [this, pConsoleTab, pTabWidget, pTabBar](int index)
            {
               if(nullptr != pTabWidget && nullptr != pConsoleTab && nullptr != pTabBar)
               {
                   auto indexOfConsoleView = pTabWidget->indexOf(pConsoleTab);

                   if(index == indexOfConsoleView)
                   {
                       pTabBar->setTabTextColor(index, qApp->palette().text().color());
                       pTabBar->setTabText(index, g_TabName());
                       mMessageCounters.clear();
                       mCountedMessageType = eMessageType::eMsg;
                   }
               }
            });
        }
    }

    connect(&mFlushBufferedMessagsTimer, &QTimer::timeout, this, [this]()
    {
        if(mConsoleConfig.pConsoleTextEdit)
        {
            if(!mBufferedMessages.empty())
            {
                QString finalMessage;

                auto sizeCounter = 0;

                for(const auto& message : mBufferedMessages)
                {
                    sizeCounter += message.size();
                }

                finalMessage.reserve(sizeCounter);

                for(const auto& message : mBufferedMessages)
                {
                    finalMessage.push_back(message);
                }

                mConsoleConfig.pConsoleTextEdit->appendHtml(finalMessage);

                mBufferedMessages.clear();
            }
        }

        mFlushBufferedMessagsTimer.stop();
    });
}

void CConsoleCtrl::addMessage( const QString& message, const tMessageSettings& messageSettings )
{
    const QColor textColor = qApp->palette().text().color();
    QString colorStr = QString("#%1").arg(textColor.rgba(), 8, 16);

    bool isDarkModeOn = isDarkMode();
    const QString msgHtml = "<font color=\"" + colorStr + "\">";
    QString wrnStr = isDarkModeOn ? "#fafa00" : "#969600";
    const QString wrnHtml = "<font color=\"" + wrnStr + "\">";
    QString errorStr = isDarkModeOn ? "#fa0000" : "#960000";
    const QString errHtml = "<font color=\"" + errorStr + "\">";
    const QString endHtml = "</font>";
    const QColor msgColor = qApp->palette().text().color();
    const QColor wrnColor = isDarkModeOn ? QColor(250,250,0) : QColor(150,150,0);
    const QColor errColor = isDarkModeOn ? QColor(250,0,0) : QColor(150,0,0);

    if(nullptr != mConsoleConfig.pConsoleTextEdit)
    {
        if(true == messageSettings.clear)
        {
            mConsoleConfig.pConsoleTextEdit->clear();
        }
        else
        {

            QString messageEscaped = message.toHtmlEscaped();
            messageEscaped.replace("\n", "<br/>");

            QString HTMLMessage;
            HTMLMessage.reserve(messageEscaped.size() + 50);

            HTMLMessage.append("<pre>");

            if(true == messageSettings.bCustomColor)
            {
                HTMLMessage.append("<font color=\"").append(rgb2hex(messageSettings.color)).append("\">");
            }
            else
            {
                switch(messageSettings.messageType)
                {
                    case eMessageType::eMsg: HTMLMessage.append(msgHtml); break;
                    case eMessageType::eWrn: HTMLMessage.append(wrnHtml); break;
                    case eMessageType::eErr: HTMLMessage.append(errHtml); break;
                }
            }

            HTMLMessage.append("[").append(QDateTime::currentDateTime().toString()).append("]");

            switch(messageSettings.messageType)
            {
                case eMessageType::eMsg: HTMLMessage.append("[M]"); break;
                case eMessageType::eWrn: HTMLMessage.append("[W]"); break;
                case eMessageType::eErr: HTMLMessage.append("[E]"); break;
            }

            HTMLMessage.append(" : ");

            QString messageNormalized = messageEscaped.size() >= static_cast<int>(mConsoleConfig.maxMsgSize)
                    ? messageEscaped.mid(0, static_cast<int>(mConsoleConfig.maxMsgSize)).append("...")
                    : messageEscaped;

            HTMLMessage.append(messageNormalized);
            HTMLMessage.append(endHtml);

            HTMLMessage.append("</pre>");

            mBufferedMessages.push_back(HTMLMessage);
            mFlushBufferedMessagsTimer.start(100);
        }

        if(nullptr != mConsoleConfig.pTabWidget && nullptr != mConsoleConfig.pConsoleTab)
        {
            auto* pTabWidget = mConsoleConfig.pTabWidget;
            auto* pConsoleTab = mConsoleConfig.pConsoleTab;
            auto* pTabBar = pTabWidget->tabBar();

            auto consoleTabIndex = pTabWidget->indexOf(pConsoleTab);

            ++mMessageCounters[messageSettings.messageType];

            if(consoleTabIndex != pTabWidget->currentIndex()) // if console view tab is not selected
            {
                if(messageSettings.messageType >= mCountedMessageType)
                {
                    mCountedMessageType = messageSettings.messageType;

                    // update tab's highlighting
                    QColor tabHighlightingColor;

                    switch(messageSettings.messageType)
                    {
                        case eMessageType::eMsg: tabHighlightingColor = msgColor; break;
                        case eMessageType::eWrn: tabHighlightingColor = wrnColor; break;
                        case eMessageType::eErr: tabHighlightingColor = errColor; break;
                    }

                    pTabBar->setTabTextColor(consoleTabIndex, tabHighlightingColor);

                    // update tab's text
                    pTabBar->setTabText( consoleTabIndex,
                                         QString(g_TabName()).append(" (%1)").arg(QString::number(mMessageCounters[messageSettings.messageType])));
                }
            }
            else
            {
                // reset counter
                mMessageCounters[messageSettings.messageType] = 0u;
            }
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_Log)
    PUML_SINGLETONE_BEGIN_CHECKED(CConsoleCtrl)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTabWidget, 1, 1, console view tab widget)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QWidget, 1, 1, console tab)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QPlainTextEdit, 1, 1, console text edit)
    PUML_SINGLETONE_END()
PUML_PACKAGE_END()
