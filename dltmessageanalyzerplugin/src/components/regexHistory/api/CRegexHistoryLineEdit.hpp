#pragma once

#include "QLineEdit"
#include "QKeyEvent"
#include "QWidget"

#include "components/regexHistory/api/IRegexHistoryProvider.hpp"

/**
 * @brief A custom QLineEdit with regex history support.
 *
 * This class extends QLineEdit to include features for managing regex history
 * and handling specific key events.
 */
class CRegexHistoryLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    /**
     * @brief Constructs a CRegexHistoryLineEdit object.
     *
     * @param parent The parent widget for the line edit. Defaults to nullptr.
     */
    explicit CRegexHistoryLineEdit(QWidget *parent = nullptr);

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
     * This method activates the regex history feature.
     */
    void activateRegexHistory();

protected:

    virtual void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:

    tRegexHistoryProviderPtr mpRegexHistoryProvider;
    bool mbIgnoreReturnKeyEvent;
};
