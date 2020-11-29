/**
 * @file    Definitions.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the Definitions.hpp class
 */
#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include "memory"
#include "set"

#include "QMap"
#include "QString"
#include "QVector"
#include "QMetaType"
#include "QColor"
#include "QPair"

#include "../common/variant/variant.hpp"

//#define DEBUG_BUILD

extern const std::map<QString, QColor> sColorsMap;

extern const QString sVARPrefix;
extern const QString sRegexScriptingDelimiter;

////////////////////// UML DEFINITIONS //////////////////////

enum class eUML_ID
{
    UML_SEQUENCE_ID = 0,
    UML_CLIENT ,
    UML_REQUEST,
    UML_RESPONSE,
    UML_EVENT,
    UML_SERVICE,
    UML_METHOD,
    UML_ARGUMENTS
};

QString getUMLIDAsString( const eUML_ID& val );
bool parseUMLIDFromString( const QString& data, eUML_ID& val );

enum class eUML_ID_Type
{
    e_Optional,
    e_Mandatory,
    e_RequestType
};

QString getUMLIDTypeAsString( const eUML_ID_Type& val );

struct tUML_ID_Item
{
    eUML_ID_Type id_type;
    QString id_str;
    QString description;
};

typedef std::map<eUML_ID, tUML_ID_Item> tUML_IDs_Map;
extern const tUML_IDs_Map s_UML_IDs_Map;

////////////////////// UML DEFINITIONS (END) //////////////////////

typedef int tWorkerThreadCookie;

typedef uint64_t tSettingsManagerVersion;

// base tree item
class CTreeItem;
typedef CTreeItem tTreeItem;
typedef tTreeItem* tTreeItemPtr;
typedef QVector<tTreeItemPtr> tTreeItemPtrVec;
typedef std::shared_ptr<CTreeItem> tTreeItemSharedPtr;

class CPatternsModel;
typedef CPatternsModel* tPatternsModelPtr;

extern const QString sDefaultStatusText;
extern const QString sDefaultRegexFileName;

typedef std::shared_ptr<QString> tQStringPtr;

struct tQStringPtrWrapper
{
    tQStringPtrWrapper();
    tQStringPtrWrapper(const tQStringPtr& pString_);
    bool operator== ( const tQStringPtrWrapper& rVal ) const;
    bool operator< ( const tQStringPtrWrapper& rVal ) const;
    tQStringPtr pString = nullptr;
};

Q_DECLARE_METATYPE(tQStringPtrWrapper)

typedef int tMsgId;
extern const tMsgId INVALID_MSG_ID;

typedef int tHighlightingRangeItem;

struct tHighlightingRange
{
    tHighlightingRange();
    tHighlightingRange( const tHighlightingRangeItem& from_, const tHighlightingRangeItem& to_, const QColor& color_, bool explicitColor_ );
    bool operator< ( const tHighlightingRange& rVal ) const;
    tHighlightingRangeItem from;
    tHighlightingRangeItem to;
    QColor color;
    bool explicitColor;
};
typedef std::vector<tHighlightingRange> tHighlightingRangeList;
typedef std::set<tHighlightingRange> tHighlightingRangeSet;

template <typename T>
struct tRange
{
    typedef T tRangeItem;

    tRange( const tRangeItem& from_, const tRangeItem& to_ ):
    from(from_), to(to_)
    {}

    tRange():
    from(0), to(0)
    {}

    bool operator< ( const tRange& rVal ) const
    {
        bool bResult = false;

        if( from < rVal.from )
        {
            bResult = true;
        }
        else if( from > rVal.from )
        {
            bResult = false;
        }
        else // if from == rVal.from
        {
            if( to < rVal.to )
            {
                bResult = true;
            }
            else
            {
                bResult = false;
            }
        }

        return bResult;
    }

    bool operator== ( const tRange& rVal ) const
    {
        return ( from == rVal.from && to == rVal.to );
    }

    tRangeItem from;
    tRangeItem to;
};

typedef tRange<int> tIntRange;

Q_DECLARE_METATYPE(tIntRange)

struct tIntRangePtrWrapper
{
    bool operator== ( const tIntRangePtrWrapper& rVal ) const;
    bool operator< ( const tIntRangePtrWrapper& rVal ) const;
    const tIntRange* pRange = nullptr;
};

Q_DECLARE_METATYPE(tIntRangePtrWrapper)

typedef QVector<tIntRange> tIntRangeList;
typedef std::set<tIntRange> tIntRangeSet;

/**
 * @brief The eSearchResultColumn enum
 * Enum, which is used to operate on the columns of the search result view in a readable way
 */
enum class eSearchResultColumn : int
{
    UML_Applicability = 0,
    Index,
    Time,
    Timestamp,
    Count,
    Ecuid,
    Apid,
    Ctid,
    SessionId,
    Type,
    Subtype,
    Mode,
    Args,
    Payload,
    Last
};

QString getName(eSearchResultColumn val);

typedef QMap<eSearchResultColumn, bool> tSearchResultColumnsVisibilityMap;

enum class ePatternsRowType
{
    ePatternsRowType_Alias = 0,
    ePatternsRowType_FakeTreeLevel
};

Q_DECLARE_METATYPE(ePatternsRowType)

/**
 * @brief The ePatternsColumn enum
 * Enum, which is used to operate on the columns of the patterns view in a readable way
 */
enum class ePatternsColumn : int
{
    AliasTreeLevel = 0, /*QString*/
    Default, /*Qt::CheckState*/
    Combine, /*Qt::CheckState*/
    Regex, /*QString*/
    AfterLastVisible, /*empty string*/
    Alias, /*QString*/
    RowType, /*ePatternsRowType*/
    IsFiltered, /*bool*/
    Last /*nothing*/
};

QString getName(ePatternsColumn val);
ePatternsColumn toPatternsColumn(int column);

typedef QMap<ePatternsColumn, bool> tPatternsColumnsVisibilityMap;

typedef unsigned int tTimeStamp;
struct tGroupedViewMetadata
{
    tGroupedViewMetadata();

    /**
     * @brief operator < - fake. No actually used as of now.
     * Create just to place this item into the nonstd::variant
     * @param rhs - item to compare against it
     * @return - true if this item is less than provided rhs. False otherwise.
     */
    bool operator< (const tGroupedViewMetadata& rhs) const;

    bool operator== (const tGroupedViewMetadata& rhs) const;

    tGroupedViewMetadata( const unsigned int timeStamp_, const tMsgId& msgId_ );
    tTimeStamp timeStamp;
    tMsgId msgId;
};
Q_DECLARE_METATYPE(tGroupedViewMetadata)

/**
 * @brief The ePatternsColumn enum
 * Enum, which is used to operate on the columns of the patterns view in a readable way
 */
enum class eGroupedViewColumn : int
{
    SubString = 0, /*QString*/
    Messages, /*int*/
    MessagesPercantage, /*int*/
    MessagesPerSecondAverage, /*int*/
    Payload, /*int*/
    PayloadPercantage, /*int*/
    PayloadPerSecondAverage, /*int*/
    AfterLastVisible, /*nothing*/
    Metadata, /*tGroupedViewMetadata*/
    Last
};

QString getName(eGroupedViewColumn field);
eGroupedViewColumn toGroupedViewColumn(int column);

typedef QMap<eGroupedViewColumn, bool> tGroupedViewColumnsVisibilityMap;

enum class eGroupSyntaxType
{
    SYNTAX_1 = 0,  /*?<name>*/
    SYNTAX_2 = 1,  /*?'name'*/
    SYNTAX_3 = 2   /*?P<name>*/
};

//////////////////////FILTERS VIEW DEFINITIONS//////////////////////

/**
 * @brief The eRegexFiltersColumn enum
 * Enum, which is used to operate on the columns of the patterns view in a readable way
 */
enum class eRegexFiltersColumn : int
{
    Value = 0, /*QString*/
    Index, /*int*/
    ItemType, /*QString*/
    AfterLastVisible, /*empty string*/
    Color, /*tColorWrapper*/
    Range, /*tIntRange*/
    RowType, /*eRegexFiltersRowType*/
    IsFiltered, /*bool*/
    GroupName, /*QString*/
    GroupSyntaxType, /*int ... enough of enums in our variant*/
    GroupIndex,      /*index of the group, if item is group. If item is text - filled in with -1*/
    Last /*nothing*/
};

QString getName(eRegexFiltersColumn val);
eRegexFiltersColumn toRegexFiltersColumn(int column);

typedef QMap<eRegexFiltersColumn, bool> tRegexFiltersColumnsVisibilityMap;

enum class eRegexFiltersRowType
{
    VarGroup = 0, // variable group
    NonVarGroup, // non-variable group
    Text // text
};

Q_DECLARE_METATYPE( eRegexFiltersRowType )

QString getName(eRegexFiltersRowType val);

//////////////////////FILTERS VIEW DEFINITIONS END//////////////////


struct tHighlightingGradient
{
    tHighlightingGradient();
    tHighlightingGradient(const QColor& from_, const QColor& to_, int numberOfColors_);
    bool operator==(const tHighlightingGradient&) const;
    bool operator!=(const tHighlightingGradient&) const;
    QColor from;
    QColor to;
    int numberOfColors;
};

// generates a set of colors from a gradient's definition
QVector<QColor> generateColors( const tHighlightingGradient& gradient );

typedef QMap<eSearchResultColumn, tHighlightingRangeSet> tHighlightingInfoMulticolor;
typedef QMap<eSearchResultColumn, tIntRange> tFieldRanges;

struct tFoundMatch
{
    tFoundMatch();
    tFoundMatch( const tQStringPtr& pMatchStr_,
                 const tIntRange& range_,
                 const int& idx_,
                 const unsigned int& msgSizeBytes_,
                 const unsigned int& timeStamp_,
                 const tMsgId& msgId_);

    /**
     * @brief operator < - fake. No actually used as of now.
     * Create just to place this item into the nonstd::variant
     * @param rhs - item to compare against it
     * @return - true if this item is less than provided rhs. False otherwise.
     */
    bool operator< (const tFoundMatch& rhs) const;

    tQStringPtr pMatchStr;
    tIntRange range;
    int idx;
    unsigned int msgSizeBytes;
    unsigned int timeStamp;
    tMsgId msgId;
};

Q_DECLARE_METATYPE( const tFoundMatch* )

struct QOptionalColor
{
    bool isSet;
    QColor color;
    bool operator== ( const QOptionalColor& rhs ) const;
};
typedef QVector<QOptionalColor> QOptionalColorVec;

struct tColorWrapper
{
    QOptionalColor optColor;
    bool operator< ( const tColorWrapper& rhs ) const;
    bool operator== ( const tColorWrapper& rhs ) const;
};

Q_DECLARE_METATYPE( tColorWrapper )

////////////////// TREE ITEM ////////////////
typedef nonstd::variant<QString,
                        tQStringPtrWrapper,
                        Qt::CheckState,
                        ePatternsRowType,
                        bool,
                        int,
                        double,
                        tGroupedViewMetadata,
                        tIntRangePtrWrapper,
                        const tFoundMatch*,
                        tColorWrapper,
                        tIntRange,
                        eRegexFiltersRowType> tTreeDataItem;
typedef tTreeDataItem tDataItem; // just to refactor less code
QVariant toQVariant(const tDataItem& item);
tDataItem toRegexDataItem(const QVariant& variant, const eRegexFiltersColumn& column);
/////////////////////////////////////////////

typedef std::vector<tFoundMatch> tFoundMatches;

typedef QVector<QColor> QColorVec;

typedef QString tVarName;
typedef QPair<bool, tVarName> tOptionalVarName;

struct tOptional_UML_ID_Item
{
    // optional string, which can be assigned by the user in form of e.g. <US_myService>.
    // In above case this variable will be filled in with "myService" value
    QString UML_Custom_Value;
};

typedef std::map<eUML_ID, tOptional_UML_ID_Item> tOptional_UML_IDMap;

struct tOptional_UML_ID
{
    tOptional_UML_IDMap optional_UML_IDMap;
};

///////////////////////////REGEX_SCRIPTING_METADATA/////////////////////////////////

/**
 * @brief The tRegexScriptingMetadataItem struct - represents a scripting metadata item.
 * Item represents metadata of one regex group, which was parsed out of regex group name
 */
struct tRegexScriptingMetadataItem
{
public:
    // highlighting
    QOptionalColor highlightingColor;

    // variable
    tOptionalVarName varName;

    // UML_ID data
    tOptional_UML_ID optionalUML_ID;
};

typedef std::shared_ptr<tRegexScriptingMetadataItem> tRegexScriptingMetadataItemPtr;
typedef QVector<tRegexScriptingMetadataItemPtr> tRegexScriptingMetadataItemPtrVec;

Q_DECLARE_METATYPE( tRegexScriptingMetadataItemPtr )

/**
 * @brief The tRegexScriptingMetadata struct
 * Structure, which represents the scripting metadata, which is usually parsed from the regex names.
 * Has representation of items in form of a tree - which is manually calculated out of QRegularExpression Qt API.
 * Also has representation in form of the vector, which is a direct overtake from the QRegularExpression Qt API.
 * Items in vector are ordered in the same way, in which Qt API is ordering the found groupes.
 */
struct tRegexScriptingMetadata
{
    bool parse(const QRegularExpression& regex, bool bParseUMLData);
    const tRegexScriptingMetadataItemPtrVec& getItemsVec() const;
    typedef std::set<int> tCheckIDs;
    std::pair<bool /*status*/, QString /*status description*/> doesContainConsistentUMLData(bool fillInStringMsg) const;
    std::pair<bool /*status*/, QString /*status description*/> doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs) const;
    bool doesContainAnyUMLGroup() const;

private:
    std::pair<bool /*status*/, QString /*status description*/> doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool bCheckAll) const;

private:
    tRegexScriptingMetadataItemPtrVec mItemsVec;
};
Q_DECLARE_METATYPE(tRegexScriptingMetadata)

tRegexScriptingMetadataItemPtr parseRegexGroupName( const QString& groupName, bool bParseUMLData );

////////////////////////////////////////////////////////////

struct tCalcRangesCoverageMulticolorResult
{
    tHighlightingRangeSet highlightingRangeSet;
};

typedef std::map<int /*group id*/, int /*gradient color id*/> tGroupIdToColorMap;

/**
 * @brief calcRangesCoverageMulticolor - determines in which way set of ranges coveres the inputRange.
 * @param pMatchesTree - tree of the matches
 * @param inputRange - input range, which we try to analyze.
 * @param regexScriptingMetadata - regex scripting metadata, which contains colors, which were scripted by the user.
 * @param gradientColors - gradiant colors, which are assigned to ranges in case if there are no corresponding scripted colors. Scripted colors have higher priority.
 * @param groupIdToColorMap - group ids to colors mapping
 * @return - QPair, with:
 * first - updated prevColorCounter value
 * second - set of ranges, which shows in which way rangeList coveres the inputRange.
 * Result of returned ranges will be sorted.
 */
tCalcRangesCoverageMulticolorResult calcRangesCoverageMulticolor( const tTreeItemSharedPtr& pMatchesTree,
                                                    const tIntRange& inputRange,
                                                    const tRegexScriptingMetadata& regexScriptingMetadata,
                                                    const QVector<QColor>& gradientColors,
                                                    const tGroupIdToColorMap& groupIdToColorMap);

/**
 * @brief getMatchesTree - forms a tree from the found matches, which then can be used in other methods, like calcRangesCoverageMulticolor
 * @param foundMatches - collection of found matches
 * @return - tree of matches
 * Note! Until you operate with the tree the collection of foundMatches should stay alive and unchanged!!!
 * Otherwise you might face crashes. Such non-safe behavior is implemented to preserve the speed.
 */
tTreeItemSharedPtr getMatchesTree( const tFoundMatches& foundMatches );

struct tStringCoverageItem
{
    tIntRange range;
    /*whether we should add separator after the string*/
    bool bAddSeparator = false;
};

typedef std::map<eSearchResultColumn, tStringCoverageItem> tStringCoverageMap;

struct tUMLDataItem
{
    QString UML_Custom_Value;
    tStringCoverageMap stringCoverageMap;
};

typedef std::vector<tUMLDataItem> tUMLDataItemsVec;

typedef std::map<eUML_ID, tUMLDataItemsVec> tUMLDataMap;
struct tUMLInfo
{
    bool bUMLConstraintsFulfilled = false;
    bool bApplyForUMLCreation = false;
    tUMLDataMap UMLDataMap;
};

struct tItemMetadata
{
    tItemMetadata();
    tItemMetadata( const tMsgId& msgId_,
                   const tMsgId& msgIdFiltered_,
                   const tFieldRanges& fieldRanges_,
                   const int& strSize_,
                   const unsigned int& msgSize_,
                   const unsigned int& timeStamp_);
    tTreeItemSharedPtr updateHighlightingInfo( const tFoundMatches& foundMatches,
                                 const QVector<QColor>& gradientColors,
                                 const tRegexScriptingMetadata& regexScriptingMetadata,
                                 tTreeItemSharedPtr pTree = nullptr);
    tTreeItemSharedPtr updateUMLInfo(const tFoundMatches& foundMatches,
                       const tRegexScriptingMetadata& regexScriptingMetadata,
                       tTreeItemSharedPtr pTree = nullptr);
    tMsgId msgId;
    tMsgId msgIdFiltered;
    tHighlightingInfoMulticolor highlightingInfoMultiColor;
    tFieldRanges fieldRanges;
    int strSize;
    unsigned int msgSize;
    unsigned int timeStamp;
    tUMLInfo UMLInfo;
};

typedef QPair<tItemMetadata, tQStringPtr> tProcessingStringItem;
typedef QVector<tProcessingStringItem> tProcessingStrings;
Q_DECLARE_METATYPE(tProcessingStrings)

struct tFoundMatchesPackItem
{
    tFoundMatchesPackItem();
    tFoundMatchesPackItem( tItemMetadata&& itemMetadata, tFoundMatches&& foundMatches );

    const tItemMetadata& getItemMetadata() const;
    const tFoundMatches& getFoundMatches() const;
    tItemMetadata& getItemMetadataWriteable();
    tFoundMatches& getFoundMatchesWriteable();
private:
    tItemMetadata mItemMetadata;
    tFoundMatches mFoundMatches;
};
typedef std::vector<tFoundMatchesPackItem> tFoundMatchesPackItemVec;

struct tFoundMatchesPack
{
    tFoundMatchesPack();
    tFoundMatchesPack( const tFoundMatchesPackItemVec& matchedItemVec_ );
    tFoundMatchesPackItemVec matchedItemVec;
};

class QDltPlugin;
typedef QList<QDltPlugin*> tPluginPtrList;

typedef uint64_t tRequestId;
extern const tRequestId INVALID_REQUEST_ID;
typedef std::set<tRequestId> tRequestIdSet;

enum class eRequestState
{
    SUCCESSFUL = 0,
    ERROR_STATE,
    PROGRESS
};

QString getName(eRequestState field);

typedef int tWorkerId;

class CDLTFileWrapper;
typedef std::shared_ptr<CDLTFileWrapper> tDLTFileWrapperPtr;

class CDLTMsgWrapper;
typedef std::shared_ptr<CDLTMsgWrapper> tDLTMsgWrapperPtr;

typedef uint32_t tCacheSizeMB;
typedef uint64_t tCacheSizeB;

tCacheSizeB MBToB( const tCacheSizeMB& mb );
tCacheSizeMB BToMB( const tCacheSizeB& b );

struct tIntRangeProperty : public tIntRange
{
    bool isSet = false;
    bool isFiltered = false;
    int fromFiltered = 0;
    int toFiltered = 0;
};

enum class eSearchViewLastColumnWidthStrategy
{
    eReset = 0,             // reset width on each search ( default strategy )
    ePreserveUserWidth,     // save user-selected width
    eFitToContent,          // fit width to the content of the visible rows
    eLast                   // last enum value
};

QString getPayloadWidthAsString(const eSearchViewLastColumnWidthStrategy& val);

/**
 * @brief V_2_CS - tries to get check state out of variant
 * @param val - input variant
 * @return - retrieved check state value
 */
Qt::CheckState V_2_CS( const tDataItem& val );
Qt::CheckState V_2_CS( const QVariant& val );

QString rgb2hex(const QColor& color, bool with_head = true);

QString addRegexOptions( const QString& regex );
int getRegexOptionsCharSize();
QString getFormattedRegexError(const QRegularExpression& regex);
int getRegexErrorColumn(const QRegularExpression& regex);

#endif // DEFINITIONS_HPP
