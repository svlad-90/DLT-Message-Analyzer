/**
 * @file    Definitions.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the Definitions class
 */

#include <assert.h>

#include <map>
#include <vector>
#include <atomic>
#include <random>

#ifdef DMA_TC_MALLOC_OPTIMIZATION_ENABLED
#include <gperftools/malloc_extension.h>
#elif DMA_GLIBC_MALLOC_OPTIMIZATION_ENABLED
#include <malloc.h>
#endif

#include <QDateTime>

#include <QElapsedTimer>
#include <QFile>
#include <QApplication>
#include <QPalette>

#include "Definitions.hpp"

#include "QRegularExpression"
#include "QTextStream"

#include "dlt_common.h"

#include "components/logsWrapper/api/IMsgWrapper.hpp"

#include "components/log/api/CLog.hpp"
#include "CTreeItem.hpp"

#include "DMA_Plantuml.hpp"

Q_DECLARE_METATYPE(eRequestState)
Q_DECLARE_METATYPE(int8_t)
Q_DECLARE_METATYPE(tRequestId)

const QString sDefaultStatusText = "No error ...";
const QString sDefaultRegexFileName = "Regex_Default.json";
const QString sRegexScriptingDelimiter = "_and_";
const QString sRGBPrefix = "RGB_";
const QString sVARPrefix = "VAR_";
const QString sGroupedViewPrefix = "GV";

//////// UML_IDENTIFIERS ////////
const QString s_UML_SEQUENCE_ID = "USID"; // - optional
const QString s_UML_CLIENT = "UCL"; // - mandatory
const QString s_UML_REQUEST = "URT"; // - request type
const QString s_UML_RESPONSE = "URS"; // - request type
const QString s_UML_EVENT = "UEV"; // - request type
const QString s_UML_SERVICE = "US"; // - mandatory
const QString s_UML_METHOD = "UM"; // - mandatory
const QString s_UML_ARGUMENTS = "UA"; // - optional
const QString s_UML_TIMESTAMP = "UTS"; // - optional
const QString s_UML_ALIAS_DELIMITER = "_";
const QString s_Regex_options = "(?J)";

//////// OTHER CONSTANTS ////////
static const tGroupedViewIdx sInvalidGroupedViewIdx = -1;
static const tGroupedViewIdx sMaxGroupedViewIdx = std::numeric_limits<int>::max();

static tUML_IDs_Map createUMLIDsMap()
{
    tUML_IDs_Map result;

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_Optional;
        item.id_str = s_UML_TIMESTAMP;
        item.description = "Call timestamp";
        result.insert(std::make_pair(eUML_ID::UML_TIMESTAMP, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_Optional;
        item.id_str = s_UML_SEQUENCE_ID;
        item.description = "Sequence id of the communication";
        result.insert(std::make_pair(eUML_ID::UML_SEQUENCE_ID, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_Mandatory;
        item.id_str = s_UML_CLIENT;
        item.description = "Name of the client";
        result.insert(std::make_pair(eUML_ID::UML_CLIENT, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_RequestType;
        item.id_str = s_UML_REQUEST;
        item.description = "Request identifier";
        result.insert(std::make_pair(eUML_ID::UML_REQUEST, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_RequestType;
        item.id_str = s_UML_RESPONSE;
        item.description = "Response identifier";
        result.insert(std::make_pair(eUML_ID::UML_RESPONSE, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_RequestType;
        item.id_str = s_UML_EVENT;
        item.description = "Event identifier";
        result.insert(std::make_pair(eUML_ID::UML_EVENT, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_Mandatory;
        item.id_str = s_UML_SERVICE;
        item.description = "Service name";
        result.insert(std::make_pair(eUML_ID::UML_SERVICE, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_Mandatory;
        item.id_str = s_UML_METHOD;
        item.description = "Method name";
        result.insert(std::make_pair(eUML_ID::UML_METHOD, item));
    }

    {
        tUML_ID_Item item;
        item.id_type = eUML_ID_Type::e_Optional;
        item.id_str = s_UML_ARGUMENTS;
        item.description = "Call arguments";
        result.insert(std::make_pair(eUML_ID::UML_ARGUMENTS, item));
    }

    return result;
}

const tUML_IDs_Map s_UML_IDs_Map = createUMLIDsMap();

const tMsgId INVALID_MSG_ID = -1;
const tRequestId INVALID_REQUEST_ID = static_cast<uint>(-1);

const std::map<QString, QColor>& getColorsMap()
{
    static const std::map<QString, QColor> sColorsMap =
        {
         {"ok", QColor(0,150,0)},
         {"warning", QColor(150,150,0)},
         {"error", QColor(150,0,0)},
         {"black", QColor(0,0,0)},
         {"white", QColor(255,255,255)},
         {"red", QColor(255,0,0)},
         {"lime", QColor(0,255,0)},
         {"blue", QColor(0,0,255)},
         {"yellow", QColor(255,255,0)},
         {"cyan", QColor(0,255,255)},
         {"magenta", QColor(255,0,255)},
         {"silver", QColor(192,192,192)},
         {"gray", QColor(128,128,128)},
         {"maroon", QColor(128,0,0)},
         {"olive", QColor(128,128,0)},
         {"green", QColor(0,128,0)},
         {"purple", QColor(128,0,128)},
         {"teal", QColor(0,128,128)},
         {"navy", QColor(0,0,128)},
         {"maroon", QColor(128,0,0)},
         {"dark_red", QColor(139,0,0)},
         {"brown", QColor(165,42,42)},
         {"firebrick", QColor(178,34,34)},
         {"crimson", QColor(220,20,60)},
         {"red", QColor(255,0,0)},
         {"tomato", QColor(255,99,71)},
         {"coral", QColor(255,127,80)},
         {"indian_red", QColor(205,92,92)},
         {"light_coral", QColor(240,128,128)},
         {"dark_salmon", QColor(233,150,122)},
         {"salmon", QColor(250,128,114)},
         {"light_salmon", QColor(255,160,122)},
         {"orange_red", QColor(255,69,0)},
         {"dark_orange", QColor(255,140,0)},
         {"orange", QColor(255,165,0)},
         {"gold", QColor(255,215,0)},
         {"dark_golden_rod", QColor(184,134,11)},
         {"golden_rod", QColor(218,165,32)},
         {"pale_golden_rod", QColor(238,232,170)},
         {"dark_khaki", QColor(189,183,107)},
         {"khaki", QColor(240,230,140)},
         {"olive", QColor(128,128,0)},
         {"yellow", QColor(255,255,0)},
         {"yellow_green", QColor(154,205,50)},
         {"dark_olive_green", QColor(85,107,47)},
         {"olive_drab", QColor(107,142,35)},
         {"lawn_green", QColor(124,252,0)},
         {"chart_reuse", QColor(127,255,0)},
         {"green_yellow", QColor(173,255,47)},
         {"dark_green", QColor(0,100,0)},
         {"green", QColor(0,128,0)},
         {"forest_green", QColor(34,139,34)},
         {"lime", QColor(0,255,0)},
         {"lime_green", QColor(50,205,50)},
         {"light_green", QColor(144,238,144)},
         {"pale_green", QColor(152,251,152)},
         {"dark_sea_green", QColor(143,188,143)},
         {"medium_spring_green", QColor(0,250,154)},
         {"spring_green", QColor(0,255,127)},
         {"sea_green", QColor(46,139,87)},
         {"medium_aqua_marine", QColor(102,205,170)},
         {"medium_sea_green", QColor(60,179,113)},
         {"light_sea_green", QColor(32,178,170)},
         {"dark_slate_gray", QColor(47,79,79)},
         {"teal", QColor(0,128,128)},
         {"dark_cyan", QColor(0,139,139)},
         {"aqua", QColor(0,255,255)},
         {"cyan", QColor(0,255,255)},
         {"light_cyan", QColor(224,255,255)},
         {"dark_turquoise", QColor(0,206,209)},
         {"turquoise", QColor(64,224,208)},
         {"medium_turquoise", QColor(72,209,204)},
         {"pale_turquoise", QColor(175,238,238)},
         {"aqua_marine", QColor(127,255,212)},
         {"powder_blue", QColor(176,224,230)},
         {"cadet_blue", QColor(95,158,160)},
         {"steel_blue", QColor(70,130,180)},
         {"corn_flower_blue", QColor(100,149,237)},
         {"deep_sky_blue", QColor(0,191,255)},
         {"dodger_blue", QColor(30,144,255)},
         {"light_blue", QColor(173,216,230)},
         {"sky_blue", QColor(135,206,235)},
         {"light_sky_blue", QColor(135,206,250)},
         {"midnight_blue", QColor(25,25,112)},
         {"navy", QColor(0,0,128)},
         {"dark_blue", QColor(0,0,139)},
         {"medium_blue", QColor(0,0,205)},
         {"blue", QColor(0,0,255)},
         {"royal_blue", QColor(65,105,225)},
         {"blue_violet", QColor(138,43,226)},
         {"indigo", QColor(75,0,130)},
         {"dark_slate_blue", QColor(72,61,139)},
         {"slate_blue", QColor(106,90,205)},
         {"medium_slate_blue", QColor(123,104,238)},
         {"medium_purple", QColor(147,112,219)},
         {"dark_magenta", QColor(139,0,139)},
         {"dark_violet", QColor(148,0,211)},
         {"dark_orchid", QColor(153,50,204)},
         {"medium_orchid", QColor(186,85,211)},
         {"purple", QColor(128,0,128)},
         {"thistle", QColor(216,191,216)},
         {"plum", QColor(221,160,221)},
         {"violet", QColor(238,130,238)},
         {"magenta", QColor(255,0,255)},
         {"orchid", QColor(218,112,214)},
         {"medium_violet_red", QColor(199,21,133)},
         {"pale_violet_red", QColor(219,112,147)},
         {"deep_pink", QColor(255,20,147)},
         {"hot_pink", QColor(255,105,180)},
         {"light_pink", QColor(255,182,193)},
         {"pink", QColor(255,192,203)},
         {"antique_white", QColor(250,235,215)},
         {"beige", QColor(245,245,220)},
         {"bisque", QColor(255,228,196)},
         {"blanched_almond", QColor(255,235,205)},
         {"wheat", QColor(245,222,179)},
         {"corn_silk", QColor(255,248,220)},
         {"lemon_chiffon", QColor(255,250,205)},
         {"light golden rod yellow", QColor(250,250,210)},
         {"light_yellow", QColor(255,255,224)},
         {"saddle_brown", QColor(139,69,19)},
         {"sienna", QColor(160,82,45)},
         {"chocolate", QColor(210,105,30)},
         {"peru", QColor(205,133,63)},
         {"sandy_brown", QColor(244,164,96)},
         {"burly_wood", QColor(222,184,135)},
         {"tan", QColor(210,180,140)},
         {"rosy_brown", QColor(188,143,143)},
         {"moccasin", QColor(255,228,181)},
         {"navajo_white", QColor(255,222,173)},
         {"peach_puff", QColor(255,218,185)},
         {"misty_rose", QColor(255,228,225)},
         {"lavender_blush", QColor(255,240,245)},
         {"linen", QColor(250,240,230)},
         {"old_lace", QColor(253,245,230)},
         {"papaya_whip", QColor(255,239,213)},
         {"sea_shell", QColor(255,245,238)},
         {"mint_cream", QColor(245,255,250)},
         {"slate_gray", QColor(112,128,144)},
         {"light_slate_gray", QColor(119,136,153)},
         {"light_steel_blue", QColor(176,196,222)},
         {"lavender", QColor(230,230,250)},
         {"floral_white", QColor(255,250,240)},
         {"alice_blue", QColor(240,248,255)},
         {"ghost_white", QColor(248,248,255)},
         {"honeydew", QColor(240,255,240)},
         {"ivory", QColor(255,255,240)},
         {"azure", QColor(240,255,255)},
         {"snow", QColor(255,250,250)},
         {"black", QColor(0,0,0)},
         {"dim_gray", QColor(105,105,105)},
         {"gray", QColor(128,128,128)},
         {"dark_gray", QColor(169,169,169)},
         {"silver", QColor(192,192,192)},
         {"ight_gray", QColor(211,211,211)},
         {"gainsboro", QColor(220,220,220)},
         {"white_smoke", QColor(245,245,245)},
         {"white", QColor(255,255,255)}};

    return sColorsMap;
}

static std::list<QColor> createColorsList()
{
    std::list<QColor> result;

    const auto& colorsMap = getColorsMap();

    for(const auto& colorPair : colorsMap)
    {
        result.push_back(colorPair.second);
    }

    return result;
}

const std::list<QColor>& getColorsList()
{
    static std::list<QColor> sColorsList = createColorsList();
    return sColorsList;
}

QVector<QColor> generateColors( const tHighlightingGradient& gradient )
{
    QVector<QColor> result;

    result.push_back(gradient.from);

    int maxIterations = gradient.numberOfColors >= 2 ? gradient.numberOfColors - 2 : 2;

    if( maxIterations > 0 )
    {
        int rStart = gradient.from.red();
        int rEnd = gradient.to.red();
        int rDiff = rEnd - rStart;
        int rStep = rDiff / ( maxIterations + 1 );

        int gStart = gradient.from.green();
        int gEnd = gradient.to.green();
        int gDiff = gEnd - gStart;
        int gStep = gDiff / ( maxIterations + 1 );

        int bStart = gradient.from.blue();
        int bEnd = gradient.to.blue();
        int bDiff = bEnd - bStart;
        int bStep = bDiff / ( maxIterations + 1 );

        for( int i = 1; i <= maxIterations; ++i )
        {
            QColor color;

            color.setRed(rStart + rStep * i);
            color.setGreen(gStart + gStep * i);
            color.setBlue(bStart + bStep * i);

            result.push_back(color);
        }
    }

    result.push_back(gradient.to);

    return result;
}

QString getUMLIDAsString( const eUML_ID& val )
{
    QString result;

    auto foundValue = s_UML_IDs_Map.find(val);

    if(foundValue != s_UML_IDs_Map.end())
    {
        result = foundValue->second.id_str;
    }

    return result;
}

bool parseUMLIDFromString( const QString& data, eUML_ID& val )
{
    bool bResult = false;

    if(false == data.isEmpty())
    {
        for(const auto& item : s_UML_IDs_Map)
        {
            if( data.compare(item.second.id_str, Qt::CaseInsensitive) == 0 )
            {
                val = item.first;
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

QString getUMLIDTypeAsString( const eUML_ID_Type& val )
{
    QString result;

    switch( val )
    {
        case eUML_ID_Type::e_Optional:
        {
            result = "optional";
        }
            break;
        case eUML_ID_Type::e_Mandatory:
        {
            result = "mandatory";
        }
            break;
        case eUML_ID_Type::e_RequestType:
        {
            result = "request_type";
        }
            break;
    }

    return result;
}

/// tHighlightingRange
tHighlightingRange::tHighlightingRange( const tHighlightingRangeItem& from_,
                                        const tHighlightingRangeItem& to_,
                                        const QColor& color_,
                                        bool explicitColor_ ):
from(from_), to(to_), color_code(color_.rgb()), explicitColor(explicitColor_)
{}

tHighlightingRange::tHighlightingRange():
from(0), to(0), color_code(RGB_MASK), explicitColor(false)
{}

bool tHighlightingRange::operator< ( const tHighlightingRange& rVal ) const
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

/////////////////////////////tQStringPtrWrapper///////////////////////////
tQStringPtrWrapper::tQStringPtrWrapper(): pString(nullptr)
{}

tQStringPtrWrapper::tQStringPtrWrapper(const tQStringPtr& pString_): pString(pString_)
{}

bool tQStringPtrWrapper::operator< ( const tQStringPtrWrapper& rVal ) const
{
    bool bResult = false;

    if(pString == nullptr && rVal.pString != nullptr)
        bResult = true;
    else if(pString != nullptr && rVal.pString == nullptr)
        bResult = false;
    else if(pString == nullptr && rVal.pString == nullptr)
        bResult = true;
    else
    {
        if( *pString < *rVal.pString )
        {
            bResult = true;
        }
    }

    return bResult;
}

bool tQStringPtrWrapper::operator== ( const tQStringPtrWrapper& rVal ) const
{
    if(pString == nullptr && rVal.pString != nullptr)
        return false;
    else if(pString != nullptr && rVal.pString == nullptr)
        return false;
    else if(pString == nullptr && rVal.pString == nullptr)
        return true;

    return ( *pString == *rVal.pString );
}

bool tQStringPtrWrapper::operator!= ( const tQStringPtrWrapper& rVal ) const
{
    return !(*this == rVal);
}
//////////////////////////////////////////////////////////////////////////

struct tAnalysisRange
{
    tAnalysisRange(const tIntRange& inputRange,
                   const tIntRange& shrinkToRange):
        bFromShrinked(false),
        bToShrinked(false)
    {
        if(inputRange.from < shrinkToRange.from)
        {
            from = shrinkToRange.from;
            bFromShrinked = true;
        }
        else
        {
            from = inputRange.from;
        }

        if(inputRange.to > shrinkToRange.to)
        {
            to = shrinkToRange.to;
            bToShrinked = true;
        }
        else
        {
            to = inputRange.to;
        }
    }

    tIntRange::tRangeItem from;
    bool bFromShrinked;
    tIntRange::tRangeItem to;
    bool bToShrinked;
};

//#define DEBUG_CALC_RANGES
//static const int TRACE_THREASHOLD_NS = 200000;

enum eTreeColumns
{
    eTreeColumn_FoundMatch = 0
};

tTreeItemSharedPtr getMatchesTree( const tFoundMatches& foundMatches )
{
    /////////////////////////////FORM TREE///////////////////////////////////

#ifdef DEBUG_CALC_RANGES
    QElapsedTimer timer;
    timer.start();

    auto nsecStart = timer.nsecsElapsed();
    auto fillInTreeTime = 0u;
#endif

    // we should form a tree of all incoming matches in order to properly assign the colors
    tTreeItemSharedPtr pRootItem =
    std::make_shared<CTreeItem>(nullptr, static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch),
                                CTreeItem::tSortingFunction(),
                                CTreeItem::tHandleDuplicateFunc(),
                                CTreeItem::tFindItemFunc());

    {
        auto formData = [](const tFoundMatch& match) -> CTreeItem::tData
        {
            CTreeItem::tData result;
            result.reserve(1);

            tDataItem matchVariant( &match );
            result.push_back( matchVariant );

            return result;
        };

        auto pCurrentItem = pRootItem.get();

        typedef std::vector<const tFoundMatch*> tStack;
        tStack matchesStack;

        auto switchToParent = [&pCurrentItem, &matchesStack](int numberOfElementsToPop)
        {
            int counter = 0;

            while(counter < numberOfElementsToPop)
            {
                pCurrentItem = pCurrentItem->getParent();
                ++counter;
            }

            matchesStack.resize(matchesStack.size() - static_cast<std::size_t>(numberOfElementsToPop));
        };

        auto appendChild = [&pCurrentItem, &formData, &matchesStack](const tFoundMatch& match)
        {
            auto data = formData(match);

            assert(false == data.empty());

            tIntRange range = match.range;
            tDataItem rangeVariant( range );
            auto* pAddedChild = pCurrentItem->appendChild(rangeVariant, data);
            pCurrentItem = pAddedChild;

            matchesStack.push_back( &match ); // push new element to the stack
        };

        for(const auto& match : foundMatches.foundMatchesVec)
        {
            int counter = 0;

            for(auto it = matchesStack.rbegin(); it != matchesStack.rend(); ++it)
            {
                if( match.range.from >= (*it)->range.from &&
                    match.range.to <= (*it)->range.to ) // if we are still inside the range
                {
                    // break the loop
                    break;
                }
                else // otherwise
                {
                    ++counter; // increment the counter
                }
            }

            if(counter > 0) // if counter is greater than 0
            {
                switchToParent( counter );
            }

#ifdef DEBUG_CALC_RANGES
            auto nsecBeforeFillTree = timer.nsecsElapsed();
#endif
            appendChild(match);

#ifdef DEBUG_CALC_RANGES
            auto nsecAfterFillTree = timer.nsecsElapsed();
            fillInTreeTime += nsecAfterFillTree - nsecBeforeFillTree;
#endif
        }
    }

    /////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_CALC_RANGES
    {
        auto nsecBuildTree = timer.nsecsElapsed();
        auto buildTreeTime = nsecBuildTree - nsecStart;
        if(buildTreeTime > TRACE_THREASHOLD_NS)
            SEND_MSG(QString("[%1] fillInTreeTime - %2 nsec; buildTreeTime - %3")
                     .arg(__FUNCTION__)
                     .arg(fillInTreeTime)
                     .arg(buildTreeTime));
    }
#endif

    return pRootItem;
}

tCalcRangesCoverageMulticolorResult calcRangesCoverageMulticolor( const tTreeItemSharedPtr& pMatchesTree,
                               const tIntRange& inputRange,
                               const tRegexScriptingMetadata& regexScriptingMetadata,
                               const QVector<QColor>& gradientColors,
                               const tGroupIdToColorMap& groupIdToColorMap )
{
#ifdef DEBUG_CALC_RANGES
    QElapsedTimer timer;
    timer.start();

    auto nsecStart = timer.nsecsElapsed();
#endif

    if(true == gradientColors.empty()) // let's reject bad case, when collection of input colors is empty
    {
        tCalcRangesCoverageMulticolorResult dummyResult;
        return dummyResult;
    }

    tCalcRangesCoverageMulticolorResult result;
    tHighlightingRangeVec& resultVec = result.highlightingRangeVec;
    resultVec.reserve(static_cast<std::size_t>(regexScriptingMetadata.getItemsVec().size()));

    auto postVisitFunction = [&resultVec, &inputRange, &regexScriptingMetadata, &gradientColors, &groupIdToColorMap](tTreeItem* pItem)
    {
        const auto& match = *(pItem->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());

        //SEND_MSG(QString("[calcRangesCoverageMulticolor][postVisitFunction] visit item - |idx:%1;range:from-%2,to-%3;msg-%4|")
        //         .arg(match.idx)
        //         .arg(match.range.from)
        //         .arg(match.range.to)
        //         .arg(match.matchStr));

        if( ( match.range.from < inputRange.from && match.range.to < inputRange.from ) ||
            ( match.range.from > inputRange.to && match.range.to > inputRange.to ) )
        {
            // we are outside target range.
            // We can skip further analysis of this match.
            return true;
        }

        tAnalysisRange analysisRange( match.range, inputRange );
        const auto& matchIdx = match.idx;

        auto getColor = [&regexScriptingMetadata,
                &gradientColors,
                &groupIdToColorMap,
                &matchIdx]()->QPair<bool /*isExplicit*/, QColor>
        {
            QOptionalColor scriptedColor;
            scriptedColor.isSet = false;

            if( matchIdx < regexScriptingMetadata.getItemsVec().size() )
            {
                scriptedColor = regexScriptingMetadata.getItemsVec().operator[](matchIdx)->highlightingColor;
            }

            QColor color;
            bool bIsExplicitColor = false;

            if(true == scriptedColor.isSet)
            {
                color = QColor(scriptedColor.color_code);
                bIsExplicitColor = true;
            }
            else
            {
                auto foundGradientColor = groupIdToColorMap.find(matchIdx);

                if( groupIdToColorMap.end() != foundGradientColor )
                {
                    color = gradientColors[foundGradientColor->second];
                }
            }

            return QPair<bool /*isExplicit*/, QColor>(bIsExplicitColor, color);
        };

        // we need to analyze first level children of pItem and split the ranges
        const auto& children = pItem->getChildren();

        auto selectedColor = getColor();

        if(false == children.isEmpty()) // if there are some children available
        {
            // we need to process "from beginning to first child" case manually
            {
                const auto& firstChild = *children.begin();
                const auto& firstChildMatch = *(firstChild->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                tAnalysisRange firstChildAnalysisRange(firstChildMatch.range, inputRange);

                if( firstChildAnalysisRange.from > analysisRange.from ) // if there is room for additional range
                {
                    // let's add it
                    tAnalysisRange rangeToBeAdded( tIntRange( analysisRange.from, firstChildAnalysisRange.from - 1 ), inputRange);
                    resultVec.push_back( tHighlightingRange( rangeToBeAdded.from,
                                                              rangeToBeAdded.to,
                                                              selectedColor.second,
                                                              selectedColor.first ) );
                }
            }

            // wee need to process "between children" cases in a loop ( in case if there are 2 children or more )
            if(children.size() >= 2)
            {
                for(auto it = children.begin(); it != std::prev(children.end()); ++it)
                {
                    const auto& firstChildMatch = *((*it)->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                    tAnalysisRange firstChildAnalysisRange( firstChildMatch.range, inputRange );

                    const auto& secondChildMatch = *((*(std::next(it)))->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                    tAnalysisRange secondChildAnalysisRange( secondChildMatch.range, inputRange );

                    if((secondChildAnalysisRange.from - firstChildAnalysisRange.to) > 1)
                    {
                        tAnalysisRange rangeToBeAdded( tIntRange( firstChildAnalysisRange.to+1, secondChildAnalysisRange.from-1 ), inputRange);
                        resultVec.push_back( tHighlightingRange( rangeToBeAdded.from,
                                                                  rangeToBeAdded.to,
                                                                  selectedColor.second,
                                                                  selectedColor.first ) );
                    }
                }
            }

            // we need to process "from last child to end" case manually
            {
                const auto& lastChild = *(std::prev(children.end()));
                const auto& lastChildMatch = *(lastChild->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                tAnalysisRange lastChildAnalysisRange(lastChildMatch.range, inputRange);

                if( analysisRange.to > lastChildAnalysisRange.to ) // if there is room for additional range
                {
                    // let's add it
                    tAnalysisRange rangeToBeAdded( tIntRange( lastChildAnalysisRange.to + 1, analysisRange.to ), inputRange);
                    resultVec.push_back( tHighlightingRange( rangeToBeAdded.from,
                                                              rangeToBeAdded.to,
                                                              selectedColor.second,
                                                              selectedColor.first ) );
                }
            }
        }
        else // if there are no children
        {
            // let's add item unconditionally
            resultVec.push_back( tHighlightingRange( analysisRange.from,
                                                      analysisRange.to,
                                                      selectedColor.second,
                                                      selectedColor.first ) );
        }

        return true;
    };

    pMatchesTree->visit(CTreeItem::tVisitFunction(), postVisitFunction, false, true, false);

    int normalizationIdx = inputRange.from;

    if(0 != normalizationIdx)
    {
        for(auto& resultVecItem : resultVec)
        {
            resultVecItem.from = resultVecItem.from - normalizationIdx;
            resultVecItem.to = resultVecItem.to - normalizationIdx;
        }
    }

    //  for( const auto& resultItem : result.highlightingRangeVec )
    //  {
    //      SEND_MSG(QString("[calcRangesCoverageMulticolor][result] - |from-%1;to-%2;color-%3,expColor-%4|")
    //              .arg(resultItem.from)
    //              .arg(resultItem.to)
    //              .arg(resultItem.color_code)
    //              .arg(resultItem.explicitColor));
    //  }

#ifdef DEBUG_CALC_RANGES
    auto nsecEnd = timer.nsecsElapsed();
    auto timeVal = nsecEnd - nsecStart;
    if(timeVal > TRACE_THREASHOLD_NS)
        SEND_MSG(QString("[%1] Spent time - %2 nsec")
                 .arg(__FUNCTION__)
                 .arg(nsecEnd - nsecStart));
#endif

    return result;
}

//tItemMetadata
tItemMetadata::tItemMetadata():
    highlightingInfoMultiColor(),
    fieldRanges(),
    msgId(-1),
    strSize(0),
    timeStamp(0u),
    msgSize(0u)
{

}

tItemMetadata::tItemMetadata(const tItemMetadata& rhs):
highlightingInfoMultiColor(rhs.highlightingInfoMultiColor),
fieldRanges(rhs.fieldRanges),
pUMLInfo(rhs.pUMLInfo == nullptr ? nullptr : std::make_unique<tUMLInfo>(*rhs.pUMLInfo)),
pPlotViewInfo(rhs.pPlotViewInfo == nullptr ? nullptr : std::make_unique<tPlotViewInfo>(*rhs.pPlotViewInfo)),
msgId(rhs.msgId),
msgIdxInMainTable(rhs.msgIdxInMainTable),
strSize(rhs.strSize),
timeStamp(rhs.timeStamp),
msgSize(rhs.msgSize)
{

}

tItemMetadata& tItemMetadata::operator= (const tItemMetadata& rhs)
{
    if(&rhs == this)
        return *this;

    highlightingInfoMultiColor = rhs.highlightingInfoMultiColor;
    fieldRanges = rhs.fieldRanges;
    pUMLInfo = rhs.pUMLInfo == nullptr ? nullptr : std::make_unique<tUMLInfo>(*rhs.pUMLInfo);
    pPlotViewInfo = rhs.pPlotViewInfo == nullptr ? nullptr : std::make_unique<tPlotViewInfo>(*rhs.pPlotViewInfo);
    msgId = rhs.msgId;
    msgIdxInMainTable = rhs.msgIdxInMainTable;
    strSize = rhs.strSize;
    timeStamp = rhs.timeStamp;
    msgSize = rhs.msgSize;

    return *this;
}

tItemMetadata::tItemMetadata( const tMsgId& msgId_,
                              const tMsgId& msgIdxInMainTable_,
                              const tFieldRanges& fieldRanges_,
                              const int& strSize_,
                              const std::uint32_t& msgSize_,
                              const unsigned int& timeStamp_):
    fieldRanges(fieldRanges_),
    msgId(msgId_),
    msgIdxInMainTable(msgIdxInMainTable_),
    strSize(strSize_),
    timeStamp(timeStamp_),
    msgSize(msgSize_)
{

}

tTreeItemSharedPtr tItemMetadata::updateHighlightingInfo( const tFoundMatches& foundMatches,
                                                          const QVector<QColor>& gradientColors,
                                                          const tRegexScriptingMetadata& regexScriptingMetadata,
                                                          tTreeItemSharedPtr pTree)
{
    //QElapsedTimer timer;
    //timer.start();

    auto createColorsMappingTable = [&foundMatches, &gradientColors]()->tGroupIdToColorMap
    {
        tGroupIdToColorMap result;

        auto maxGradientColorsSize = gradientColors.size();
        int gradientColorsCounter = 0;

        struct tSortedMatchKey
        {
            tSortedMatchKey(const int& from_, const int& size_):
            from(from_),
            size(size_)
            {}

            bool operator< (const tSortedMatchKey& rVal) const
            {
                bool bResult = false;

                if(from != rVal.from)
                {
                    bResult = from < rVal.from;
                }
                else
                {
                    bResult = size < rVal.size;
                }

                return bResult;
            }

            int from;
            int size;
        };

        typedef std::map<tSortedMatchKey, const tFoundMatch*> tSortedMatches;
        tSortedMatches sortedMatches;

        for(const auto& match : foundMatches.foundMatchesVec)
        {
            sortedMatches.insert(std::make_pair(tSortedMatchKey(match.range.from, match.range.to - match.range.from), &match));
        }

        for(const auto& match : sortedMatches)
        {
            if(false == match.second->matchStr.isEmpty())
            {
                result.insert(std::make_pair( match.second->idx, gradientColorsCounter % maxGradientColorsSize ));
                ++gradientColorsCounter;
            }
        }

        return result;
    };

    auto pMatchesTree = pTree != nullptr ? pTree : getMatchesTree(foundMatches);

    for( auto it = fieldRanges.begin(); it != fieldRanges.end(); ++it )
    {
        auto highlightingRangesMulticolorRes = calcRangesCoverageMulticolor( pMatchesTree, it.value(), regexScriptingMetadata, gradientColors, createColorsMappingTable() );

        if(false == highlightingRangesMulticolorRes.highlightingRangeVec.empty())
        {
            highlightingInfoMultiColor.insert( it.key(), highlightingRangesMulticolorRes.highlightingRangeVec );
        }
    }

    //static std::atomic<int> counter(0);

    //if(counter % 1000 == 0)
    //{
    //    auto elapsedTimeNano = timer.nsecsElapsed();
    //    SEND_MSG(QString("[%1] Item #%2 - elapsed time - %3").arg(__FUNCTION__).arg(counter).arg(elapsedTimeNano));
    //}

    //++counter;

    return pMatchesTree;
}

tItemMetadata::tUpdateUMLInfoResult tItemMetadata::updateUMLInfo(const tFoundMatches& foundMatches,
                                                                 const tRegexScriptingMetadata& regexScriptingMetadata,
                                                                 tTreeItemSharedPtr pTree)
{
    tUpdateUMLInfoResult result;

    tRegexScriptingMetadata::tCheckIDs checkIDs;

    for(const auto& match : foundMatches.foundMatchesVec)
    {
        checkIDs.insert(match.idx);
    }

    pUMLInfo = std::make_unique<tUMLInfo>();

    // check if string matches all required UML attributes
    pUMLInfo->bUMLConstraintsFulfilled = regexScriptingMetadata.doesContainConsistentUMLData(false, checkIDs).first;
    pUMLInfo->UMLDataMap.clear();

    /*
     * What should we fill in here?
     * E.g. for the following string:
     * UI
     * IF1
     * [comp.if1verbose] 10886 Service#2:RP : setIsClientConnected() [unknown:0]]
     * With the following regex:
     * ^(?<UCL>[\w]+) IF1 .*\] (?<USID>[\d]+) (?<US>[A-Za-z]+#[\d]+):(?<URS>RP) : (?<UM>[\w])\((?<UA>.*)\) \[unknown
     * We will need to fill in the following information:
     * tUMLInfo( bUMLConstraintsFulfilled = true,
     * UMLDataMap( { eUML_ID::UML_CLIENT, { eSearchResultColumn::Apid, {0, 3} },
     *             { eUML_ID::UML_SEQUENCE_ID, { eSearchResultColumn::Payload, {22, 26} }
     *             { eUML_ID::UML_SERVICE, { eSearchResultColumn::Payload, {28, 40} }
     *             { eUML_ID::UML_RESPONSE, { eSearchResultColumn::Payload, {44, 45} } ),
     * bContains_Req_Resp_Ev = false
     *  );
     */

    auto pMatchesTree = pTree != nullptr ? pTree : getMatchesTree(foundMatches);

    if(true == pUMLInfo->bUMLConstraintsFulfilled) // if UML matches are sufficient
    {
        if(nullptr != pMatchesTree)
        {
            auto preVisitFunction = [&regexScriptingMetadata, &result, this](tTreeItem* pItem)
            {
                const auto& match = *(pItem->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());

                // for each tree element we should check, whether it is related to UML data representation.
                // That can be done with checking each tree element against regexScriptingMetadata

                // lambda, which checks whether specific match is representing a UML data
                auto isUMLData = [&match, &regexScriptingMetadata]()->std::pair<bool, tOptional_UML_IDMap*>
                {
                    std::pair<bool, tOptional_UML_IDMap*> res;
                    res.first = false;

                    auto groupIdx = match.idx;
                    auto groups = regexScriptingMetadata.getItemsVec();

                    if(groupIdx >= 0 && groupIdx < groups.size())
                    {
                        const auto& pGroupMetadata = groups[groupIdx];

                        if(nullptr != pGroupMetadata &&
                            ( false == pGroupMetadata->optionalUML_ID.optional_UML_IDMap.empty() ) )
                        {
                            res.first = true; // we got the UML data
                            res.second = &pGroupMetadata->optionalUML_ID.optional_UML_IDMap;
                        }
                    }

                    return res;
                };

                auto UMLDataRes = isUMLData();

                if(true == UMLDataRes.first) // if we have a UML data
                {
                    // let's check and fill in the areas of the string, in which it is located.

                    for(const auto& UML_IDItem : *(UMLDataRes.second))
                    {
                        auto isReqResEv = [&UML_IDItem]()
                        {
                            return UML_IDItem.first == eUML_ID::UML_REQUEST ||
                                   UML_IDItem.first == eUML_ID::UML_RESPONSE ||
                                   UML_IDItem.first == eUML_ID::UML_EVENT;
                        };

                        if(false == isReqResEv() || false == pUMLInfo->bContains_Req_Resp_Ev)
                        {
                            if(nullptr == UML_IDItem.second.pUML_Custom_Value ||
                               true == UML_IDItem.second.pUML_Custom_Value->isEmpty()) // if there is no client's custom value
                            {
                                //let's grab groups content
                                tUMLDataItem UMLDataItem;

                                for(auto it = fieldRanges.begin(); it != fieldRanges.end(); ++it)
                                {
                                    const auto& fieldRange = *it;

                                    bool insideRange = (match.range.from >= fieldRange.from || match.range.to >= fieldRange.from) &&
                                                       (match.range.from <= fieldRange.to || match.range.to <= fieldRange.to);

                                    if(true == insideRange) // if group is even partially inside the range
                                    {
                                        tStringCoverageItem stringCoverageItem;
                                        stringCoverageItem.range = tIntRange( std::max(fieldRange.from, match.range.from) - fieldRange.from,
                                                                             std::min( fieldRange.to, match.range.to ) - fieldRange.from );
                                        stringCoverageItem.bAddSeparator = match.range.to > fieldRange.to;
                                        UMLDataItem.stringCoverageMap[it.key()] = stringCoverageItem;
                                    }
                                }

                                pUMLInfo->UMLDataMap[UML_IDItem.first].push_back(UMLDataItem);
                            }
                            else // otherwise
                            {
                                // let's directly assign custom client's value
                                tUMLDataItem UMLDataItem;
                                UMLDataItem.pUML_Custom_Value = UML_IDItem.second.pUML_Custom_Value;
                                pUMLInfo->UMLDataMap[UML_IDItem.first].push_back(UMLDataItem);
                            }

                            if(true == isReqResEv())
                            {
                                pUMLInfo->bContains_Req_Resp_Ev = true;
                            }
                        }
                        else
                        {
                            result.bUML_Req_Res_Ev_DuplicateFound = true;
                        }
                    }
                }

                return true;
            };

            pMatchesTree->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);
        }
    }

    result.pTreeItem = pMatchesTree;

    return result;
}

tItemMetadata::tUpdatePlotViewInfoResult
tItemMetadata::updatePlotViewInfo(const tFoundMatches& foundMatches,
                                  const tRegexScriptingMetadata& regexScriptingMetadata,
                                  tTreeItemSharedPtr pTree)
{
    tUpdatePlotViewInfoResult result;

    tRegexScriptingMetadata::tCheckIDs checkIDs;

    for(const auto& match : foundMatches.foundMatchesVec)
    {
        checkIDs.insert(match.idx);
    }

    pPlotViewInfo = std::make_unique<tPlotViewInfo>();

    // check if string matches all required UML attributes
    pPlotViewInfo->bPlotViewConstraintsFulfilled = regexScriptingMetadata.doesContainConsistentPlotViewData(false, checkIDs, true).first;
    pPlotViewInfo->plotViewDataMap.clear();

    /*
     * What should we fill in here?
     * E.g. for the following string:
     * {"TIMESTAMP": 1682268321.122626, "MODULE_NAME": "CPU_LOAD", "THREAD_ID": 281472591822624, "API_NAME": "MY_API", "CPU": 7.142857}
     * With the following regex:
     * (?<PXN_CPUC_Timestamp>(?<PYN_CPUC_CPUConsumption>{))"TIMESTAMP": (?<PXData_CPUC_1>([\d]+\.[\d]+)).*?
     * "MODULE_NAME": .*?"CPU_LOAD".*API_NAME": "(?<PGN_CPUC>.*?)".*?"CPU": (?<PYData_CPUC_1>[\d]+\.[\d]+)
     * We will need to fill in the following information:
     * tPlotViewInfo( bPlotViewConstraintsFulfilled = true,
     * plotViewDataMap( { ePlotViewID::PLOT_X_NAME, { eSearchResultColumn::Payload, {0, 0} },
     *             { ePlotViewID::PLOT_Y_NAME, { eSearchResultColumn::Payload, {0, 0} },
     *             { ePlotViewID::PLOT_X_DATA, { eSearchResultColumn::Payload, {15, 31} },
     *             { ePlotViewID::PLOT_GRAPH_NAME, { eSearchResultColumn::Payload, {104, 109} },
     *             { ePlotViewID::PLOT_Y_DATA, { eSearchResultColumn::Payload, {120, 127} } )
     * );
     */

    auto pMatchesTree = pTree != nullptr ? pTree : getMatchesTree(foundMatches);

    if(true == pPlotViewInfo->bPlotViewConstraintsFulfilled) // if UML matches are sufficient
    {
        if(nullptr != pMatchesTree)
        {
            auto preVisitFunction = [&regexScriptingMetadata, this](tTreeItem* pItem)
            {
                const auto& match = *(pItem->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());

                // for each tree element we should check, whether it is related to the plot view data representation.
                // That can be done with checking each tree element against regexScriptingMetadata

                // lambda, which checks whether specific match is representing a plot view data
                auto isPlotViewData = [&match, &regexScriptingMetadata]()->std::pair<bool, tPlotViewIDParametersMap*>
                {
                    std::pair<bool, tPlotViewIDParametersMap*> res;
                    res.first = false;

                    auto groupIdx = match.idx;
                    auto groups = regexScriptingMetadata.getItemsVec();

                    if(groupIdx >= 0 && groupIdx < groups.size())
                    {
                        const auto& pGroupMetadata = groups[groupIdx];

                        if(nullptr != pGroupMetadata &&
                            ( false == pGroupMetadata->plotViewIDParameters.plotViewIDParametersMap.empty() ) )
                        {
                            res.first = true; // we got the UML data
                            res.second = &pGroupMetadata->plotViewIDParameters.plotViewIDParametersMap;
                        }
                    }

                    return res;
                };

                auto plotViewDataRes = isPlotViewData();

                if(true == plotViewDataRes.first) // if we have a plot view data
                {
                    auto fillInFieldRanges = [this, &match](tPlotViewDataItem& plotViewDataItem)
                    {
                        //let's grab groups content
                        for(auto it = fieldRanges.begin(); it != fieldRanges.end(); ++it)
                        {
                            const auto& fieldRange = *it;

                            bool insideRange = (match.range.from >= fieldRange.from || match.range.to >= fieldRange.from) &&
                                               (match.range.from <= fieldRange.to || match.range.to <= fieldRange.to);

                            if(true == insideRange) // if group is even partially inside the range
                            {
                                tStringCoverageItem stringCoverageItem;
                                stringCoverageItem.range = tIntRange( std::max(fieldRange.from, match.range.from) - fieldRange.from,
                                                                     std::min( fieldRange.to, match.range.to ) - fieldRange.from );
                                stringCoverageItem.bAddSeparator = match.range.to > fieldRange.to;
                                plotViewDataItem.stringCoverageMap[it.key()] = stringCoverageItem;
                            }
                        }
                    };

                    auto fillInParameters = [this](tPlotViewDataItem& plotViewDataItem, const tPlotViewIDParametersMap::value_type& plotViewIDParametersMap)
                    {
                        plotViewDataItem.optColor = plotViewIDParametersMap.second.optColor;
                        plotViewDataItem.pPlotViewGroupName = plotViewIDParametersMap.second.pPlotViewGroupName;
                        plotViewDataItem.plotViewSplitParameters = plotViewIDParametersMap.second.plotViewSplitParameters;
                        pPlotViewInfo->plotViewDataMap[plotViewIDParametersMap.first].push_back(plotViewDataItem);
                    };

                    for(const auto& plotViewIDParametersItem : *(plotViewDataRes.second))
                    {
                        tPlotViewDataItem plotViewDataItem;

                        fillInFieldRanges(plotViewDataItem);
                        fillInParameters(plotViewDataItem, plotViewIDParametersItem);
                    }
                }

                return true;
            };

            pMatchesTree->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);
        }
    }

    result.pTreeItem = pMatchesTree;

    return result;
}

//tFoundMatch

tFoundMatch::tFoundMatch():
matchStr(),
range(0,0),
idx(0)
{}

tFoundMatch::tFoundMatch( const QString& matchStr_,
                          const tIntRange& range_,
                          const int& idx_):
matchStr(matchStr_),
range(range_),
idx(idx_)
{}

bool tFoundMatch::operator< (const tFoundMatch& rhs) const
{
    Q_UNUSED(rhs);
    return false;
}

//tFoundMatches
tFoundMatches::tFoundMatches():
timeStamp(0u),
msgId(0),
msgSizeBytes(0u)
{}

tFoundMatches::tFoundMatches(const std::uint32_t& msgSizeBytes_,
                             const unsigned int& timeStamp_,
                             const tMsgId& msgId_):
timeStamp(timeStamp_),
msgId(msgId_),
msgSizeBytes(msgSizeBytes_)
{}

//tFoundMatchesPackItem
tFoundMatchesPackItem::tFoundMatchesPackItem()
{

}

tFoundMatchesPackItem::tFoundMatchesPackItem( tItemMetadata&& itemMetadata_,
                                              tFoundMatches&& foundMatches_ ):
  mItemMetadata(std::move(itemMetadata_)),
  mFoundMatches(std::move(foundMatches_))
{
}

const tItemMetadata& tFoundMatchesPackItem::getItemMetadata() const
{
    return mItemMetadata;
}

const tFoundMatches& tFoundMatchesPackItem::getFoundMatches() const
{
    return mFoundMatches;
}

tItemMetadata& tFoundMatchesPackItem::getItemMetadataWriteable()
{
    return mItemMetadata;
}

tFoundMatches& tFoundMatchesPackItem::getFoundMatchesWriteable()
{
    return mFoundMatches;
}

//tFoundMatchesPack
tFoundMatchesPack::tFoundMatchesPack():
matchedItemVec()
{

}

tFoundMatchesPack::tFoundMatchesPack( const tFoundMatchesPackItemVec& matchedItemVec_ ):
matchedItemVec(matchedItemVec_)
{

}

int tFoundMatchesPack::findRowByMsgId(const tMsgId& msgIdToFind) const
{
    int left = 0;
    int right = static_cast<int>(matchedItemVec.size()) - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        tMsgId midMsgId = matchedItemVec[mid]->getItemMetadata().msgId;

        if (midMsgId == msgIdToFind)
        {
            return mid;
        }
        else if (midMsgId < msgIdToFind)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    // Item not found
    return -1;
}

QString getName(eSearchResultColumn val)
{
    QString result;

    switch(val)
    {
        case eSearchResultColumn::UML_Applicability:
        {
            result = "UML";
        }
            break;
        case eSearchResultColumn::PlotView_Applicability:
        {
            result = "Plot";
        }
            break;
        case eSearchResultColumn::Apid:
        {
            result = "Apid";
        }
            break;
        case eSearchResultColumn::Args:
        {
            result = "Args";
        }
            break;
        case eSearchResultColumn::Ctid:
        {
            result = "Ctid";
        }
            break;
        case eSearchResultColumn::Last:
        {
            result = "Last";
        }
            break;
        case eSearchResultColumn::Mode:
        {
            result = "Mode";
        }
            break;
        case eSearchResultColumn::Time:
        {
            result = "Time";
        }
            break;
        case eSearchResultColumn::Type:
        {
            result = "Type";
        }
            break;
        case eSearchResultColumn::Count:
        {
            result = "Count";
        }
            break;
        case eSearchResultColumn::Ecuid:
        {
            result = "Ecuid";
        }
            break;
        case eSearchResultColumn::Index:
        {
            result = "Index";
        }
            break;
        case eSearchResultColumn::Subtype:
        {
            result = "Subtype";
        }
            break;
        case eSearchResultColumn::SessionId:
        {
            result = "SessionId";
        }
            break;
        case eSearchResultColumn::Timestamp:
        {
            result = "Timestamp";
        }
        break;
        case eSearchResultColumn::Payload:
        {
            result = "Payload";
        }
        break;
    }

    return result;
}

QString getName(ePatternsColumn val)
{
    QString result;

    switch(val)
    {
        case ePatternsColumn::Alias:
        {
            result = "Full alias";
        }
            break;
        case ePatternsColumn::AliasTreeLevel:
        {
            result = "Alias";
        }
        break;
        case ePatternsColumn::Default:
        {
            result = "Def.";
        }
            break;
        case ePatternsColumn::Combine:
        {
            result = "Comb.";
        }
            break;
        case ePatternsColumn::Regex:
        {
            result = "Regex";
        }
            break;
        case ePatternsColumn::AfterLastVisible:
        {
            result = "After last visible";
        }
            break;
        case ePatternsColumn::RowType:
        {
            result = "Row type";
        }
            break;
        case ePatternsColumn::IsFiltered:
        {
            result = "Is filtered";
        }
            break;
        case ePatternsColumn::Last:
        {
            result = "Last";
        }
            break;
    }

    return result;
}

ePatternsColumn toPatternsColumn(int column)
{
    return static_cast<ePatternsColumn>(column);
}

//tGroupedViewMetadata
tGroupedViewMetadata::tGroupedViewMetadata():
timeStamp(0u),
msgId(0)
{
}

bool tGroupedViewMetadata::operator< (const tGroupedViewMetadata& rhs) const
{
    return msgId < rhs.msgId;
}

bool tGroupedViewMetadata::operator== (const tGroupedViewMetadata& rhs) const
{
    return msgId == rhs.msgId;
}

tGroupedViewMetadata::tGroupedViewMetadata( const unsigned int timeStamp_, const tMsgId& msgId_ ):
timeStamp(timeStamp_),
msgId(msgId_)
{}

QString getName(eGroupedViewColumn val)
{
    QString result;

    switch(val)
    {
        case eGroupedViewColumn::SubString:
        {
            result = "Found sub-string";
        }
            break;
        case eGroupedViewColumn::Messages:
        {
            result = "Msg-s";
        }
            break;
        case eGroupedViewColumn::MessagesPercantage:
        {
            result = "Msg-s, %";
        }
            break;
        case eGroupedViewColumn::MessagesPerSecondAverage:
        {
            result = "Msg-s/sec, av.";
        }
            break;
        case eGroupedViewColumn::Payload:
        {
            result = "Payload";
        }
            break;
        case eGroupedViewColumn::PayloadPercantage:
        {
            result = "Payload, %";
        }
            break;
        case eGroupedViewColumn::PayloadPerSecondAverage:
        {
            result = "Payload, b/sec, av.";
        }
            break;
        case eGroupedViewColumn::AfterLastVisible:
        {
            result = "Last";
        }
            break;
        case eGroupedViewColumn::Metadata:
        {
            result = "Metadata";
        }
            break;
        case eGroupedViewColumn::Last:
        {
            result = "Last";
        }
            break;
    }

    return result;
}

eGroupedViewColumn toGroupedViewColumn(int column)
{
    return static_cast<eGroupedViewColumn>(column);
}

QString getName(eRegexFiltersColumn val)
{
    QString result;

    switch(val)
    {
        case eRegexFiltersColumn::Index:
        {
            result = "Index";
        }
        break;
        case eRegexFiltersColumn::ItemType:
        {
            result = "Item type";
        }
            break;
        case eRegexFiltersColumn::Value:
        {
            result = "Value";
        }
            break;
        case eRegexFiltersColumn::AfterLastVisible:
        {
            result = "Last";
        }
            break;
        case eRegexFiltersColumn::Color:
        {
            result = "Color";
        }
            break;
        case eRegexFiltersColumn::Range:
        {
            result = "Range";
        }
            break;
        case eRegexFiltersColumn::RowType:
        {
            result = "Row type";
        }
            break;
        case eRegexFiltersColumn::IsFiltered:
        {
            result = "Is filtered";
        }
            break;
        case eRegexFiltersColumn::GroupName:
        {
            result = "Group name";
        }
            break;
        case eRegexFiltersColumn::GroupSyntaxType:
        {
            result = "Group Syntax type";
        }
            break;
        case eRegexFiltersColumn::GroupIndex:
        {
            result = "Group index";
        }
            break;
        case eRegexFiltersColumn::Last:
        {
            result = "Last";
        }
            break;
    }

    return result;
}

eRegexFiltersColumn toRegexFiltersColumn(int column)
{
    return static_cast<eRegexFiltersColumn>(column);
}

QString getName(eRegexFiltersRowType val)
{
    QString result;

    switch(val)
    {
        case eRegexFiltersRowType::Text:
        {
            result = "Text";
        }
        break;
        case eRegexFiltersRowType::VarGroup:
        {
            result = "Variable";
        }
            break;
        case eRegexFiltersRowType::NonVarGroup:
        {
            result = "Group";
        }
            break;
    }

    return result;
}

QString getName(eRequestState field)
{
    QString result;

    switch(field)
    {
        case eRequestState::ERROR_STATE:
        {
            result = "Error";
        }
            break;
        case eRequestState::PROGRESS:
        {
            result = "Progress";
        }
            break;
        case eRequestState::SUCCESSFUL:
        {
            result = "Successful";
        }
            break;
    }

    return result;
}

tCacheSizeB MBToB( const tCacheSizeMB& mb )
{
    return static_cast<tCacheSizeB>(mb) * 1024u * 1024u;
}

tCacheSizeMB BToMB( const tCacheSizeB& b )
{
    return static_cast<tCacheSizeMB>(b / 1024u / 1024u);
}

// tHighlightingGradient
tHighlightingGradient::tHighlightingGradient():
from(), to(), numberOfColors(0)
{

}

tHighlightingGradient::tHighlightingGradient(const QColor& from_, const QColor& to_, int numberOfColors_):
from(from_), to(to_), numberOfColors(numberOfColors_)
{

}

bool tHighlightingGradient::operator==(const tHighlightingGradient& rhs) const
{
    return (from == rhs.from && to == rhs.to && numberOfColors == rhs.numberOfColors);
}

bool tHighlightingGradient::operator!=(const tHighlightingGradient& rhs) const
{
    return !( *this == rhs );
}

tRegexScriptingMetadataItemPtr parseRegexGroupName( const QString& groupName,
                                                    bool bParseUMLData,
                                                    bool bParsePlotViewData,
                                                    bool /* bParseGroupedViewData */ )
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList splitGroupName = groupName.split(sRegexScriptingDelimiter,
                                                 QString::SplitBehavior::SkipEmptyParts,
                                                 Qt::CaseInsensitive);
#else
    QStringList splitGroupName = groupName.split(sRegexScriptingDelimiter,
                                                 Qt::SkipEmptyParts,
                                                 Qt::CaseInsensitive);
#endif

    tRegexScriptingMetadataItem result;

    // static const section
    static const QString rgbRegexStr( QString("%1([0-9]{1,3})_([0-9]{1,3})_([0-9]{1,3})").arg(sRGBPrefix) );
    static const QRegularExpression rgbRegex(rgbRegexStr, QRegularExpression::CaseInsensitiveOption);
    static const QString varRegexStr( QString("%1([\\w\\d]+)").arg(sVARPrefix) );
    static const QRegularExpression varRegex(varRegexStr, QRegularExpression::CaseInsensitiveOption);

    auto normalizeRGBItem = [](const int& rgbItem)->int
    {
        int normalizationResult = 0;

        if(rgbItem > 255)
        {
            normalizationResult = 255;
        }
        else if( rgbItem < 0 )
        {
            normalizationResult = 0;
        }
        else
        {
            normalizationResult = rgbItem;
        }

        return normalizationResult;
    };

    QOptionalColor optColor;
    optColor.isSet = false;

    tOptionalVarName optVarName;
    optVarName.first = false;

    tOptional_UML_ID optUML_ID;

    tPlotViewIDParameters plotViewIDParameters;

    for( const auto& groupNamePart : splitGroupName )
    {
        // parse color
        if(false == optColor.isSet)
        {
            QRegularExpressionMatch rgbMatch = rgbRegex.match(groupNamePart);

            if(rgbMatch.hasMatch())
            {
                bool scriptedColorFound = rgbMatch.lastCapturedIndex() == 3                    // if all 3 groups found
                                       && rgbMatch.capturedEnd(3) == groupNamePart.length();   // and if last group's ending index is equal to the group name's length

                if(scriptedColorFound)
                {
                    bool bRedParsed = false;
                    int red = 0;
                    bool bGreenParsed = false;
                    int green = 0;
                    bool bBlueParsed = false;
                    int blue = 0;

                    for (int i = 0; i <= rgbMatch.lastCapturedIndex(); ++i) // let's iterate over the groups
                    {
                        const auto& matchItem = rgbMatch.captured(i);

                        if(i>0) // 0 index stands for the whole matched string
                        {
                            if(0 != matchItem.size()) // if match is non-zero string
                            {
                                switch (i)
                                {
                                    case 1: // R
                                    {
                                        red = matchItem.toInt( &bRedParsed, 10 );

                                        if(true == bRedParsed)
                                        {
                                            red = normalizeRGBItem(red);
                                        }
                                    }
                                        break;
                                    case 2: // G
                                    {
                                        green = matchItem.toInt( &bGreenParsed, 10 );

                                        if(true == bGreenParsed)
                                        {
                                            green = normalizeRGBItem(green);
                                        }
                                    }
                                        break;
                                    case 3: // B
                                    {
                                        blue = matchItem.toInt( &bBlueParsed, 10 );

                                        if(true == bBlueParsed)
                                        {
                                            blue = normalizeRGBItem(blue);
                                        }
                                    }
                                        break;

                                    default:
                                        break;
                                }
                            }
                        }
                    }

                    if( true == bRedParsed
                     && true == bGreenParsed
                     && true == bBlueParsed)
                    {
                        optColor.isSet = true;
                        optColor.color_code = QColor(red, green, blue).rgb();
                    }
                }
            }
            else
            {
                QString lowerGroupNamePart = groupNamePart.toLower();

                const auto& colorsMap = getColorsMap();

                auto foundColor = colorsMap.find(lowerGroupNamePart);

                if(colorsMap.end() != foundColor)
                {
                    optColor.isSet = true;
                    optColor.color_code = foundColor->second.rgb();
                }
            }
        }

        // parse var
        if(false == optVarName.first)
        {
            QRegularExpressionMatch varMatch = varRegex.match(groupNamePart);

            if(varMatch.hasMatch())
            {
                bool scriptedVarFound = varMatch.lastCapturedIndex() == 1; // if 1 group found

                if(true == scriptedVarFound)
                {
                    optVarName.first = true;
                    optVarName.second = varMatch.captured(1);
                }
            }
        }

        if(true == bParseUMLData)
        {
            // parse UML data
            auto createUMLRegexStr = [](  ) -> QString
            {
                QString resultRegex("^(");

                auto finalIter = s_UML_IDs_Map.end();
                --finalIter;

                for( auto it = s_UML_IDs_Map.begin(); it != s_UML_IDs_Map.end(); ++it )
                {
                    const auto& UML_IDs_MapItem = *it;

                    resultRegex.append(UML_IDs_MapItem.second.id_str);

                    if(it != finalIter)
                    {
                        resultRegex.append("|");
                    }
                }

                resultRegex.append(QString(")[%1]{0,1}([\\w\\d]*)$").arg(s_UML_ALIAS_DELIMITER));

                return resultRegex;
            };

            static const QString UMLRegexStr = createUMLRegexStr();
            static const QRegularExpression UMLRegex(UMLRegexStr, QRegularExpression::CaseInsensitiveOption);

            QRegularExpressionMatch varMatch = UMLRegex.match(groupNamePart);

            if(varMatch.hasMatch())
            {
                bool b_UML_ID_Found = varMatch.lastCapturedIndex() >= 1; // if 1 or more groups found

                if(true == b_UML_ID_Found)
                {
                    eUML_ID UML_ID = eUML_ID::UML_EVENT; // any value
                    bool bUML_ID_Parsed = parseUMLIDFromString( varMatch.captured(1), UML_ID );

                    if(true == bUML_ID_Parsed)
                    {
                        const auto& key = UML_ID;
                        auto& pValue = optUML_ID.optional_UML_IDMap[key].pUML_Custom_Value;

                        if(varMatch.lastCapturedIndex() == 2) // if 2 groups found
                        {
                            QString capturedString = varMatch.captured(2);
                            bool b_UML_Custom_Value_Found = false == capturedString.isEmpty(); // if 2 groups found

                            if(true == b_UML_Custom_Value_Found)
                            {
                                pValue = std::make_shared<QString>(capturedString);
                            }
                        }
                    }
                }
            }
        }

        if(true == bParsePlotViewData)
        {
            // parse plot data
            static const QRegularExpression plotViewRegex = createPlotViewRegex();

            QRegularExpressionMatch varMatch = plotViewRegex.match(groupNamePart);

            if(varMatch.hasMatch())
            {
                bool b_PlotView_ID_Found = varMatch.lastCapturedIndex() >= 1; // if 1 or more groups found

                if(true == b_PlotView_ID_Found)
                {
                    ePlotViewID plotView_ID = ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE; // any value
                    bool bPlotView_ID_Parsed = parsePlotViewIDFromString( varMatch.captured(1), plotView_ID );

                    if(true == bPlotView_ID_Parsed)
                    {
                        const auto& key = plotView_ID;
                        plotViewIDParameters.plotViewIDParametersMap[key].pPlotViewGroupName = std::make_shared<QString>(groupName);

                        if(varMatch.lastCapturedIndex() == 2) // if 2 groups found
                        {
                            QString parameters = varMatch.captured(2);
                            bool b_PlotView_Parameters_Found = false == parameters.isEmpty(); // if 2 groups found

                            if(true == b_PlotView_Parameters_Found)
                            {
                                auto splitParameters = splitPlotViewParameters(parameters);
                                for(const auto& splitParameter : splitParameters)
                                {
                                        plotViewIDParameters.plotViewIDParametersMap[key].plotViewSplitParameters.push_back(std::make_shared<QString>(splitParameter));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for(auto& pair : plotViewIDParameters.plotViewIDParametersMap)
    {
        pair.second.optColor = optColor;
    }

    tRegexScriptingMetadataItemPtr pItem = std::make_shared<tRegexScriptingMetadataItem>();
    pItem->highlightingColor = optColor;
    pItem->varName = optVarName;
    pItem->optionalUML_ID = optUML_ID;
    pItem->plotViewIDParameters = plotViewIDParameters;

    return pItem;
}

tGroupedViewIdx parseRegexGroupedViewIndices( const QString& groupName,
                                              bool bParseGroupedViewData )
{
    tGroupedViewIdx result = sInvalidGroupedViewIdx;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QStringList splitGroupName = groupName.split(sRegexScriptingDelimiter,
                                                 QString::SplitBehavior::SkipEmptyParts,
                                                 Qt::CaseInsensitive);
#else
    QStringList splitGroupName = groupName.split(sRegexScriptingDelimiter,
                                                 Qt::SkipEmptyParts,
                                                 Qt::CaseInsensitive);
#endif

    static const QString groupedViewIdxStr( QString("^%1_([\\d]+)$").arg(sGroupedViewPrefix) );
    static const QRegularExpression groupedViewIdxRegex(groupedViewIdxStr, QRegularExpression::CaseInsensitiveOption);
    static const QString groupedViewStr( QString("^%1$").arg(sGroupedViewPrefix) );
    static const QRegularExpression groupedViewRegex(groupedViewStr, QRegularExpression::CaseInsensitiveOption);

    if(bParseGroupedViewData)
    {
        for( const auto& groupNamePart : splitGroupName )
        {
            // parse grouped view data
            QRegularExpressionMatch groupedViewIdxMatch = groupedViewIdxRegex.match(groupNamePart);

            if(groupedViewIdxMatch.hasMatch())
            {
                bool groupedViewIdxFound = groupedViewIdxMatch.lastCapturedIndex() == 1; // if 1 group found

                if(true == groupedViewIdxFound)
                {
                    bool bGroupIdxParsed = false;
                    int groupIdx = 0;
                    groupIdx = groupedViewIdxMatch.captured(1).toInt(&bGroupIdxParsed);

                    if(bGroupIdxParsed)
                    {
                        result = groupIdx;
                    }
                }
            }
            else
            {
                QRegularExpressionMatch groupedViewMatch = groupedViewRegex.match(groupNamePart);

                if(groupedViewMatch.hasMatch())
                {
                    result = sMaxGroupedViewIdx;
                }
            }
        }
    }

    return result;
}

//tRegexScriptingMetadata
bool tRegexScriptingMetadata::parse(const QRegularExpression& regex,
                                    bool bParseUMLData,
                                    bool bParsePlotViewData,
                                    bool bParseGroupedViewData)
{
    bool bResult = true;

    if(true == regex.isValid())
    {
        auto groupNames = regex.namedCaptureGroups();

        int groupCounter = 0;
        typedef std::multimap<tGroupedViewIdx /*specified grouped view ordering*/, int /*regex group index*/> tSortingMap;
        tSortingMap sortingMap;

        for(auto it = groupNames.begin(); it != groupNames.end(); ++it)
        {
            const auto& groupName = *it;

            mItemsVec.push_back(parseRegexGroupName(groupName,
                                                    bParseUMLData,
                                                    bParsePlotViewData,
                                                    bParseGroupedViewData));

            auto parsedIndex = parseRegexGroupedViewIndices(groupName, bParseGroupedViewData);

            if(parsedIndex != sInvalidGroupedViewIdx)
            {
                sortingMap.insert(std::make_pair(parsedIndex, groupCounter));
            }

            ++groupCounter;
        }

        mGroupedViewIndexes.clear();

        groupCounter = 0;
        for(const auto& pair : sortingMap)
        {
            mGroupedViewIndexes.insert(std::make_pair(pair.second, groupCounter));
            ++groupCounter;
        }
    }
    else
    {
        bResult = false;
    }

    return bResult;
}

const tRegexScriptingMetadataItemPtrVec& tRegexScriptingMetadata::getItemsVec() const
{
    return mItemsVec;
}

tRegexScriptingMetadata::tStatusPair
tRegexScriptingMetadata::doesContainConsistentUMLData(bool fillInStringMsg) const
{
    return doesContainConsistentUMLData(fillInStringMsg, tCheckIDs(), true);
}

tRegexScriptingMetadata::tStatusPair
tRegexScriptingMetadata::doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs) const
{
    return doesContainConsistentUMLData(fillInStringMsg, checkIDs, false);
}

tRegexScriptingMetadata::tStatusPair
tRegexScriptingMetadata::doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool bCheckAll) const
{
    std::pair<bool /*status*/, QString /*status description*/> result;

    result.first = false;

    if(true == fillInStringMsg)
    {
        result.second.append("UML parsing result:");
    }

    if(false == bCheckAll && true == checkIDs.empty())
    {
        if(true == fillInStringMsg)
        {
            result.second.append("No groups available. Result is \"false\"");
        }
    }
    else
    {
        tUML_IDs_Map mandatoryUMLElements;

        for(const auto& element : s_UML_IDs_Map)
        {
            if(element.second.id_type == eUML_ID_Type::e_Mandatory)
            {
                mandatoryUMLElements.insert(std::make_pair(element.first, element.second));
            }
        }

        tRegexScriptingMetadataItemPtrVec iterationVec;

        if(true == bCheckAll)
        {
            iterationVec = mItemsVec;
        }
        else
        {
            for(const auto& checkId : checkIDs)
            {
                if(checkId >= 0 && checkId < mItemsVec.size())
                {
                    iterationVec.push_back(mItemsVec[checkId]);
                }
            }
        }

        for(const auto& pElement : iterationVec)
        {
            if(nullptr != pElement)
            {
                if(false == pElement->optionalUML_ID.optional_UML_IDMap.empty())
                {
                    for(const auto& UML_IDItem : pElement->optionalUML_ID.optional_UML_IDMap)
                    {
                        auto foundElement = mandatoryUMLElements.find(UML_IDItem.first);

                        if(foundElement != mandatoryUMLElements.end())
                        {
                            mandatoryUMLElements.erase(foundElement);

                            if(true == mandatoryUMLElements.empty())
                            {
                                    break;
                            }
                        }
                    }
                }
            }
        }

        auto checkRequestType = [&fillInStringMsg, &iterationVec](QString& msg) -> bool
        {
            bool bResult = false;

            // get request type elements
            tUML_IDs_Map requestTypeUMLElements;

            for(const auto& element : s_UML_IDs_Map)
            {
                if(element.second.id_type == eUML_ID_Type::e_RequestType)
                {
                    requestTypeUMLElements.insert(std::make_pair(element.first, element.second));
                }
            }

            auto requestTypeColelctionInitialSize = requestTypeUMLElements.size();

            for(const auto& pElement : iterationVec)
            {
                if(nullptr != pElement)
                {
                    if(false == pElement->optionalUML_ID.optional_UML_IDMap.empty())
                    {
                        for(const auto& UML_IDItem : pElement->optionalUML_ID.optional_UML_IDMap)
                        {
                            auto foundElement = requestTypeUMLElements.find(UML_IDItem.first);

                            if(foundElement != requestTypeUMLElements.end())
                            {
                                requestTypeUMLElements.erase(foundElement);

                                if(true == requestTypeUMLElements.empty())
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if(requestTypeUMLElements.size() < requestTypeColelctionInitialSize) // if at least one request type element was found
            {
                bResult = true;
            }
            else
            {
                if(true == fillInStringMsg)
                {
                    QString subMsg;

                    bool bFirstIteration = true;

                    for(const auto& element : requestTypeUMLElements)
                    {
                        if(true == bFirstIteration)
                        {
                            bFirstIteration = false;
                        }
                        else
                        {
                            subMsg.append("|");
                        }

                        subMsg.append(QString("%1").arg(getUMLIDAsString(element.first)));
                    }

                    msg.append(QString(" <Request type elements (%1) not found>").arg(subMsg));
                }
            }

            return bResult;
        };

        if(true == mandatoryUMLElements.empty()) // if all mandatory elements were found
        {
            result.first = checkRequestType(result.second); // proceed with analysis
        }
        else // otherwise
        {
            if(true == fillInStringMsg)
            {
                // collect warning string
                for(const auto& element : mandatoryUMLElements)
                {
                    result.second.append(QString(" <Mandatory element %1 not found>").arg(getUMLIDAsString(element.first)));
                }
            }

            static_cast<void>(checkRequestType(result.second));
        }
    }

    return result;
}

bool tRegexScriptingMetadata::doesContainAnyUMLGroup() const
{
    bool bResult = false;

    for(const auto& element : mItemsVec)
    {
        if(false == element->optionalUML_ID.optional_UML_IDMap.empty())
        {
            for(const auto& UML_IDItem : element->optionalUML_ID.optional_UML_IDMap)
            {
                auto foundElement = s_UML_IDs_Map.find(UML_IDItem.first);

                if(foundElement != s_UML_IDs_Map.end()) // match found
                {
                    bResult = true;
                    break;
                }
            }
        }
    }

    return bResult;
}

tRegexScriptingMetadata::tStatusPair
tRegexScriptingMetadata::doesContainConsistentPlotViewData(bool fillInStringMsg, bool checkParameters) const
{
    return doesContainConsistentPlotViewData(fillInStringMsg, tCheckIDs(), true, checkParameters);
}

tRegexScriptingMetadata::tStatusPair
tRegexScriptingMetadata::doesContainConsistentPlotViewData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool checkParameters) const
{
    return doesContainConsistentPlotViewData(fillInStringMsg, checkIDs, false, checkParameters);
}

tRegexScriptingMetadata::tStatusPair
tRegexScriptingMetadata::doesContainConsistentPlotViewData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool bCheckAll, bool checkParameters) const
{
    std::pair<bool /*status*/, QString /*status description*/> result;
    result.first = false;

    if(true == fillInStringMsg)
    {
        result.second.append("Plot view parsing result:");
    }

    if(false == bCheckAll && true == checkIDs.empty())
    {
        if(true == fillInStringMsg)
        {
            result.second.append("No groups available. Result is \"false\"");
        }
    }
    else
    {
        tRegexScriptingMetadataItemPtrVec iterationVec;

        if(true == bCheckAll)
        {
            iterationVec = mItemsVec;
        }
        else
        {
            for(const auto& checkId : checkIDs)
            {
                if(checkId >= 0 && checkId < mItemsVec.size())
                {
                    iterationVec.push_back(mItemsVec[checkId]);
                }
            }
        }

        auto checkMandatoryElements = [&iterationVec, &fillInStringMsg]( ePlotViewIDType parameterType, QString& msg ) -> bool
        {
            bool bResult = false;

            tPlotViewIDsMap mandatoryElements;

            for(const auto& element : sPlotViewIDsMap)
            {
                if(element.second.id_type == parameterType)
                {
                    mandatoryElements.insert(std::make_pair(element.first, element.second));
                }
            }

            for(const auto& pElement : iterationVec)
            {
                if(nullptr != pElement)
                {
                    if(false == pElement->plotViewIDParameters.plotViewIDParametersMap.empty())
                    {
                        for(const auto& PlotView_IDItem : pElement->plotViewIDParameters.plotViewIDParametersMap)
                        {
                            auto foundElement = mandatoryElements.find(PlotView_IDItem.first);

                            if(foundElement != mandatoryElements.end())
                            {
                                mandatoryElements.erase(foundElement);

                                if(true == mandatoryElements.empty())
                                {
                                    bResult = true;
                                }
                            }
                        }
                    }
                }
            }

            if(false == mandatoryElements.empty())
            {
                bResult = false;

                if(true == fillInStringMsg)
                {
                    // collect warning string
                    for(const auto& element : mandatoryElements)
                    {
                        msg.append(QString(" <Mandatory element %1 not found>").arg(getPlotIDAsString(element.first)));
                    }
                }
            }

            return bResult;
        };

        auto checkAllParameters = [&iterationVec, &fillInStringMsg](QString& msg) -> bool
        {
            bool bResult = true;

            for(const auto& pElement : iterationVec)
            {
                if(nullptr != pElement)
                {
                    if(false == pElement->plotViewIDParameters.plotViewIDParametersMap.empty())
                    {
                        bool bCheckParametersTmp = checkPlotViewParameters(msg, fillInStringMsg, pElement->plotViewIDParameters.plotViewIDParametersMap);

                        if(true == bResult)
                        {
                            bResult = bCheckParametersTmp;
                        }
                    }
                }
            }

            return bResult;
        };

        auto uniqueAvailableAxisTypes = getUniqueAvailableAxisTypes();

        if(uniqueAvailableAxisTypes.find(ePlotViewAxisType::e_GANTT) != uniqueAvailableAxisTypes.end())
        {
            bool bMandatoryGanttParametersCheck = checkMandatoryElements(ePlotViewIDType::e_Mandatory_Gantt, result.second);
            bool bMandatoryNonGanttParametersCheck = true;

            if(uniqueAvailableAxisTypes.size() > 1)
            {
                bMandatoryNonGanttParametersCheck = checkMandatoryElements(ePlotViewIDType::e_Mandatory_Non_Gantt, result.second);
            }

            if(true == checkParameters)
            {
                bool bCheckParameters = checkAllParameters(result.second);
                result.first = ( bMandatoryGanttParametersCheck && bCheckParameters ) || ( bMandatoryNonGanttParametersCheck && bCheckParameters );
            }
            else
            {
                result.first = bMandatoryGanttParametersCheck || bMandatoryNonGanttParametersCheck;
            }
        }
        else if(false == uniqueAvailableAxisTypes.empty())
        {
            bool bMandatoryNonGanttParametersCheck = checkMandatoryElements(ePlotViewIDType::e_Mandatory_Non_Gantt, result.second);

            if(true == checkParameters)
            {
                bool bCheckParameters = checkAllParameters(result.second);
                result.first = bMandatoryNonGanttParametersCheck && bCheckParameters;
            }
            else
            {
                result.first = bMandatoryNonGanttParametersCheck;
            }
        }
    }

    return result;
}

bool tRegexScriptingMetadata::doesContainAnyPlotViewGroup() const
{
    bool bResult = false;

    for(const auto& element : mItemsVec)
    {
        if(false == element->plotViewIDParameters.plotViewIDParametersMap.empty())
        {
            for(const auto& UML_IDItem : element->plotViewIDParameters.plotViewIDParametersMap)
            {
                auto foundElement = sPlotViewIDsMap.find(UML_IDItem.first);

                if(foundElement != sPlotViewIDsMap.end()) // match found
                {
                    bResult = true;
                    break;
                }
            }
        }
    }

    return bResult;
}

const tGroupedViewIndices& tRegexScriptingMetadata::getGroupedViewIndices() const
{
    return mGroupedViewIndexes;
}

std::set<ePlotViewAxisType> sAllAxisTypes =
{
    ePlotViewAxisType::e_GANTT,
    ePlotViewAxisType::e_LINEAR,
    ePlotViewAxisType::e_POINT
};

std::set<ePlotViewAxisType> tRegexScriptingMetadata::getUniqueAvailableAxisTypes() const
{
    std::set<ePlotViewAxisType> result;

    for(const auto& item : mItemsVec)
    {
        const auto& optionalPlot_IDMap = item->plotViewIDParameters.plotViewIDParametersMap;
        auto foundItem = optionalPlot_IDMap.find(ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE);
        if(foundItem != optionalPlot_IDMap.end())
        {
            ePlotViewAxisType axisType = ePlotViewAxisType::e_LINEAR;
            auto parameterIndex = getPlotViewParameterIndex(ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE, "axisRectType");

            if(parameterIndex >= 0 && parameterIndex <= static_cast<int32_t>(foundItem->second.plotViewSplitParameters.size() - 1) )
            {
                assert(foundItem->second.plotViewSplitParameters[parameterIndex] != nullptr);
                if(true == parseAxisTypeFromString(*foundItem->second.plotViewSplitParameters[parameterIndex], axisType))
                {
                    result.insert(axisType);
                    if(result == sAllAxisTypes)
                    {
                        break;
                    }
                }
            }
        }
    }

    if(true == result.empty())
    {
        result.insert(ePlotViewAxisType::e_LINEAR);
    }

    return result;
}
//////////////////////////////////////////////////////////

Qt::CheckState V_2_CS( const tDataItem& val )
{
    return val.get<Qt::CheckState>();
}

Qt::CheckState V_2_CS( const QVariant& val )
{
    return static_cast<Qt::CheckState>( val.value<int>() );
}

QVariant toQVariant(const tDataItem& item)
{
    QVariant result;

    // holds QString, Qt::CheckState, ePatternsRowType, bool, int, tGroupedViewMetadata

    if(item.index() == tDataItem::index_of<QString>())
    {
        result.setValue(item.get<QString>());
    }
    else if(item.index() == tDataItem::index_of<Qt::CheckState>())
    {
        result.setValue(item.get<Qt::CheckState>());
    }
    else if(item.index() == tDataItem::index_of<ePatternsRowType>())
    {
        result.setValue(item.get<ePatternsRowType>());
    }
    else if(item.index() == tDataItem::index_of<bool>())
    {
        result.setValue(item.get<bool>());
    }
    else if(item.index() == tDataItem::index_of<int>())
    {
        result.setValue(item.get<int>());
    }
    else if(item.index() == tDataItem::index_of<double>())
    {
        result.setValue(item.get<double>());
    }
    else if(item.index() == tDataItem::index_of<tGroupedViewMetadata>())
    {
        result.setValue(item.get<tGroupedViewMetadata>());
    }
    else if(item.index() == tDataItem::index_of<tIntRange>())
    {
        result.setValue(item.get<tIntRange>());
    }
    else if(item.index() == tDataItem::index_of<tFoundMatch*>())
    {
        result.setValue(item.get<const tFoundMatch*>());
    }
    else if(item.index() == tDataItem::index_of<tColorWrapper>())
    {
        result.setValue(item.get<tColorWrapper>());
    }
    else if(item.index() == tDataItem::index_of<tIntRange>())
    {
        result.setValue(item.get<tIntRange>());
    }
    else if(item.index() == tDataItem::index_of<eRegexFiltersRowType>())
    {
        result.setValue(item.get<eRegexFiltersRowType>());
    }
    else if(item.index() == tDataItem::index_of<tQStringPtrWrapper>())
    {
        result.setValue(*item.get<tQStringPtrWrapper>().pString);
    }

    return result;
}

tDataItem toRegexDataItem(const QVariant& variant, const eRegexFiltersColumn& column)
{
    tDataItem result;

    switch(column)
    {
        case eRegexFiltersColumn::Index:
        {
            result = variant.value<int>();
        }
        break;
        case eRegexFiltersColumn::ItemType:
        case eRegexFiltersColumn::Value:
        {
            result = variant.value<QString>();
        }
            break;
        case eRegexFiltersColumn::AfterLastVisible:
        {
            result = QString();
        }
            break;
        case eRegexFiltersColumn::Color:
        {
            result = variant.value<tColorWrapper>();
        }
            break;
        case eRegexFiltersColumn::Range:
        {
            result = variant.value<tIntRange>();
        }
            break;
        case eRegexFiltersColumn::RowType:
        {
            result = variant.value<eRegexFiltersRowType>();
        }
            break;
        case eRegexFiltersColumn::IsFiltered:
        {
            result = variant.value<bool>();
        }
            break;
        case eRegexFiltersColumn::GroupName:
        {
            result = variant.value<QString>();
        }
            break;
        case eRegexFiltersColumn::GroupSyntaxType:
        case eRegexFiltersColumn::GroupIndex:
        {
            result = variant.value<int>();
        }
            break;
        case eRegexFiltersColumn::Last:
        {
            result = QString();
        }
            break;
    }

    return result;
}

bool tColorWrapper::operator< ( const tColorWrapper& rhs ) const
{
    bool bResult;

    if(optColor.isSet < rhs.optColor.isSet)
    {
        bResult = true;
    }
    else
    {
        if( qRed(optColor.color_code) < qRed(rhs.optColor.color_code) )
        {
            bResult = true;
        }
        else if( qRed(optColor.color_code) > qRed(rhs.optColor.color_code) )
        {
            bResult = false;
        }
        else
        {
            if( qGreen(optColor.color_code) < qGreen(rhs.optColor.color_code) )
            {
                bResult = true;
            }
            else if( qGreen(optColor.color_code) > qGreen(rhs.optColor.color_code) )
            {
                bResult = false;
            }
            else
            {
                if( qBlue(optColor.color_code) < qBlue(rhs.optColor.color_code) )
                {
                    bResult = true;
                }
                else
                {
                    bResult = false;
                }
            }
        }
    }

    return bResult;
}

bool tColorWrapper::operator== ( const tColorWrapper& rhs ) const
{
    return optColor == rhs.optColor;
}

QString rgb2hex(const QColor& color, bool with_head)
{
    QString result;
    QTextStream ss(&result);

    if (with_head)
    {
        ss << "#";
    }

    if(color.red() == 0 && color.green() != 0)
    {
        ss << "00";
    }

    if(color.red() == 0 && color.green() == 0 && color.red() != 0)
    {
        ss << "0000";
    }

    if(color.red() == 0 && color.green() == 0 && color.red() == 0)
    {
        ss << "000000";
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ss << hex << (color.red() << 16 | color.green() << 8 | color.blue() );
#else
    ss << Qt::hex << (color.red() << 16 | color.green() << 8 | color.blue() );
#endif

    return result;
}

QString addRegexOptions( const QString& regex )
{
    QString result;
    result.append(s_Regex_options).append(regex);
    return result;
}

int getRegexOptionsCharSize()
{
    return s_Regex_options.size();
}

QString getFormattedRegexError(const QRegularExpression& regex)
{
    return QString().append(QString("<col:%1> %2").arg(getRegexErrorColumn(regex)).arg(regex.errorString()));
}

int getRegexErrorColumn(const QRegularExpression& regex)
{
    return regex.patternErrorOffset() - getRegexOptionsCharSize();
}

QString getPayloadWidthAsString(const eSearchViewLastColumnWidthStrategy& val)
{
    QString result;

    switch(val)
    {
        case eSearchViewLastColumnWidthStrategy::eReset:
            result = "Reset user width";
        break;
        case eSearchViewLastColumnWidthStrategy::eFitToContent:
            result = "Fit to content";
        break;
        case eSearchViewLastColumnWidthStrategy::ePreserveUserWidth:
            result = "Preserve user width";
        break;
        case eSearchViewLastColumnWidthStrategy::eLast:
            result = "Last value";
        break;
    }

    return result;
}

QString getPathModeAsString(const ePathMode& val)
{
    QString result;

    switch(val)
    {
        case ePathMode::eUseDefaultPath:
            result = "Use default path";
        break;
        case ePathMode::eUseCustomPath:
            result = "Use custom path";
        break;
        case ePathMode::eUseEnvVar:
            result = "Use environment variable";
        break;
        case ePathMode::eLast:
            result = "Last";
        break;
    }

    return result;
}

bool convertLogFileToDLTV1( const QString& sourceFilePath, const QString& targetFilePath )
{
    bool bResult = true;

    QElapsedTimer timer;

    timer.start();

    QFile sourceFile(sourceFilePath);
    QFile targetFile(targetFilePath);

    if(sourceFile.open(QFile::OpenModeFlag::ReadOnly)) // open source file
    {
        if(targetFile.open(QFile::OpenModeFlag::ReadWrite | QFile::OpenModeFlag::Truncate)) // open target file
        {
            uint8_t messageCounter = 0u;
            uint32_t messageIndex = 0u;

            auto writeDltMsg = [&targetFile,
                                &messageCounter,
                                &messageIndex,
                                &timer](const QString& str)
            {
                uint16_t storageHeaderSize = 0u;

                auto elapsedNsecs = timer.nsecsElapsed();

                auto elapsedSeconds = static_cast<uint32_t>(elapsedNsecs / 1000000000);
                auto elapsedMicroSeconds = static_cast<uint32_t>(elapsedNsecs % 1000000000 / 1000);

                auto currentMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
                auto currentSecsSinceEpoch = currentMSecsSinceEpoch / 1000;
                auto currentMicroSecsSinceEpoch = currentMSecsSinceEpoch % 1000 * 1000;

                QByteArray data;

                {
                    // STORAGE HEADER

                    data.push_back(0x44); // D
                    data.push_back(0x4c); // L
                    data.push_back(0x54); // T
                    data.push_back(0x01);

                    uint8_t currentSecsSinceEpoch_1_byte = currentSecsSinceEpoch & 0xff;
                    uint8_t currentSecsSinceEpoch_2_byte = (currentSecsSinceEpoch >> 8);
                    uint8_t currentSecsSinceEpoch_3_byte = (currentSecsSinceEpoch >> 16);
                    uint8_t currentSecsSinceEpoch_4_byte = (currentSecsSinceEpoch >> 24);
                    data.push_back( currentSecsSinceEpoch_1_byte );
                    data.push_back( currentSecsSinceEpoch_2_byte );
                    data.push_back( currentSecsSinceEpoch_3_byte );
                    data.push_back( currentSecsSinceEpoch_4_byte );

                    uint8_t currentMicroSecsSinceEpoch_1_byte = currentMicroSecsSinceEpoch & 0xff;
                    uint8_t currentMicroSecsSinceEpoch_2_byte = (currentMicroSecsSinceEpoch >> 8);
                    uint8_t currentMicroSecsSinceEpoch_3_byte = (currentMicroSecsSinceEpoch >> 16);
                    uint8_t currentMicroSecsSinceEpoch_4_byte = (currentMicroSecsSinceEpoch >> 24);
                    data.push_back( currentMicroSecsSinceEpoch_1_byte );
                    data.push_back( currentMicroSecsSinceEpoch_2_byte );
                    data.push_back( currentMicroSecsSinceEpoch_3_byte );
                    data.push_back( currentMicroSecsSinceEpoch_4_byte );

                    data.push_back('E');
                    data.push_back('C');
                    data.push_back('U');
                    data.push_back('1');

                    storageHeaderSize = data.size();
                }

                {
                    // HEADER

                    // 0 byte: HTYP
                    char HTYP = 0u;
                    HTYP |= 1UL;            // use extended header
                    HTYP &= ~(1UL << 1);    // Most Significant Byte First
                    HTYP |= 1UL << 2;       // With ECU ID
                    HTYP |= 1UL << 3;       // With Session ID
                    HTYP |= 1UL << 4;       // With Timestamp
                    HTYP |=  ( 1UL << 5 );  // Version
                    HTYP &= ~( 1UL << 6 );  // Version
                    HTYP &= ~( 1UL << 7 );  // Version
                    data.push_back(HTYP);

                    // 1 byte: MCNT
                    data.push_back(messageCounter);
                    ++messageCounter;

                    // 2-3 bytes: LEN - set last, when we can calculate this value.
                    data.push_back('0');
                    data.push_back('0');

                    // 4-7 bytes: ECU
                    data.push_back('E');
                    data.push_back('C');
                    data.push_back('U');
                    data.push_back('1');

                    // 8-11 bytes: SEID
                    uint32_t sessionId = 9999;
                    uint8_t sessionId_1_byte = sessionId & 0xff;
                    uint8_t sessionId_2_byte = (sessionId >> 8);
                    uint8_t sessionId_3_byte = (sessionId >> 16);
                    uint8_t sessionId_4_byte = (sessionId >> 24);
                    data.push_back( sessionId_4_byte );
                    data.push_back( sessionId_3_byte );
                    data.push_back( sessionId_2_byte );
                    data.push_back( sessionId_1_byte );

                    // 12-15 bytes: TMSP. Measured in 0.1 ms
                    uint32_t timestamp = elapsedSeconds * 10000 + elapsedMicroSeconds / 100;
                    uint8_t timestamp_1_byte = timestamp & 0xff;
                    uint8_t timestamp_2_byte = (timestamp >> 8);
                    uint8_t timestamp_3_byte = (timestamp >> 16);
                    uint8_t timestamp_4_byte = (timestamp >> 24);
                    data.push_back( timestamp_4_byte );
                    data.push_back( timestamp_3_byte );
                    data.push_back( timestamp_2_byte );
                    data.push_back( timestamp_1_byte );
                }

                {
                    // EXTENDED HEADER

                    // 0 byte: Message info
                    char messageInfo = 0u;
                    messageInfo |= 1UL;         // Verbose
                    messageInfo &= ~(1UL << 1); // Message Type Info
                    messageInfo &= ~(1UL << 2); // Message Type Info
                    messageInfo &= ~(1UL << 3); // Message Type Info
                    messageInfo &= ~(1UL << 4); // Message Type Info
                    messageInfo |= 1UL << 5;    // Message Type Info
                    messageInfo |= 1UL << 6;    // Message Type Info
                    messageInfo &= ~(1UL << 7); // Message Type Info
                    data.push_back(messageInfo);

                    // 1 byte: Number of arguments
                    data.push_back(1);

                    // 2-5 byte: APID
                    data.push_back('C');
                    data.push_back('O');
                    data.push_back('N');
                    data.push_back('V');

                    // 6-9 byte: CTID
                    data.push_back('I');
                    data.push_back('M');
                    data.push_back('P');
                    data.push_back('1');
                }

                {
                    // PAYLOAD

                    // 0-4 bytes: ARG1 Type Info
                    uint8_t typeInfo_byte_1 = 0u;

                    typeInfo_byte_1 &= ~(1UL);       // Type length
                    typeInfo_byte_1 &= ~(1UL << 1);  // Type length
                    typeInfo_byte_1 &= ~(1UL << 2);  // Type length
                    typeInfo_byte_1 &= ~(1UL << 3);  // Type length
                    typeInfo_byte_1 &= ~(1UL << 4);  // Bool
                    typeInfo_byte_1 &= ~(1UL << 5);  // SINT
                    typeInfo_byte_1 &= ~(1UL << 6);  // UINT
                    typeInfo_byte_1 &= ~(1UL << 7);  // FLOAT

                    uint8_t typeInfo_byte_2 = 0u;

                    typeInfo_byte_2 &= ~(1UL);       // ARAY
                    typeInfo_byte_2 |= (1UL << 1);   // STRG
                    typeInfo_byte_2 &= ~(1UL << 2);  // RAWD
                    typeInfo_byte_2 &= ~(1UL << 3);  // VARI
                    typeInfo_byte_2 &= ~(1UL << 4);  // FIXP
                    typeInfo_byte_2 &= ~(1UL << 5);  // TRAI
                    typeInfo_byte_2 &= ~(1UL << 6);  // STRU
                    typeInfo_byte_2 |= (1UL << 7);   // SCOD

                    uint8_t typeInfo_byte_3 = 0u;

                    typeInfo_byte_3 &= ~(1UL);       // SCOD
                    typeInfo_byte_3 &= ~(1UL << 1);  // SCOD
                    typeInfo_byte_3 &= ~(1UL << 2);  // Reserved
                    typeInfo_byte_3 &= ~(1UL << 3);  // Reserved
                    typeInfo_byte_3 &= ~(1UL << 4);  // Reserved
                    typeInfo_byte_3 &= ~(1UL << 5);  // Reserved
                    typeInfo_byte_3 &= ~(1UL << 6);  // Reserved
                    typeInfo_byte_3 &= ~(1UL << 7);  // Reserved

                    uint8_t typeInfo_byte_4 = 0u;

                    typeInfo_byte_4 &= ~(1UL);       // Reserved
                    typeInfo_byte_4 &= ~(1UL << 1);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 2);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 3);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 4);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 5);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 6);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 7);  // Reserved

                    data.push_back(typeInfo_byte_1);
                    data.push_back(typeInfo_byte_2);
                    data.push_back(typeInfo_byte_3);
                    data.push_back(typeInfo_byte_4);

                    // ARG1 Payload
                    QString strCopy = str;
                    auto maxLength = 8192;
                    if(strCopy.length() >= maxLength )
                    {
                        strCopy = strCopy.mid(0, maxLength - 1);
                    }

                    uint16_t strLength = strCopy.size();
                    uint8_t strLength_low = strLength & 0xff;
                    uint8_t strLength_high = (strLength >> 8);
                    data.push_back(strLength_low);
                    data.push_back(strLength_high);

                    data.append(strCopy.toUtf8());

                    // 2-3 bytes: LEN - at last, set the data length.
                    uint16_t dataSize = data.size() - storageHeaderSize;
                    dataSize = DLT_SWAP_16(dataSize);
                    uint8_t dataSize_low = dataSize & 0xff;
                    uint8_t dataSize_high = (dataSize >> 8);
                    data[storageHeaderSize + 2] = dataSize_low;
                    data[storageHeaderSize + 3] = dataSize_high;
                }

                targetFile.write(data);

                ++ messageIndex;
            };

            QString line = sourceFile.readLine();
            while (!line.isNull())
            {
                writeDltMsg(line);
                line = sourceFile.readLine();
            }

            targetFile.close();
        }
        else
        {
            SEND_ERR(QString("Failed to open file \"%1\". Error: \"%2\"")
                         .arg(targetFilePath)
                         .arg(targetFile.errorString()));
            bResult = false;
        }
    }
    else
    {
        SEND_ERR(QString("Failed to open file \"%1\". Error: \"%2\"")
                     .arg(sourceFilePath)
                     .arg(sourceFile.errorString()));
        bResult = false;
    }

    return bResult;
}

bool convertLogFileToDLTV2( const QString& sourceFilePath,
                           const QString& targetFilePath )
{
    bool bResult = true;

    QElapsedTimer timer;

    timer.start();

    QFile sourceFile(sourceFilePath);
    QFile targetFile(targetFilePath);

    if(sourceFile.open(QFile::OpenModeFlag::ReadOnly)) // open source file
    {
        if(targetFile.open(QFile::OpenModeFlag::ReadWrite | QFile::OpenModeFlag::Truncate)) // open target file
        {
            uint8_t messageCounter = 0u;

            auto writeDltMsg = [&targetFile,
                                &messageCounter,
                                &timer](const QString& str)
            {
                uint16_t storageHeaderSize = 0u;
                uint16_t headerLenOffset = 0u;

                auto elapsedNsecs = timer.nsecsElapsed();

                auto elapsedSeconds = static_cast<uint64_t>(elapsedNsecs / 1000000000);
                auto elapsedNanoSeconds = static_cast<uint64_t>(elapsedNsecs % 1000000000);

                auto currentMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
                auto currentSecsSinceEpoch = currentMSecsSinceEpoch / 1000;
                auto currentNanoSecsSinceEpoch = currentMSecsSinceEpoch % 1000 * 1000000;

                QByteArray data;

                {
                    // STORAGE HEADER

                    data.push_back(0x44); // D
                    data.push_back(0x4c); // L
                    data.push_back(0x54); // T
                    data.push_back(0x02);

                    uint8_t currentNanoSecsSinceEpoch_1_byte = currentNanoSecsSinceEpoch & 0xff;
                    uint8_t currentNanoSecsSinceEpoch_2_byte = (currentNanoSecsSinceEpoch >> 8);
                    uint8_t currentNanoSecsSinceEpoch_3_byte = (currentNanoSecsSinceEpoch >> 16);
                    uint8_t currentNanoSecsSinceEpoch_4_byte = (currentNanoSecsSinceEpoch >> 24);

                    data.push_back( currentNanoSecsSinceEpoch_1_byte );
                    data.push_back( currentNanoSecsSinceEpoch_2_byte );
                    data.push_back( currentNanoSecsSinceEpoch_3_byte );
                    data.push_back( currentNanoSecsSinceEpoch_4_byte );

                    uint8_t currentSecsSinceEpoch_1_byte = currentSecsSinceEpoch & 0xff;
                    uint8_t currentSecsSinceEpoch_2_byte = (currentSecsSinceEpoch >> 8);
                    uint8_t currentSecsSinceEpoch_3_byte = (currentSecsSinceEpoch >> 16);
                    uint8_t currentSecsSinceEpoch_4_byte = (currentSecsSinceEpoch >> 24);
                    uint8_t currentSecsSinceEpoch_5_byte = (currentSecsSinceEpoch >> 32);

                    data.push_back( currentSecsSinceEpoch_1_byte );
                    data.push_back( currentSecsSinceEpoch_2_byte );
                    data.push_back( currentSecsSinceEpoch_3_byte );
                    data.push_back( currentSecsSinceEpoch_4_byte );
                    data.push_back( currentSecsSinceEpoch_5_byte );

                    data.push_back( 4u );

                    data.push_back('E');
                    data.push_back('C');
                    data.push_back('U');
                    data.push_back('1');

                    storageHeaderSize = data.size();
                }

                {
                    // HEADER

                    // 0-3 bytes: HTYP2
                    char HTYP2_byte1 = 0u;
                    HTYP2_byte1 &= ~( 1UL );         // CNTI. Content Info. 2 bits. 0 stands for:
                    HTYP2_byte1 &= ~( 1UL << 1 );    // Verbose Mode Data Message
                    HTYP2_byte1 |=  ( 1UL << 2 );    // WEID. With ECU ID
                    HTYP2_byte1 |=  ( 1UL << 3 );    // WACID. With App and Context ID
                    HTYP2_byte1 &= ~( 1UL << 4 );    // WSID. With Session ID
                    HTYP2_byte1 &= ~( 1UL << 5 );    // Version. 3 bits. We are using version 2.
                    HTYP2_byte1 |=  ( 1UL << 6 );    // Version
                    HTYP2_byte1 &= ~( 1UL << 7 );    // Version

                    char HTYP2_byte2 = 0u;
                    HTYP2_byte2 &= ~( 1UL );         // WSFLN. With Source File Name and Line Number
                    HTYP2_byte2 &= ~( 1UL << 1 );    // WTGS. With Tags
                    HTYP2_byte2 &= ~( 1UL << 2 );    // WPVL. With Privacy Level
                    HTYP2_byte2 &= ~( 1UL << 3 );    // WSGM. With Segmentation
                    HTYP2_byte2 &= ~( 1UL << 4 );    // Reserved
                    HTYP2_byte2 &= ~( 1UL << 5 );    // Reserved
                    HTYP2_byte2 &= ~( 1UL << 6 );    // Reserved
                    HTYP2_byte2 &= ~( 1UL << 7 );    // Reserved

                    // 2: reserved
                    char HTYP2_byte3 = 0u;

                    // 3: reserved
                    char HTYP2_byte4 = 0u;

                    data.push_back(HTYP2_byte1);
                    data.push_back(HTYP2_byte2);
                    data.push_back(HTYP2_byte3);
                    data.push_back(HTYP2_byte4);

                    // 4 byte: MCNT. Message counter.
                    data.push_back(messageCounter);
                    ++messageCounter;

                    headerLenOffset = data.size();

                    // 5-6 bytes: LEN - set last, when we can calculate this value.
                    data.push_back('0');
                    data.push_back('0');

                    // 7 byte: MSIN. Message info
                    char MSIN_byte = 0u;
                    MSIN_byte &= ~( 1UL );         // Reserved
                    MSIN_byte &= ~( 1UL << 1 );    // MSTP. Message type.
                    MSIN_byte &= ~( 1UL << 2 );    // 3 bytes
                    MSIN_byte &= ~( 1UL << 3 );    // 0 stands for Dlt Log Message
                    MSIN_byte &= ~( 1UL << 4 );    // MTIN. Message type info
                    MSIN_byte |= ( 1UL << 5 );     // 4 bytes
                    MSIN_byte |= ( 1UL << 6 );     // 6 stands for DLT_LOG_VERBOSE
                    MSIN_byte &= ~( 1UL << 7 );    //
                    data.push_back(MSIN_byte);

                    // 8 byte: NOAR. Number of arguments
                    // We have one argument, which is a payload
                    data.push_back(1u);

                    // 7-15 bytes: ns-Timestamp
                    uint8_t elapsedNanoSeconds_1_byte = elapsedNanoSeconds & 0xff;
                    uint8_t elapsedNanoSeconds_2_byte = (elapsedNanoSeconds >> 8);
                    uint8_t elapsedNanoSeconds_3_byte = (elapsedNanoSeconds >> 16);
                    uint8_t elapsedNanoSeconds_4_byte = (elapsedNanoSeconds >> 24);

                    auto elapsedNanoSeconds_4_byte_with_reserved_bits = elapsedNanoSeconds_4_byte;
                    elapsedNanoSeconds_4_byte_with_reserved_bits |= ( 1UL << 7 );
                    data.push_back( elapsedNanoSeconds_4_byte_with_reserved_bits );
                    data.push_back( elapsedNanoSeconds_3_byte );
                    data.push_back( elapsedNanoSeconds_2_byte );
                    data.push_back( elapsedNanoSeconds_1_byte );

                    uint8_t elapsedSeconds_1_byte = elapsedSeconds & 0xff;
                    uint8_t elapsedSeconds_2_byte = (elapsedSeconds >> 8);
                    uint8_t elapsedSeconds_3_byte = (elapsedSeconds >> 16);
                    uint8_t elapsedSeconds_4_byte = (elapsedSeconds >> 24);
                    uint8_t elapsedSeconds_5_byte = (elapsedSeconds >> 32);

                    data.push_back( elapsedSeconds_5_byte );
                    data.push_back( elapsedSeconds_4_byte );
                    data.push_back( elapsedSeconds_3_byte );
                    data.push_back( elapsedSeconds_2_byte );
                    data.push_back( elapsedSeconds_1_byte );
                }

                {
                    // EXTENSION HEADER

                    // ECU ID
                    data.push_back(4u);
                    data.push_back('E');
                    data.push_back('C');
                    data.push_back('U');
                    data.push_back('1');

                    // APP ID
                    data.push_back(4u);
                    data.push_back('C');
                    data.push_back('O');
                    data.push_back('N');
                    data.push_back('V');

                    // CONTEXT ID
                    data.push_back(4u);
                    data.push_back('I');
                    data.push_back('M');
                    data.push_back('P');
                    data.push_back('1');
                }

                {
                    // PAYLOAD

                    // 0-4 bytes: ARG1 Type Info
                    uint8_t typeInfo_byte_1 = 0u;

                    typeInfo_byte_1 &= ~(1UL);       // Type length
                    typeInfo_byte_1 &= ~(1UL << 1);  // Type length
                    typeInfo_byte_1 &= ~(1UL << 2);  // Type length
                    typeInfo_byte_1 &= ~(1UL << 3);  // Type length
                    typeInfo_byte_1 &= ~(1UL << 4);  // Bool
                    typeInfo_byte_1 &= ~(1UL << 5);  // SINT
                    typeInfo_byte_1 &= ~(1UL << 6);  // UINT
                    typeInfo_byte_1 &= ~(1UL << 7);  // FLOAT

                    uint8_t typeInfo_byte_2 = 0u;

                    typeInfo_byte_2 &= ~(1UL);       // ARAY
                    typeInfo_byte_2 |= (1UL << 1);   // STRG
                    typeInfo_byte_2 &= ~(1UL << 2);  // RAWD
                    typeInfo_byte_2 &= ~(1UL << 3);  // VARI
                    typeInfo_byte_2 &= ~(1UL << 4);  // FIXP
                    typeInfo_byte_2 &= ~(1UL << 5);  // TRAI
                    typeInfo_byte_2 &= ~(1UL << 6);  // STRU
                    typeInfo_byte_2 |= (1UL << 7);   // TYFM

                    uint8_t typeInfo_byte_3 = 0u;

                    typeInfo_byte_3 &= ~(1UL);       // TYFM
                    typeInfo_byte_3 &= ~(1UL << 1);  // TYFM
                    typeInfo_byte_3 &= ~(1UL << 2);  // (TYPR)
                    typeInfo_byte_3 &= ~(1UL << 3);  // (TYPR)
                    typeInfo_byte_3 &= ~(1UL << 4);  // (TYPR)
                    typeInfo_byte_3 &= ~(1UL << 5);  // (TYPR)
                    typeInfo_byte_3 &= ~(1UL << 6);  // (TYPR)
                    typeInfo_byte_3 &= ~(1UL << 7);  // (TYPR)

                    uint8_t typeInfo_byte_4 = 0u;

                    typeInfo_byte_4 &= ~(1UL);       // Reserved
                    typeInfo_byte_4 &= ~(1UL << 1);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 2);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 3);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 4);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 5);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 6);  // Reserved
                    typeInfo_byte_4 &= ~(1UL << 7);  // Reserved

                    data.push_back(typeInfo_byte_1);
                    data.push_back(typeInfo_byte_2);
                    data.push_back(typeInfo_byte_3);
                    data.push_back(typeInfo_byte_4);

                    // ARG1 Payload
                    QString strCopy = str;
                    auto maxLength = 8192;
                    if(strCopy.length() >= maxLength )
                    {
                        strCopy = strCopy.mid(0, maxLength - 1);
                    }

                    uint16_t strLength = strCopy.size();
                    uint8_t strLength_low = strLength & 0xff;
                    uint8_t strLength_high = (strLength >> 8);
                    data.push_back(strLength_low);
                    data.push_back(strLength_high);

                    data.append(strCopy.toUtf8());

                    // 2-3 bytes: LEN - at last, set the data length.
                    uint16_t dataSize = data.size() - storageHeaderSize;
                    dataSize = DLT_SWAP_16(dataSize);
                    uint8_t dataSize_low = dataSize & 0xff;
                    uint8_t dataSize_high = (dataSize >> 8);
                    data[storageHeaderSize + 2] = dataSize_low;
                    data[storageHeaderSize + 3] = dataSize_high;
                }

                // at last, set the data length.
                uint16_t dataSize = data.size() - storageHeaderSize;
                dataSize = DLT_SWAP_16(dataSize);
                uint8_t dataSize_low = dataSize & 0xff;
                uint8_t dataSize_high = (dataSize >> 8);
                data[headerLenOffset] = dataSize_low;
                data[headerLenOffset + 1] = dataSize_high;

                targetFile.write(data);
            };

            QString line = sourceFile.readLine();
            while (!line.isNull())
            {
                writeDltMsg(line);
                line = sourceFile.readLine();
            }

            targetFile.close();
        }
        else
        {
            SEND_ERR(QString("Failed to open file \"%1\". Error: \"%2\"")
                         .arg(targetFilePath)
                         .arg(targetFile.errorString()));
            bResult = false;
        }
    }
    else
    {
        SEND_ERR(QString("Failed to open file \"%1\". Error: \"%2\"")
                     .arg(sourceFilePath)
                     .arg(sourceFile.errorString()));
        bResult = false;
    }

    return bResult;
}

bool isDarkMode()
{
    const QColor textColor = qApp->palette().text().color();
    return textColor.red() > 150 && textColor.green() > 150 && textColor.blue() > 150;
}

tQStringPtr getDataStrFromMsg(const tMsgId& msgId, const tMsgWrapperPtr &pMsg, eSearchResultColumn field)
{
    if(nullptr == pMsg)
    {
        return tQStringPtr();
    }

    tQStringPtr pStrRes = std::make_shared<QString>();

    switch(field)
    {
    case eSearchResultColumn::Index:
    {
        *pStrRes = QString("%1").arg(msgId);
    }
    break;
    case eSearchResultColumn::Time:
    {
        *pStrRes = QString("%1.%2").arg(pMsg->getTimeString()).arg(pMsg->getMicroseconds(),6,10,QLatin1Char('0'));
    }
    break;
    case eSearchResultColumn::Timestamp:
    {
        *pStrRes = QString("%1.%2").arg(pMsg->getTimestamp()/10000).arg(pMsg->getTimestamp()%10000,4,10,QLatin1Char('0'));
    }
    break;
    case eSearchResultColumn::Count:
    {
        *pStrRes = QString("%1").arg(pMsg->getMessageCounter());
    }
    break;
    case eSearchResultColumn::Ecuid:
    {
        *pStrRes = pMsg->getEcuid();
    }
    break;
    case eSearchResultColumn::Apid:
    {
        *pStrRes = pMsg->getApid();
    }
    break;
    case eSearchResultColumn::Ctid:
    {
        *pStrRes = pMsg->getCtid();
    }
    break;
    case eSearchResultColumn::SessionId:
    {
        *pStrRes = QString("%1").arg(pMsg->getSessionid());
    }
    break;
    case eSearchResultColumn::Type:
    {
        *pStrRes = pMsg->getTypeString();
    }
    break;
    case eSearchResultColumn::Subtype:
    {
        *pStrRes = pMsg->getSubtypeString();
    }
    break;
    case eSearchResultColumn::Mode:
    {
        *pStrRes = pMsg->getModeString();
    }
    break;
    case eSearchResultColumn::Args:
    {
        *pStrRes = QString("%1").arg(pMsg->getNumberOfArguments());
    }
    break;
    case eSearchResultColumn::Payload:
    {
        *pStrRes = pMsg->getPayload();
    }
    break;
    case eSearchResultColumn::UML_Applicability:
    {
        *pStrRes = ""; // no string value provided for this column
    }
    break;
    case eSearchResultColumn::PlotView_Applicability:
    {
        *pStrRes = ""; // no string value provided for this column
    }
    break;
    case eSearchResultColumn::Last:
    {
        *pStrRes = "Unhandled field type!";
    }
    break;
    default:
        break;
    }

    return pStrRes;
}

QColor getChartColor()
{
    static const std::vector<QColor> sColors
    {
        QColor(255, 0, 0),     // Bright Red
        QColor(0, 128, 0),     // Dark Green
        QColor(0, 0, 255),     // Bright Blue
        QColor(255, 165, 0),   // Orange
        QColor(128, 0, 128),   // Purple
        QColor(0, 255, 255),   // Cyan
        QColor(139, 69, 19),   // Saddle Brown
        QColor(255, 20, 147),  // Deep Pink
        QColor(255, 140, 0),   // Dark Orange
        QColor(75, 0, 130),    // Indigo
        QColor(0, 206, 209),   // Dark Turquoise
        QColor(34, 139, 34),   // Forest Green
        QColor(210, 105, 30),  // Chocolate
        QColor(148, 0, 211),   // Dark Violet
        QColor(0, 0, 139),     // Dark Blue
        QColor(255, 69, 0),    // Red-Orange
        QColor(0, 255, 127),   // Spring Green
        QColor(112, 128, 144), // Slate Gray
        QColor(0, 191, 255),   // Deep Sky Blue
        QColor(220, 20, 60),   // Crimson
        QColor(0, 128, 128),   // Teal
    };
    static const int sColorsSize = sColors.size();
    static std::atomic<int> sColorsCounter(0);
    return sColors[sColorsCounter++ % sColorsSize];
}

void releaseMemoryToOS()
{
#ifdef DMA_TC_MALLOC_OPTIMIZATION_ENABLED
    MallocExtension::instance()->ReleaseFreeMemory();
#elif DMA_GLIBC_MALLOC_OPTIMIZATION_ENABLED
    malloc_trim(0);
#endif
}

#ifdef DMA_TC_MALLOC_PROFILING_ENABLED
void dumpMemoryStatistics()
{
    SEND_MSG("");
    SEND_MSG("----------------------------------------------------|");
    SEND_MSG("---------------TC_MALLOC_OUTPUT_START---------------|");
    SEND_MSG("----------------------------------------------------|");
    SEND_MSG("");

    const int bufferSize = 100000;
    char stats[bufferSize];
    MallocExtension::instance()->GetStats(stats, bufferSize);
    QString str(stats);
    auto strVec = str.split("\n");

    for(const auto& str : strVec)
    {
        SEND_MSG(QString("%1").arg(str));
    }

    SEND_MSG("");
    SEND_MSG("----------------------------------------------------|");
    SEND_MSG("----------------TC_MALLOC_OUTPUT_END----------------|");
    SEND_MSG("----------------------------------------------------|");
    SEND_MSG("");
}
#endif

PUML_PACKAGE_BEGIN(Qt)
    PUML_CLASS_BEGIN(QThread)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QObject)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QWidget)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QLineEdit)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QTimer)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QElapsedTimer)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QTreeView)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QStyledItemDelegate)
    PUML_CLASS_END()
    PUML_ABSTRACT_CLASS_BEGIN(QAbstractItemModel)
    PUML_ABSTRACT_CLASS_END()
    PUML_ABSTRACT_CLASS_BEGIN(QAbstractTableModel)
    PUML_ABSTRACT_CLASS_END()
    PUML_CLASS_BEGIN(QImage)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QProcess)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QTableView)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QTabWidget)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QPlainTextEdit)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QPushButton)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCompleter)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QFileSystemWatcher)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QTextEdit)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QListView)
    PUML_CLASS_END()
PUML_PACKAGE_END()

PUML_PACKAGE_BEGIN(DLT)
    PUML_INTERFACE_BEGIN(QDLTPluginInterface)
    PUML_INTERFACE_END()
    PUML_INTERFACE_BEGIN(QDltPluginControlInterface)
    PUML_INTERFACE_END()
    PUML_INTERFACE_BEGIN(QDltPluginViewerInterface)
    PUML_INTERFACE_END()
#ifdef PLUGIN_API_COMPATIBILITY_MODE_1_0_0
    PUML_CLASS_BEGIN(QDltPlugin)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QDltPluginManager)
    PUML_CLASS_END()
#endif
    PUML_CLASS_BEGIN(QDltFile)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QDltMsg)
    PUML_CLASS_END()
PUML_PACKAGE_END()

PUML_PACKAGE_BEGIN(qcustomplot)
    PUML_CLASS_BEGIN(QCustomPlot)
        PUML_INHERITANCE(QWidget, extends)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCPLegend)
        PUML_INHERITANCE(QObject, extends)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCPAxis)
        PUML_INHERITANCE(QCPLayerable, extends)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCPLayerable)
        PUML_INHERITANCE(QObject, extends)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCPAxisRect)
        PUML_INHERITANCE(QObject, extends)
PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCPGraph)
        PUML_INHERITANCE(QObject, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()

PUML_PACKAGE_BEGIN(nlohmann_json)
    PUML_CLASS_BEGIN(nlohmann::json)
    PUML_CLASS_END()
PUML_PACKAGE_END()
