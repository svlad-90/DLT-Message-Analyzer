/**
 * @file    Definitions.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the Definitions.hpp class
 */
#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include <list>
#include <set>

#include "QMap"
#include "QSet"
#include "QString"
#include "QVector"
#include "QMetaType"
#include "QColor"
#include "QPair"
#include "TOptional.hpp"

#include "../common/variant/variant.hpp"

#include "BaseDefinitions.hpp"
#include "PlotDefinitions.hpp"

//#define DEBUG_BUILD

const std::map<QString, QColor>& getColorsMap();
const std::list<QColor>& getColorsList();

extern const QString sVARPrefix;
extern const QString sRegexScriptingDelimiter;

////////////////////// UML DEFINITIONS //////////////////////

enum class eUML_ID
{
    UML_TIMESTAMP = 0,
    UML_SEQUENCE_ID,
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

class IPatternsModel;
typedef std::shared_ptr<IPatternsModel> tPatternsModelPtr;

extern const QString sDefaultStatusText;
extern const QString sDefaultRegexFileName;

typedef int tMsgId;
extern const tMsgId INVALID_MSG_ID;
typedef std::set<tMsgId> tMsgIdSet;

typedef std::int32_t tHighlightingRangeItem;

struct tHighlightingRange
{
    tHighlightingRange();
    tHighlightingRange( const tHighlightingRangeItem& from_, const tHighlightingRangeItem& to_, const QColor& color_, bool explicitColor_ );
    bool operator< ( const tHighlightingRange& rVal ) const;
    tHighlightingRangeItem from;
    tHighlightingRangeItem to;
    QRgb color_code;
    bool explicitColor;
};
typedef std::vector<tHighlightingRange> tHighlightingRangeVec;
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

typedef tRange<int32_t> tIntRange;
Q_DECLARE_METATYPE(tIntRange)

typedef QVector<tIntRange> tIntRangeList;
typedef std::set<tIntRange> tIntRangeSet;

/**
 * @brief The eSearchResultColumn enum
 * Enum, which is used to operate on the columns of the search result view in a readable way
 */
enum class eSearchResultColumn : int
{
    UML_Applicability = 0,
    PlotView_Applicability,
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
    nonstd::variant<int, tMsgIdSet> relatedMsgIds = 0;
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

typedef QMap<eSearchResultColumn, tHighlightingRangeVec> tHighlightingInfoMulticolor;
typedef QMap<eSearchResultColumn, tIntRange> tFieldRanges;

struct tFoundMatch
{
    tFoundMatch();
    tFoundMatch( const QString& matchStr_,
                 const tIntRange& range_,
                 const int& idx_ );

    /**
     * @brief operator < - fake. No actually used as of now.
     * Create just to place this item into the nonstd::variant
     * @param rhs - item to compare against it
     * @return - true if this item is less than provided rhs. False otherwise.
     */
    bool operator< (const tFoundMatch& rhs) const;

    QString matchStr;
    tIntRange range;
    int idx;
};

Q_DECLARE_METATYPE( const tFoundMatch* )

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
                        tIntRange,
                        const tFoundMatch*,
                        tColorWrapper,
                        eRegexFiltersRowType> tTreeDataItem;
typedef tTreeDataItem tDataItem; // just to refactor less code
QVariant toQVariant(const tDataItem& item);
tDataItem toRegexDataItem(const QVariant& variant, const eRegexFiltersColumn& column);
/////////////////////////////////////////////

typedef std::vector<tFoundMatch> tFoundMatchesVec;

struct tFoundMatches
{
    tFoundMatches();
    tFoundMatches(const std::uint32_t& msgSizeBytes_,
                  const unsigned int& timeStamp_,
                  const tMsgId& msgId_);
    tFoundMatchesVec foundMatchesVec;
    unsigned int timeStamp;
    tMsgId msgId;
    std::uint32_t msgSizeBytes;
};

typedef QVector<QColor> QColorVec;

typedef QString tVarName;
typedef QPair<bool, tVarName> tOptionalVarName;

struct tOptional_UML_ID_Item
{
    // optional string, which can be assigned by the user in form of e.g. <US_myService>.
    // In above case this variable will be filled in with "myService" value
    tQStringPtr pUML_Custom_Value;
};

typedef std::map<eUML_ID, tOptional_UML_ID_Item> tOptional_UML_IDMap;

struct tOptional_UML_ID
{
    tOptional_UML_IDMap optional_UML_IDMap;
};

///////////////////////////GROUPED VIEW DATA////////////////////////////////////////
typedef int tGroupedViewIdx;
typedef std::map<int /*regex group index*/, tGroupedViewIdx /*specified grouped view ordering*/> tGroupedViewIndices;

///////////////////////////GROUPED VIEW DATA (END)//////////////////////////////////

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

    // PlotView data
    tPlotViewIDParameters plotViewIDParameters;
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
    typedef std::pair<bool /*status*/, QString /*status description*/> tStatusPair;
    bool parse(const QRegularExpression& regex,
               bool bParseUMLData,
               bool bParsePlotViewData,
               bool bParseGroupedViewData);
    const tRegexScriptingMetadataItemPtrVec& getItemsVec() const;
    typedef std::set<int> tCheckIDs;
    tStatusPair doesContainConsistentUMLData(bool fillInStringMsg) const;
    tStatusPair doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs) const;
    bool doesContainAnyUMLGroup() const;
    tStatusPair doesContainConsistentPlotViewData(bool fillInStringMsg, bool checkParameters) const;
    tStatusPair doesContainConsistentPlotViewData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool checkParameters) const;
    bool doesContainAnyPlotViewGroup() const;
    const tGroupedViewIndices& getGroupedViewIndices() const;

private:
    tStatusPair doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool bCheckAll) const;
    tStatusPair doesContainConsistentPlotViewData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool bCheckAll, bool checkParameters) const;
    std::set<ePlotViewAxisType> getUniqueAvailableAxisTypes() const;
    typedef TOptional<tStatusPair> tOptionalStatusPair;
    typedef TOptional<bool> tOptionalBool;

private:
    tRegexScriptingMetadataItemPtrVec mItemsVec;
    tGroupedViewIndices mGroupedViewIndexes;
};
Q_DECLARE_METATYPE(tRegexScriptingMetadata)

tRegexScriptingMetadataItemPtr parseRegexGroupName( const QString& groupName,
                                                    bool bParseUMLData,
                                                    bool bParsePlotViewData,
                                                    bool bParseGroupedViewData );

tGroupedViewIdx parseRegexGroupedViewIndices( const QString& groupName );

////////////////////////////////////////////////////////////

struct tCalcRangesCoverageMulticolorResult
{
    tHighlightingRangeVec highlightingRangeVec;
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

// UML data, parsed from each processed string

struct tUMLDataItem
{
    tQStringPtr pUML_Custom_Value;
    tStringCoverageMap stringCoverageMap;
};

typedef std::vector<tUMLDataItem> tUMLDataItemsVec;

typedef std::map<eUML_ID, tUMLDataItemsVec> tUMLDataMap;
struct tUMLInfo
{
    tUMLDataMap UMLDataMap;
    bool bUMLConstraintsFulfilled = false;
    bool bApplyForUMLCreation = false;
    bool bContains_Req_Resp_Ev = false;
};

// Plot view data, that is parsed from each processed string

struct tPlotViewDataItem
{
    QOptionalColor optColor;
    tQStringPtr pPlotViewGroupName;
    tQStringPtrVec plotViewSplitParameters;
    tStringCoverageMap stringCoverageMap;
};

typedef std::vector<tPlotViewDataItem> tPlotViewDataItemVec;

typedef std::map<ePlotViewID, tPlotViewDataItemVec> tPlotViewDataMap;
struct tPlotViewInfo
{
    tPlotViewDataMap plotViewDataMap;
    bool bPlotViewConstraintsFulfilled = false;
    bool bApplyForPlotCreation = false;
};

struct tItemMetadata
{
    tItemMetadata();
    tItemMetadata(const tItemMetadata& rhs);
    tItemMetadata& operator= (const tItemMetadata& rhs);
    tItemMetadata( const tMsgId& msgId_,
                   const tMsgId& msgIdxInMainTable_,
                   const tFieldRanges& fieldRanges_,
                   const int& strSize_,
                   const std::uint32_t& msgSize_,
                   const unsigned int& timeStamp_);
    tTreeItemSharedPtr updateHighlightingInfo( const tFoundMatches& foundMatches,
                                 const QVector<QColor>& gradientColors,
                                 const tRegexScriptingMetadata& regexScriptingMetadata,
                                 tTreeItemSharedPtr pTree = nullptr);

    struct tUpdateUMLInfoResult
    {
        tTreeItemSharedPtr pTreeItem;
        bool bUML_Req_Res_Ev_DuplicateFound = false;
    };

    tUpdateUMLInfoResult updateUMLInfo(const tFoundMatches& foundMatches,
                       const tRegexScriptingMetadata& regexScriptingMetadata,
                       tTreeItemSharedPtr pTree = nullptr);

    struct tUpdatePlotViewInfoResult
    {
        tTreeItemSharedPtr pTreeItem;
    };

    tUpdatePlotViewInfoResult updatePlotViewInfo(const tFoundMatches& foundMatches,
                                       const tRegexScriptingMetadata& regexScriptingMetadata,
                                       tTreeItemSharedPtr pTree = nullptr);

    tHighlightingInfoMulticolor highlightingInfoMultiColor;
    tFieldRanges fieldRanges;
    std::unique_ptr<tUMLInfo> pUMLInfo = nullptr;
    std::unique_ptr<tPlotViewInfo> pPlotViewInfo = nullptr;
    tMsgId msgId;
    tMsgId msgIdxInMainTable;
    int strSize;
    unsigned int timeStamp;
    std::uint32_t msgSize;
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

typedef std::shared_ptr<tFoundMatchesPackItem> tFoundMatchesPackItemPtr;
typedef std::vector<tFoundMatchesPackItemPtr> tFoundMatchesPackItemVec;

struct tFoundMatchesPack
{
    tFoundMatchesPack();
    tFoundMatchesPack( const tFoundMatchesPackItemVec& matchedItemVec_ );
    int findRowByMsgId(const tMsgId& msgIdToFind) const;
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

class IFileWrapper;
typedef std::shared_ptr<IFileWrapper> tFileWrapperPtr;

class IMsgWrapper;
typedef std::shared_ptr<IMsgWrapper> tMsgWrapperPtr;

class IMsgDecoder;
typedef std::shared_ptr<IMsgDecoder> tMsgDecoderPtr;

class IDLTLogsWrapperCreator;

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

enum class ePathMode
{
    eUseDefaultPath = 0,
    eUseCustomPath,
    eUseEnvVar,
    eLast
};

QString getPathModeAsString(const ePathMode& val);

/**
 * @brief convertLogFileToDLTV1 - converts the content of the sourceFilePath file
 * to the DLT format and saves it to the targetFilePath.
 * Uses v1 dlt protocol.
 * Note! Each "\n" separation treated as a new message.
 * @param sourceFilePath - input file path. Should exist
 * @param targetFilePath - output file, which will be created and filled in with content.
 * Note! In case if file under the specified path already exists - it will be truncated!
 * @return - true in case of success. False otherwise.
 */
bool convertLogFileToDLTV1( const QString& sourceFilePath,
                           const QString& targetFilePath );

/**
 * @brief convertLogFileToDLTV2 - converts the content of the sourceFilePath file
 * to the DLT format and saves it to the targetFilePath.
 * Uses v2 dlt protocol.
 * Note! Each "\n" separation treated as a new message.
 * @param sourceFilePath - input file path. Should exist
 * @param targetFilePath - output file, which will be created and filled in with content.
 * Note! In case if file under the specified path already exists - it will be truncated!
 * @return - true in case of success. False otherwise.
 */
bool convertLogFileToDLTV2( const QString& sourceFilePath,
                           const QString& targetFilePath );

/**
 * @brief isDarkMode - tells whether dark mode is enabled
 */
bool isDarkMode();

/**
 * @brief getDataStrFromMsg - this function is used to get string data from the message in a unified way
 * @param msgId - id of the message
 * @param pMsg - pointer to a message wrapper
 * @param field - search result column type. Used to detect in which way to format the data
 * @return - shared pointer to a string with a gathered information
 */
tQStringPtr getDataStrFromMsg(const tMsgId& msgId, const tMsgWrapperPtr &pMsg, eSearchResultColumn field);

/**
 * @brief getChartColor - get color for chart
 */
QColor getChartColor();

/**
 * @enum eTabIndexes
 * @brief This enumeration defines the indexes of different views
 *        used as tab positions in the QTabWidget.
 */
enum class eTabIndexes
{
    /**
     * @brief Index for the search view tab.
     * @details Represents the position of the Search View in the QTabWidget.
     */
    SEARCH_VIEW = 0,

    /**
     * @brief Index for the grouped view tab.
     * @details Represents the position of the Grouped View in the QTabWidget.
     */
    GROUPED_VIEW = 1,

    /**
     * @brief Index for the UML view tab.
     * @details Represents the position of the UML View in the QTabWidget.
     */
    UML_VIEW = 2,

    /**
     * @brief Index for the plot view tab.
     * @details Represents the position of the Plot View in the QTabWidget.
     */
    PLOT_VIEW = 3,

    /**
     * @brief Index for the coverage note view tab.
     * @details Represents the position of the Coverage Note View in the QTabWidget.
     */
    COVERAGE_NOTE_VIEW = 4,

    /**
     * @brief Index for the files view tab.
     * @details Represents the position of the Files View in the QTabWidget.
     */
    FILES_VIEW = 5,

    /**
     * @brief Index for the console view tab.
     * @details Represents the position of the Console View in the QTabWidget.
     */
    CONSOLE_VIEW = 6
};

/**
 * @brief Releases unused memory back to the operating system.
 *
 * This function ensures that unused memory managed by the allocator is returned
 * to the operating system to free up resources. It supports different memory
 * management systems:
 *
 * - If `DMA_TC_MALLOC_OPTIMIZATION_ENABLED` is defined, it uses TCMalloc's
 *   `ReleaseFreeMemory` method to release unused memory.
 * - If `DMA_GLIBC_MALLOC_OPTIMIZATION_ENABLED` is defined, it uses glibc's
 *   `malloc_trim(0)` function to achieve the same effect.
 *
 * Use this function after memory-intensive operations to optimize memory usage.
 */
void releaseMemoryToOS();

#ifdef DMA_TC_MALLOC_PROFILING_ENABLED
/**
 * @brief Dumps detailed memory allocation statistics.
 *
 * This function retrieves and outputs memory usage statistics when TCMalloc
 * profiling is enabled (`DMA_TC_MALLOC_PROFILING_ENABLED`). The statistics
 * provide insights into memory allocation patterns, freelist usage, and
 * overall memory consumption managed by TCMalloc.
 *
 * The output includes a formatted summary of TCMalloc's memory state,
 * surrounded by clearly marked start and end tags for easier parsing or
 * debugging.
 *
 * **Example Output:**
 * ```
 * ----------------------------------------------------|
 * ---------------TC_MALLOC_OUTPUT_START---------------|
 * ----------------------------------------------------|
 * MALLOC: <details>
 * ----------------------------------------------------|
 * ----------------TC_MALLOC_OUTPUT_END----------------|
 * ----------------------------------------------------|
 * ```
 *
 * This method is useful for debugging and profiling memory usage in
 * applications relying on TCMalloc.
 */
void dumpMemoryStatistics();
#endif

#endif // DEFINITIONS_HPP
