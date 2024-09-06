#pragma once

#include "memory"

#include "QObject"
#include "QString"

/**
 * @brief Interface class for providing regex history suggestions.
 *
 * This interface defines methods for managing and retrieving suggestions
 * based on a regex input history. It can be used to update, clear, activate,
 * or deactivate regex suggestions.
 */
class IRegexHistoryProvider : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief Constructor for IRegexHistoryProvider.
     */
    IRegexHistoryProvider();

    /**
     * @brief Virtual destructor for IRegexHistoryProvider.
     */
    virtual ~IRegexHistoryProvider();

    /**
     * @brief Updates the suggestions based on the given input.
     *
     * This method will update the regex suggestions using the provided
     * input string.
     *
     * @param input The input string for updating suggestions.
     * @return int The number of proposed suggestions.
     */
    virtual int updateSuggestions(const QString& input) = 0;

    /**
     * @brief Clears the current suggestions.
     *
     * This method removes all proposed regex suggestions.
     */
    virtual void clearSuggestions() = 0;

    /**
     * @brief Checks whether suggestion mode is active.
     *
     * This method returns whether the suggestion feature is currently active.
     *
     * @return bool True if suggestion mode is active, false otherwise.
     */
    virtual bool getSuggestionActive() = 0;

    /**
     * @brief Sets the suggestion mode as active or inactive.
     *
     * This method enables or disables the suggestion feature.
     *
     * @param val True to activate suggestion mode, false to deactivate.
     */
    virtual void setSuggestionActive(bool val) = 0;

};

/**
 * @brief Shared pointer to an IRegexHistoryProvider instance.
 *
 * This typedef simplifies the usage of shared pointers for IRegexHistoryProvider.
 */
typedef std::shared_ptr<IRegexHistoryProvider> tRegexHistoryProviderPtr;
