#include <atomic>
#include <mutex>
#include <memory>

#include <QDateTime>
#include <QTabBar>
#include <QThread>

#include "CConsoleCtrl.hpp"

Q_DECLARE_METATYPE(NDLTMessageAnalyzer::NConsole::eMessageType)

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
    eMessageType messageType;
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

void CConsoleCtrl::sendMessage( const QString& message, eMessageType messageType )
{
    qRegisterMetaType<eMessageType>("eMessageType");

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
                                              Q_ARG(eMessageType, messageBufferItem.messageType));
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
                                  Q_ARG(eMessageType, messageType));
    }
    else
    {
        const std::lock_guard<std::mutex> guard(g_MessageBufferProtector());

        tMessageBufferItem bufferItem;
        bufferItem.message = message;
        bufferItem.messageType = messageType;
        g_MessageBuffer().push_back(bufferItem);
    }
}

CConsoleCtrl::CConsoleCtrl( const tConsoleConfig& consoleConfig ):
mConsoleConfig(consoleConfig),
mMessageCounters(),
mCountedMessageType(eMessageType::eMsg)
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
                       pTabBar->setTabTextColor(index, QColor(0,0,0));
                       pTabBar->setTabText(index, g_TabName());
                       mMessageCounters.clear();
                       mCountedMessageType = eMessageType::eMsg;
                   }
               }
            });
        }
    }
}

static const QString msgHtml = "<font color=\"Black\">";
static const QString wrnHtml = "<font color=\"#969600\">";
static const QString errHtml = "<font color=\"#960000\">";
static const QString endHtml = "</font>";
static const QColor msgColor = QColor(0,0,0);
static const QColor wrnColor = QColor(150,150,0);
static const QColor errColor = QColor(150,0,0);

void CConsoleCtrl::addMessage( const QString& message, eMessageType messageType )
{
    if(nullptr != mConsoleConfig.pConsoleTextEdit)
    {
        QString messageEscaped = message.toHtmlEscaped();
        QString HTMLMessage;

        switch(messageType)
        {
            case eMessageType::eMsg: HTMLMessage.append(msgHtml); break;
            case eMessageType::eWrn: HTMLMessage.append(wrnHtml); break;
            case eMessageType::eErr: HTMLMessage.append(errHtml); break;
        }

        HTMLMessage.append("[").append(QDateTime::currentDateTime().toString()).append("]");

        switch(messageType)
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

        mConsoleConfig.pConsoleTextEdit->appendHtml(HTMLMessage);
    }

    if(nullptr != mConsoleConfig.pTabWidget && nullptr != mConsoleConfig.pConsoleTab)
    {
        auto* pTabWidget = mConsoleConfig.pTabWidget;
        auto* pConsoleTab = mConsoleConfig.pConsoleTab;
        auto* pTabBar = pTabWidget->tabBar();

        auto consoleTabIndex = pTabWidget->indexOf(pConsoleTab);

        ++mMessageCounters[messageType];

        if(consoleTabIndex != pTabWidget->currentIndex()) // if console view tab is not selected
        {
            if(messageType >= mCountedMessageType)
            {
                mCountedMessageType = messageType;

                // update tab's highlighting
                QColor tabHighlightingColor;

                switch(messageType)
                {
                    case eMessageType::eMsg: tabHighlightingColor = msgColor; break;
                    case eMessageType::eWrn: tabHighlightingColor = wrnColor; break;
                    case eMessageType::eErr: tabHighlightingColor = errColor; break;
                }

                pTabBar->setTabTextColor(consoleTabIndex, tabHighlightingColor);

                // update tab's text
                pTabBar->setTabText( consoleTabIndex,
                                     QString(g_TabName()).append(" (%1)").arg(QString::number(mMessageCounters[messageType])));
            }
        }
        else
        {
            // reset counter
            mMessageCounters[messageType] = 0u;
        }
    }
}
