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
    bool parseCoverageNote(const nlohmann::json& coverageNote);
    nlohmann::json serializeCoverageNote();

    tCoverageNoteItemId addCoverageNoteItem();
    void clear();
    void removeCoverageNoteItem(const tCoverageNoteItemId&id);
    std::size_t size() const;
    bool empty() const;
    void setMessage(const tCoverageNoteItemId& id, const QString& val);
    tQStringPtr getMessage(const tCoverageNoteItemId& id) const;
    void setUsername(const tCoverageNoteItemId& id, const QString& val);
    tQStringPtr getUsername(const tCoverageNoteItemId& id) const;
    void setDateTime(const tCoverageNoteItemId& id, const QDateTime& val);
    const QDateTime& getDateTime(const tCoverageNoteItemId& id) const;
    void setComment(const tCoverageNoteItemId& id, const QString& val);
    tQStringPtr getComment(const tCoverageNoteItemId& id) const;
    void setRegex(const tCoverageNoteItemId& id, const QString& val);
    tQStringPtr getRegex(const tCoverageNoteItemId& id) const;

    void resetModified();
    bool isModified() const;

private:
    void setModified(bool val);
    tCoverageNoteItemVec coverageNoteItemVec;
    bool mbModified = false;

    friend void to_json(nlohmann::json & j, const tCoverageNote& value);
    friend void from_json(const nlohmann::json & j, tCoverageNote& value);
};
