#include "../api/Types.hpp"

#include "DMA_Plantuml.hpp"

#define FROM_JSON(JSON_OBJECT, JSON_ATTR_NAME, ASSIGN_TO_FIELD)                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        auto foundElement = JSON_OBJECT.find(JSON_ATTR_NAME);                                                          \
        if (foundElement != JSON_OBJECT.end() && false == foundElement->empty())                                       \
            foundElement->get_to(ASSIGN_TO_FIELD);                                                                     \
    } while (false)

#define FROM_JSON_WITH_DEFAULT_VALUE(JSON_OBJECT, JSON_ATTR_NAME, ASSIGN_TO_FIELD, DEFAULT_VALUE)                      \
    do                                                                                                                 \
    {                                                                                                                  \
        auto foundElement = JSON_OBJECT.find(JSON_ATTR_NAME);                                                          \
        if (foundElement != JSON_OBJECT.end() && false == foundElement->empty())                                       \
            ASSIGN_TO_FIELD = foundElement->get<decltype(ASSIGN_TO_FIELD)>();                                          \
        else                                                                                                           \
            ASSIGN_TO_FIELD = DEFAULT_VALUE;                                                                           \
    } while (false)

#define TO_JSON(JSON_OBJECT, JSON_ATTR_NAME, ASSIGN_FROM_FIELD) JSON_OBJECT[JSON_ATTR_NAME] = ASSIGN_FROM_FIELD

namespace
{
    const char* KEY_MESSAGE                = "message";
    const char* KEY_USERNAME               = "userName";
    const char* KEY_DATE_TIME              = "dateTime";
    const char* KEY_MOTE_COMMENT           = "comment";
    const char* KEY_REGEX                  = "regex";
    const char* KEY_COVERAGE_NOTE_ITEM_VEC = "coverageNoteItemVec";
}

static inline void to_json(nlohmann::json & j, const tQStringPtrWrapper& value)
{
    if(value.pString)
    {
        j = value.pString->toStdString();
    }
    else
    {
        j = "";
    }
}

static inline void from_json(const nlohmann::json & j, tQStringPtrWrapper& value)
{
    if (j.is_string())
    {
        std::string str;
        j.get_to(str);  // Get std::string from JSON
        if(!value.pString)
        {
            value.pString = std::make_shared<QString>();
        }
        *value.pString = QString::fromStdString(str);
    }
}

static inline void to_json(nlohmann::json & j, const QDateTime& value)
{
    j = value.toString().toStdString();
}

static inline void from_json(const nlohmann::json & j, QDateTime& value)
{
    if (j.is_string())
    {
        QString str;
        value = QDateTime::fromString(QString::fromStdString(j.get<std::string>()));
    }
}

static inline void to_json(nlohmann::json & j, const tCoverageNoteItem& value)
{
    TO_JSON(j, KEY_MESSAGE, value.message);
    TO_JSON(j, KEY_USERNAME, value.userName);
    TO_JSON(j, KEY_DATE_TIME, value.dateTime);
    TO_JSON(j, KEY_MOTE_COMMENT, value.comment);
    TO_JSON(j, KEY_REGEX, value.regex);
}

static inline void from_json(const nlohmann::json & j, tCoverageNoteItem& value)
{
    value.message  = j.at(KEY_MESSAGE);
    value.userName = j.at(KEY_USERNAME);
    value.dateTime = j.at(KEY_DATE_TIME);
    value.comment = j.at(KEY_MOTE_COMMENT);
    value.regex = j.at(KEY_REGEX);
}

static inline void to_json(nlohmann::json & j, const tCoverageNoteItemPtr& value)
{
    if(value)
    {
        to_json(j, *value);
    }
    else
    {
        j = "";
    }
}

static inline void from_json(const nlohmann::json & j, tCoverageNoteItemPtr& value)
{
    if(j.is_object())
    {
        if(!value)
        {
            value = std::make_shared<tCoverageNoteItem>();
        }
        from_json(j, *value);
    }
}

void to_json(nlohmann::json & j, const tCoverageNote& value)
{
    TO_JSON(j, KEY_COVERAGE_NOTE_ITEM_VEC, value.coverageNoteItemVec);
}

void from_json(const nlohmann::json & j, tCoverageNote& value)
{
    value.coverageNoteItemVec  = j.at(KEY_COVERAGE_NOTE_ITEM_VEC);
}

bool tCoverageNote::parseCoverageNote(const nlohmann::json& coverageNote)
{
    bool bResult = true;

    try
    {
        coverageNoteItemVec = (coverageNote).template get<tCoverageNoteItemVec>();
    }
    catch (const std::exception& e)
    {
        bResult = false;
    }

    return bResult;
}

nlohmann::json tCoverageNote::serializeCoverageNote()
{
    nlohmann::json doc = coverageNoteItemVec;
    return doc;
}

tCoverageNoteItemId tCoverageNote::addCoverageNoteItem()
{
    coverageNoteItemVec.push_back(std::make_shared<tCoverageNoteItem>());
    return coverageNoteItemVec.size() - 1;
}

void tCoverageNote::clear()
{
    coverageNoteItemVec.clear();
    resetModified();
}

void tCoverageNote::removeCoverageNoteItem(const tCoverageNoteItemId&id)
{
    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        coverageNoteItemVec.erase(coverageNoteItemVec.begin() + id);
        setModified(true);
    }
}

std::size_t tCoverageNote::size() const
{
    return coverageNoteItemVec.size();
}

bool tCoverageNote::empty() const
{
    return coverageNoteItemVec.empty();
}

void tCoverageNote::setMessage(const tCoverageNoteItemId& id, const QString& val)
{
    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        if(coverageNoteItemVec[id]->message.pString && *coverageNoteItemVec[id]->message.pString != val)
        {
            coverageNoteItemVec[id]->message.pString = std::make_shared<QString>(val);
            setModified(true);
        }
    }
}

tQStringPtr tCoverageNote::getMessage(const tCoverageNoteItemId& id) const
{
    tQStringPtr pResult = nullptr;

    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        pResult = coverageNoteItemVec[id]->message.pString;
    }

    return pResult;
}

void tCoverageNote::setUsername(const tCoverageNoteItemId& id, const QString& val)
{
    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        if(coverageNoteItemVec[id]->userName.pString && *coverageNoteItemVec[id]->userName.pString != val)
        {
            coverageNoteItemVec[id]->userName.pString = std::make_shared<QString>(val);
            setModified(true);
        }
    }
}

tQStringPtr tCoverageNote::getUsername(const tCoverageNoteItemId& id) const
{
    tQStringPtr pResult = nullptr;

    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        pResult = coverageNoteItemVec[id]->userName.pString;
    }

    return pResult;
}

void tCoverageNote::setDateTime(const tCoverageNoteItemId& id, const QDateTime& val)
{
    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        if(coverageNoteItemVec[id]->dateTime != val)
        {
            coverageNoteItemVec[id]->dateTime = val;
            setModified(true);
        }
    }
}

const QDateTime& tCoverageNote::getDateTime(const tCoverageNoteItemId& id) const
{
    const static QDateTime sEmptyDateTime;

    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        return coverageNoteItemVec[id]->dateTime;
    }

    return sEmptyDateTime;
}

void tCoverageNote::setComment(const tCoverageNoteItemId& id, const QString& val)
{
    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        if(coverageNoteItemVec[id]->comment.pString && *coverageNoteItemVec[id]->comment.pString != val)
        {
            coverageNoteItemVec[id]->comment.pString = std::make_shared<QString>(val);
            setModified(true);
        }
    }
}

tQStringPtr tCoverageNote::getComment(const tCoverageNoteItemId& id) const
{
    tQStringPtr pResult = nullptr;

    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        pResult = coverageNoteItemVec[id]->comment.pString;
    }

    return pResult;
}

void tCoverageNote::setRegex(const tCoverageNoteItemId& id, const QString& val)
{
    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        if(coverageNoteItemVec[id]->regex.pString && *coverageNoteItemVec[id]->regex.pString != val)
        {
            coverageNoteItemVec[id]->regex.pString = std::make_shared<QString>(val);
            setModified(true);
        }
    }
}

tQStringPtr tCoverageNote::getRegex(const tCoverageNoteItemId& id) const
{
    tQStringPtr pResult = nullptr;

    if(id >= 0 && id < static_cast<int>(coverageNoteItemVec.size()))
    {
        pResult = coverageNoteItemVec[id]->regex.pString;
    }

    return pResult;
}

void tCoverageNote::swap(const std::size_t& from, const std::size_t& to)
{
    std::swap(coverageNoteItemVec[from],
              coverageNoteItemVec[to]);
}

const tCoverageNoteItemVec& tCoverageNote::getCoverageNoteItemVec()
{
    return coverageNoteItemVec;
}

void tCoverageNote::resetModified()
{
    mbModified = false;
}

bool tCoverageNote::isModified() const
{
    return mbModified;
}

void tCoverageNote::setModified(bool val)
{
    mbModified = val;
}

tCoverageNoteItem::tCoverageNoteItem():
message(std::make_shared<QString>()),
userName(std::make_shared<QString>()),
dateTime(),
comment(std::make_shared<QString>()),
regex(std::make_shared<QString>())
{}

PUML_PACKAGE_BEGIN(DMA_CoverageNote_API)
    PUML_CLASS_BEGIN_CHECKED(tCoverageNoteItem)
        PUML_MEMBER(+, tQStringPtrWrapper message)
        PUML_MEMBER(+, tQStringPtrWrapper userName)
        PUML_MEMBER(+, QDateTime dateTime)
        PUML_MEMBER(+, tQStringPtrWrapper comment)
        PUML_MEMBER(+, tQStringPtrWrapper regex)
        PUML_USE_DEPENDENCY_CHECKED(nlohmann::json, 1, 1, uses)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN_CHECKED(tCoverageNote)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(tCoverageNoteItem, 1, *, contains)
        PUML_USE_DEPENDENCY_CHECKED(nlohmann::json, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
