#include <map>
#include <memory>

#include "QString"
#include "QDateTime"
#include "QRegularExpression"

#include "common/Definitions.hpp"
#include "nlohmann/json.hpp"


typedef int tCoverageNoteItemId;
typedef int tCoverageNoteCommentId;

struct tCoverageNoteItem
{
    tQStringPtrWrapper message;
    tQStringPtrWrapper userName;
    QDateTime time;
    tQStringPtrWrapper noteMessage;
    tQStringPtrWrapper regex;
};

typedef std::shared_ptr<tCoverageNoteItem> tCoverageNoteItemPtr;
typedef std::vector<tCoverageNoteItemPtr> tCoverageNoteItemVec;

struct tCoverageNote
{
    tCoverageNoteItemVec coverageNoteItemVec;
    bool parseCoverageNote(const nlohmann::json& coverageNote);
    nlohmann::json serializeCoverageNote();
};
