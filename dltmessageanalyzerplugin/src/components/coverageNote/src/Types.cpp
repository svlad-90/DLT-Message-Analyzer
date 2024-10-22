#include "../api/Types.hpp"

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
    const char* KEY_TIME                   = "time";
    const char* KEY_MOTE_MESSAGE           = "noteMessage";
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
        value.fromString(QString::fromStdString(j.get<std::string>()));
    }
}

static inline void to_json(nlohmann::json & j, const tCoverageNoteItem& value)
{
    TO_JSON(j, KEY_MESSAGE, value.message);
    TO_JSON(j, KEY_USERNAME, value.userName);
    TO_JSON(j, KEY_TIME, value.time);
    TO_JSON(j, KEY_MOTE_MESSAGE, value.noteMessage);
    TO_JSON(j, KEY_REGEX, value.regex);
}

static inline void from_json(const nlohmann::json & j, tCoverageNoteItem& value)
{
    value.message  = j.at(KEY_MESSAGE);
    value.userName = j.at(KEY_USERNAME);
    value.time     = j.at(KEY_TIME);
    value.noteMessage = j.at(KEY_MOTE_MESSAGE);
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

static inline void to_json(nlohmann::json & j, const tCoverageNote& value)
{
    TO_JSON(j, KEY_COVERAGE_NOTE_ITEM_VEC, value.coverageNoteItemVec);
}

static inline void from_json(const nlohmann::json & j, tCoverageNote& value)
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
