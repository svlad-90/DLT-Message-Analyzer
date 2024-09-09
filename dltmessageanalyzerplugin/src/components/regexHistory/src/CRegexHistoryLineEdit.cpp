#include "QCompleter"

#include "../api/CRegexHistoryLineEdit.hpp"

#include "DMA_Plantuml.hpp"

CRegexHistoryLineEdit::CRegexHistoryLineEdit(QWidget *parent):
    QLineEdit(parent),
    mpRegexHistoryProvider(nullptr),
    mbIgnoreReturnKeyEvent(false)
{}

void CRegexHistoryLineEdit::focusInEvent(QFocusEvent *event)
{
    //SEND_MSG(QString("%1").arg(__FUNCTION__));
    QLineEdit::focusInEvent(event);

    if(nullptr != completer())
    {
        //SEND_MSG(QString("%1 completer not nullptr").arg(__FUNCTION__));
        disconnect(completer(), QOverload<const QString &>::of(&QCompleter::activated),
                   static_cast<QLineEdit*>(this), nullptr);
        disconnect(completer(), QOverload<const QModelIndex &>::of(&QCompleter::activated),
                   static_cast<QLineEdit*>(this), nullptr);
        disconnect(completer(), QOverload<const QString &>::of(&QCompleter::highlighted),
                   static_cast<QLineEdit*>(this), nullptr);
        disconnect(completer(), QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
                   static_cast<QLineEdit*>(this), nullptr);
    }
}

void CRegexHistoryLineEdit::setRegexHistoryProvider(const tRegexHistoryProviderPtr& pRegexHistoryProviderPtr)
{
    mpRegexHistoryProvider = pRegexHistoryProviderPtr;
}

void CRegexHistoryLineEdit::setIgnoreReturnKeyEvent(bool val)
{
    mbIgnoreReturnKeyEvent = val;
}

bool CRegexHistoryLineEdit::getIgnoreReturnKeyEvent() const
{
    return mbIgnoreReturnKeyEvent;
}

void CRegexHistoryLineEdit::activateRegexHistory()
{
    if(nullptr != mpRegexHistoryProvider)
    {
        mpRegexHistoryProvider->setSuggestionActive(true);
        int numberOfSuggestions = mpRegexHistoryProvider->updateSuggestions(text());

        if(0 == numberOfSuggestions)
        {
            mpRegexHistoryProvider->setSuggestionActive(false);
        }

        update();
    }
}

void CRegexHistoryLineEdit::keyPressEvent(QKeyEvent *event)
{
    if(nullptr != event)
    {
        if (event->key() == Qt::Key_Space && event->modifiers() == Qt::ControlModifier)
        {
            activateRegexHistory();
        }
        else if(event->key() == Qt::Key_Escape)
        {
            if(nullptr != mpRegexHistoryProvider &&
              true == mpRegexHistoryProvider->getSuggestionActive())
            {
                mpRegexHistoryProvider->setSuggestionActive(false);
                update();
            }
            else
            {
                QLineEdit::keyPressEvent(event);
            }
        }
        else
        {
            QString prevText = text();

            // Pass the event to the base class for normal processing
            QLineEdit::keyPressEvent(event);

            QString newText = text();

            if(prevText != newText)
            {
                if(nullptr != mpRegexHistoryProvider &&
                   true == mpRegexHistoryProvider->getSuggestionActive())
                {
                    static_cast<void>(mpRegexHistoryProvider->updateSuggestions(text()));
                    update();
                }
            }
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_RegexHistory_API)
    PUML_CLASS_BEGIN(CRegexHistoryLineEdit)
        PUML_INHERITANCE_CHECKED(QLineEdit, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IRegexHistoryProvider, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
