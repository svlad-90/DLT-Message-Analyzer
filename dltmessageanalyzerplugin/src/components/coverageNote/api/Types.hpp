#include <map>
#include <memory>

#include "QString"
#include "QDateTime"
#include "QRegularExpression"

#include "common/Definitions.hpp"
#include "nlohmann/json.hpp"


typedef int tCoverageNoteItemId;

struct tCoverageNoteItem
{
    tCoverageNoteItem();
    tQStringPtrWrapper message;
    tQStringPtrWrapper userName;
    QDateTime dateTime;
    tQStringPtrWrapper comment;
    tQStringPtrWrapper regex;
};

typedef std::shared_ptr<tCoverageNoteItem> tCoverageNoteItemPtr;
typedef std::vector<tCoverageNoteItemPtr> tCoverageNoteItemVec;

struct tCoverageNote
{
public:
    /**
     * @brief Parses a coverage note from a JSON object.
     * @param coverageNote JSON object representing the coverage note.
     * @return true if parsing was successful, false otherwise.
     */
    bool parseCoverageNote(const nlohmann::json& coverageNote);

    /**
     * @brief Serializes the coverage note to a JSON object.
     * @return A JSON object representing the coverage note.
     */
    nlohmann::json serializeCoverageNote();

    /**
     * @brief Adds a new coverage note item.
     * @return The ID of the newly added coverage note item.
     */
    tCoverageNoteItemId addCoverageNoteItem();

    /**
     * @brief Clears all coverage note items.
     */
    void clear();

    /**
     * @brief Removes a coverage note item by its ID.
     * @param id The ID of the coverage note item to be removed.
     */
    void removeCoverageNoteItem(const tCoverageNoteItemId& id);

    /**
     * @brief Gets the number of coverage note items.
     * @return The number of coverage note items.
     */
    std::size_t size() const;

    /**
     * @brief Checks if there are no coverage note items.
     * @return true if there are no items, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Sets the message for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @param val The message to set.
     */
    void setMessage(const tCoverageNoteItemId& id, const QString& val);

    /**
     * @brief Gets the message for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @return A pointer to the message string.
     */
    tQStringPtr getMessage(const tCoverageNoteItemId& id) const;

    /**
     * @brief Sets the username for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @param val The username to set.
     */
    void setUsername(const tCoverageNoteItemId& id, const QString& val);

    /**
     * @brief Gets the username for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @return A pointer to the username string.
     */
    tQStringPtr getUsername(const tCoverageNoteItemId& id) const;

    /**
     * @brief Sets the date and time for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @param val The date and time to set.
     */
    void setDateTime(const tCoverageNoteItemId& id, const QDateTime& val);

    /**
     * @brief Gets the date and time for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @return The date and time of the coverage note item.
     */
    const QDateTime& getDateTime(const tCoverageNoteItemId& id) const;

    /**
     * @brief Sets the comment for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @param val The comment to set.
     */
    void setComment(const tCoverageNoteItemId& id, const QString& val);

    /**
     * @brief Gets the comment for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @return A pointer to the comment string.
     */
    tQStringPtr getComment(const tCoverageNoteItemId& id) const;

    /**
     * @brief Sets the regex pattern for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @param val The regex pattern to set.
     */
    void setRegex(const tCoverageNoteItemId& id, const QString& val);

    /**
     * @brief Gets the regex pattern for a coverage note item by ID.
     * @param id The ID of the coverage note item.
     * @return A pointer to the regex pattern string.
     */
    tQStringPtr getRegex(const tCoverageNoteItemId& id) const;

    /**
     * @brief Swaps the positions of two coverage note items.
     * @param from The index of the first item.
     * @param to The index of the second item.
     */
    void swap(const std::size_t& from, const std::size_t& to);

    /**
     * @brief Gets the vector of coverage note items.
     * @return A reference to the vector of coverage note items.
     */
    const tCoverageNoteItemVec& getCoverageNoteItemVec();

    /**
     * @brief Resets the modified state of the coverage note.
     */
    void resetModified();

    /**
     * @brief Checks if the coverage note has been modified.
     * @return true if the coverage note has been modified, false otherwise.
     */
    bool isModified() const;

private:
    void setModified(bool val);
    tCoverageNoteItemVec coverageNoteItemVec;
    bool mbModified = false;

    friend void to_json(nlohmann::json & j, const tCoverageNote& value);
    friend void from_json(const nlohmann::json & j, tCoverageNote& value);
};
