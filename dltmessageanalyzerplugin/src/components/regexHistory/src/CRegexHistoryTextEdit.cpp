#include "../api/CRegexHistoryTextEdit.hpp"

#include "QCompleter"
#include "QTextCursor"
#include "QMimeData"

#include "DMA_Plantuml.hpp"

CRegexHistoryTextEdit::CRegexHistoryTextEdit(QWidget *parent):
    QTextEdit(parent),
    mpRegexHistoryProvider(nullptr),
    mbIgnoreReturnKeyEvent(false),
    mpCompleter(nullptr)
{
    connect(this, &QTextEdit::textChanged, this, &CRegexHistoryTextEdit::adjustHeightToContent);
}

void CRegexHistoryTextEdit::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    adjustHeightToContent();
}

void CRegexHistoryTextEdit::handleSettingsManagerChange()
{
    if(getSettingsManager())
    {
        connect(getSettingsManager().get(), &ISettingsManager::regexInputFieldHeightChanged, this, [this](const int&)
        {
            adjustHeightToContent();
        });
    }
}

void CRegexHistoryTextEdit::adjustHeightToContent()
{
    if(getSettingsManager())
    {
        // Calculate the height of one line
        QFontMetrics fontMetrics(font());
        int lineHeight = fontMetrics.lineSpacing();

        // Calculate the height needed for the content
        int contentHeight = document()->size().height();

        auto regexInputFieldHeight = getSettingsManager()->getRegexInputFieldHeight();

        // Calculate the maximum height allowed (4 lines in this case)
        int maxHeight = lineHeight * regexInputFieldHeight + 10;

        // Set the height of the QTextEdit based on the content but capped at maxHeight
        setFixedHeight(qMin(contentHeight, maxHeight));

        if (regexInputFieldHeight == 1)
        {
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setLineWrapMode(QTextEdit::NoWrap);
        }
        else
        {
            setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            setLineWrapMode(QTextEdit::WidgetWidth);
        }
    }
}

void CRegexHistoryTextEdit::focusInEvent(QFocusEvent *event)
{
    //SEND_MSG(QString("%1").arg(__FUNCTION__));
    QTextEdit::focusInEvent(event);

    if(nullptr != completer())
    {
        //SEND_MSG(QString("%1 completer not nullptr").arg(__FUNCTION__));
        completer()->setWidget(this);

        disconnect(completer(), QOverload<const QString &>::of(&QCompleter::activated),
                   static_cast<QTextEdit*>(this), nullptr);
        disconnect(completer(), QOverload<const QModelIndex &>::of(&QCompleter::activated),
                   static_cast<QTextEdit*>(this), nullptr);
        disconnect(completer(), QOverload<const QString &>::of(&QCompleter::highlighted),
                   static_cast<QTextEdit*>(this), nullptr);
        disconnect(completer(), QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
                   static_cast<QTextEdit*>(this), nullptr);
    }
}

void CRegexHistoryTextEdit::setRegexHistoryProvider(const tRegexHistoryProviderPtr& pRegexHistoryProviderPtr)
{
    mpRegexHistoryProvider = pRegexHistoryProviderPtr;
}

void CRegexHistoryTextEdit::setIgnoreReturnKeyEvent(bool val)
{
    mbIgnoreReturnKeyEvent = val;
}

bool CRegexHistoryTextEdit::getIgnoreReturnKeyEvent() const
{
    return mbIgnoreReturnKeyEvent;
}

void CRegexHistoryTextEdit::activateRegexHistory()
{
    if(nullptr != mpRegexHistoryProvider)
    {
        mpRegexHistoryProvider->setSuggestionActive(true);
        int numberOfSuggestions = mpRegexHistoryProvider->updateSuggestions(toPlainText());

        if(0 == numberOfSuggestions)
        {
            mpRegexHistoryProvider->setSuggestionActive(false);
        }

        update();
    }
}

void CRegexHistoryTextEdit::keyPressEvent(QKeyEvent *event)
{
    if(nullptr != event)
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            emit returnPressed();
        }
        else if (event->key() == Qt::Key_Space && event->modifiers() == Qt::ControlModifier)
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
                QTextEdit::keyPressEvent(event);
            }
        }
        else
        {
            QString prevText = toPlainText();

            // Pass the event to the base class for normal processing
            QTextEdit::keyPressEvent(event);

            QString newText = toPlainText();

            if(prevText != newText)
            {
                if(nullptr != mpRegexHistoryProvider &&
                   true == mpRegexHistoryProvider->getSuggestionActive())
                {
                    static_cast<void>(mpRegexHistoryProvider->updateSuggestions(toPlainText()));
                    update();
                }
            }
        }
    }
}

void CRegexHistoryTextEdit::insertFromMimeData(const QMimeData *source)
{
    // Check if the source has plain text
    if (source->hasText())
    {
        // Get the pasted text and remove all newlines
        QString text = source->text();
        text.replace("\n", "").replace("\r", "");  // Remove all newline characters

        // Insert the modified text without newlines
        insertPlainText(text);

        adjustHeightToContent();
    }
    else
    {
        // If there's no plain text, use the default implementation
        QTextEdit::insertFromMimeData(source);
    }
}

void CRegexHistoryTextEdit::setCompleter(QCompleter* pCompleter)
{
    if (mpCompleter)
    {
        mpCompleter->disconnect(this);
    }

    mpCompleter = pCompleter;

    if (!pCompleter)
        return;

    pCompleter->setWidget(this);
    pCompleter->setCompletionMode(QCompleter::PopupCompletion);
    pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(pCompleter, QOverload<const QString &>::of(&QCompleter::activated),
                     this, &CRegexHistoryTextEdit::insertCompletion);
}

QCompleter* CRegexHistoryTextEdit::completer() const
{
    return mpCompleter;
}

void CRegexHistoryTextEdit::insertCompletion(const QString &completion)
{
    if (mpCompleter->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - mpCompleter->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CRegexHistoryTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CRegexHistoryTextEdit::setSelection(int start, int length)
{
    auto cursor_instance = textCursor();
    int end = start + length;
    cursor_instance.setPosition(start);
    cursor_instance.setPosition(end, QTextCursor::KeepAnchor);
    setTextCursor(cursor_instance);
}

PUML_PACKAGE_BEGIN(DMA_RegexHistory_API)
    PUML_CLASS_BEGIN(CRegexHistoryTextEdit)
        PUML_INHERITANCE_CHECKED(QTextEdit, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(IRegexHistoryProvider, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QCompleter, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
