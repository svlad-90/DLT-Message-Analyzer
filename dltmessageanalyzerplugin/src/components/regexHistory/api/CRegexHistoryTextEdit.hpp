#pragma once

#include "QTextEdit"
#include "QKeyEvent"
#include "QWidget"

#include "components/settings/api/CSettingsManagerClient.hpp"

class QCompleter;

#include "components/regexHistory/api/IRegexHistoryProvider.hpp"

/**
 * @brief A custom QTextEdit with regex history support.
 *
 * This class extends QTextEdit to include features for managing regex history
 * and handling specific key events.
 */
class CRegexHistoryTextEdit : public QTextEdit,
                              public CSettingsManagerClient
{
    Q_OBJECT

public:

    /**
     * @brief Constructs a CRegexHistoryTextEdit object.
     *
     * @param parent The parent widget for the text edit. Defaults to nullptr.
     */
    explicit CRegexHistoryTextEdit(QWidget *parent = nullptr);

    /**
     * @brief Sets the regex history provider.
     *
     * This method associates a regex history provider.
     *
     * @param pRegexHistoryProviderPtr Shared pointer to the regex history provider.
     */
    void setRegexHistoryProvider(const tRegexHistoryProviderPtr& pRegexHistoryProviderPtr);

    /**
     * @brief Sets whether to ignore return key events.
     *
     * This method allows control over whether the line edit should handle
     * return (Enter) key events or ignore them.
     *
     * @param val True to ignore return key events, false otherwise.
     */
    void setIgnoreReturnKeyEvent(bool val);

    /**
     * @brief Checks whether return key events are ignored.
     *
     * This method returns whether the line edit is currently set to ignore
     * return key events.
     *
     * @return bool True if return key events are ignored, false otherwise.
     */
    bool getIgnoreReturnKeyEvent() const;

    /**
     * @brief Activates regex history.
     *
     * This method activates the regex history.
     */
    void activateRegexHistory();

    /**
     * @brief Sets the completer for the text edit.
     *
     * This method allows setting a QCompleter to provide text completions.
     *
     * @param pCompleter Pointer to the QCompleter instance.
     */
    void setCompleter(QCompleter* pCompleter);

    /**
     * @brief Gets the current completer.
     *
     * This method returns the current QCompleter instance used by the text edit.
     *
     * @return QCompleter* Pointer to the current QCompleter.
     */
    QCompleter *completer() const;

    /**
     * @brief Sets the selection range within the text.
     *
     * This method allows setting a selection starting at a given position with
     * a specified length.
     *
     * @param start The starting position of the selection.
     * @param length The length of the selection.
     */
    void setSelection(int start, int length);

signals:
    /**
     * @brief Signal emitted when the return key is pressed.
     */
    void returnPressed();

private slots:
    void insertCompletion(const QString &completion);

protected:
    void insertFromMimeData(const QMimeData *source) override;
    void resizeEvent(QResizeEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void handleSettingsManagerChange() override;

private:
    QString textUnderCursor() const;
    void adjustHeightToContent();

private:
    tRegexHistoryProviderPtr mpRegexHistoryProvider;
    bool mbIgnoreReturnKeyEvent;
    QCompleter* mpCompleter;
};
