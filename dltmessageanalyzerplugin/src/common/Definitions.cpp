/**
 * @file    Definitions.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the Definitions class
 */

#include <assert.h>

#include <map>
#include <vector>
#include <atomic>

#include <QElapsedTimer>

#include "Definitions.hpp"

#include "QRegularExpression"
#include "QTextStream"

#include "../log/CConsoleCtrl.hpp"
#include "CTreeItem.hpp"

Q_DECLARE_METATYPE(eRequestState)
Q_DECLARE_METATYPE(int8_t)
Q_DECLARE_METATYPE(tRequestId)

const QString sDefaultStatusText = "No error ...";
const QString sDefaultRegexFileName = "Regex_Default.json";
const QString sRegexScriptingDelimiter = "_and_";
const QString sRGBPrefix = "RGB_";
const QString sVARPrefix = "VAR_";

//////// UML_IDENTIFIERS ////////
const QString s_UML_SEQUENCE_ID = "USID"; // - optional
const QString s_UML_CLIENT = "UCL"; // - mandatory
const QString s_UML_REQUEST = "URT"; // - mandatory
const QString s_UML_RESPONSE = "URS"; // - mandatory
const QString s_UML_EVENT = "UEV"; // - mandatory
const QString s_UML_SERVICE = "US"; // - mandatory
const QString s_UML_METHOD = "UM"; // - mandatory
const QString s_UML_ARGUMENTS = "UA"; // - optional
const QString s_UML_ALIAS_DELIMITER = "_";

static tUML_IDs_Map createUMLIDsMap()
{
    tUML_IDs_Map result;

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

const std::map<QString, QColor> sColorsMap =
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
from(from_), to(to_), color(color_), explicitColor(explicitColor_)
{}

tHighlightingRange::tHighlightingRange():
from(0), to(0), color(0,0,0), explicitColor(false)
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

/////////////////////////////tIntRangePtrWrapper/////////////////////////////
bool tIntRangePtrWrapper::operator< ( const tIntRangePtrWrapper& rVal ) const
{
    bool bResult = false;

    if(pRange == nullptr && rVal.pRange != nullptr)
        bResult = true;
    else if(pRange != nullptr && rVal.pRange == nullptr)
        bResult = false;
    else if(pRange == nullptr && rVal.pRange == nullptr)
        bResult = true;
    else
    {
        if( pRange->from < rVal.pRange->from )
        {
            bResult = true;
        }
        else if( pRange->from > rVal.pRange->from )
        {
            bResult = false;
        }
        else // if from == rVal.from
        {
            if( pRange->to < rVal.pRange->to )
            {
                bResult = true;
            }
            else
            {
                bResult = false;
            }
        }
    }

    return bResult;
}

bool tIntRangePtrWrapper::operator== ( const tIntRangePtrWrapper& rVal ) const
{
    if(pRange == nullptr && rVal.pRange != nullptr)
        return false;
    else if(pRange != nullptr && rVal.pRange == nullptr)
        return false;
    else if(pRange == nullptr && rVal.pRange == nullptr)
        return true;

    return ( pRange->from == rVal.pRange->from && pRange->to == rVal.pRange->to );
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
        matchesStack.reserve(static_cast<std::size_t>(foundMatches.size()) + 10); // +10 to cover possible overhead of nested groups.

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

            tIntRangePtrWrapper rangePtrWrapper;
            rangePtrWrapper.pRange = &match.range;
            tDataItem rangeVariant( rangePtrWrapper );
            auto* pAddedChild = pCurrentItem->appendChild(rangeVariant, data);
            pCurrentItem = pAddedChild;

            matchesStack.push_back( &match ); // push new element to the stack
        };

        for(const auto& match : foundMatches)
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

    tHighlightingRangeList resultList;
    resultList.reserve(static_cast<std::size_t>(regexScriptingMetadata.getItemsVec().size() + 10)); // +10 to cover overhead which might be caused with nested groups

    auto postVisitFunction = [&resultList, &inputRange, &regexScriptingMetadata, &gradientColors, &groupIdToColorMap](tTreeItem* pItem)
    {
        const auto& match = *(pItem->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());

        //SEND_MSG(QString("[calcRangesCoverageMulticolor][postVisitFunction] visit item - |idx:%1;range:from-%2,to-%3;msg-%4|")
        //         .arg(match.idx)
        //         .arg(match.range.from)
        //         .arg(match.range.to)
        //         .arg(*match.pMatchStr));

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
                color = scriptedColor.color;
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
                    resultList.push_back( tHighlightingRange( rangeToBeAdded.from,
                                                              rangeToBeAdded.to,
                                                              selectedColor.second,
                                                              selectedColor.first ) );
                }
            }

            // wee need to process "between children" cases in a loop ( in case if there are 2 children or more )
            if(children.size() >= 2)
            {
                for(auto it = children.begin(); it != children.end() - 1; ++it)
                {
                    const auto& firstChildMatch = *((*it)->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                    tAnalysisRange firstChildAnalysisRange( firstChildMatch.range, inputRange );

                    const auto& secondChildMatch = *((*(it+1))->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                    tAnalysisRange secondChildAnalysisRange( secondChildMatch.range, inputRange );

                    if((secondChildAnalysisRange.from - firstChildAnalysisRange.to) > 1)
                    {
                        tAnalysisRange rangeToBeAdded( tIntRange( firstChildAnalysisRange.to+1, secondChildAnalysisRange.from-1 ), inputRange);
                        resultList.push_back( tHighlightingRange( rangeToBeAdded.from,
                                                                  rangeToBeAdded.to,
                                                                  selectedColor.second,
                                                                  selectedColor.first ) );
                    }
                }
            }

            // we need to process "from last child to end" case manually
            {
                const auto& lastChild = *(children.end() - 1);
                const auto& lastChildMatch = *(lastChild->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());
                tAnalysisRange lastChildAnalysisRange(lastChildMatch.range, inputRange);

                if( analysisRange.to > lastChildAnalysisRange.to ) // if there is room for additional range
                {
                    // let's add it
                    tAnalysisRange rangeToBeAdded( tIntRange( lastChildAnalysisRange.to + 1, analysisRange.to ), inputRange);
                    resultList.push_back( tHighlightingRange( rangeToBeAdded.from,
                                                              rangeToBeAdded.to,
                                                              selectedColor.second,
                                                              selectedColor.first ) );
                }
            }
        }
        else // if there are no children
        {
            // let's add item unconditionally
            resultList.push_back( tHighlightingRange( analysisRange.from,
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
        for(auto& resultListItem : resultList)
        {
            resultListItem.from = resultListItem.from - normalizationIdx;
            resultListItem.to = resultListItem.to - normalizationIdx;
        }
    }

    tCalcRangesCoverageMulticolorResult result;


    result.highlightingRangeSet.insert(resultList.begin(), resultList.end());

    //for( const auto& resultItem : resultList )
    //{
        //SEND_MSG(QString("[calcRangesCoverageMulticolor][result] - |from-%1;to-%2;red-%3;green-%4;blue-%5,expColor-%6|")
        //        .arg(resultItem.from)
        //        .arg(resultItem.to)
        //        .arg(resultItem.color.red())
        //        .arg(resultItem.color.green())
        //        .arg(resultItem.color.blue())
        //        .arg(resultItem.explicitColor));
    //}

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
tItemMetadata::tItemMetadata(): msgId(-1),
    highlightingInfoMultiColor(),
    fieldRanges(),
    strSize(0),
    msgSize(0u),
    timeStamp(0u)
{

}

tItemMetadata::tItemMetadata( const tMsgId& msgId_,
                              const tMsgId& msgIdFiltered_,
                              const tFieldRanges& fieldRanges_,
                              const int& strSize_,
                              const unsigned int& msgSize_,
                              const unsigned int& timeStamp_):
    msgId(msgId_),
    msgIdFiltered(msgIdFiltered_),
    fieldRanges(fieldRanges_),
    strSize(strSize_),
    msgSize(msgSize_),
    timeStamp(timeStamp_)
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

        for(const auto& match : foundMatches)
        {
            sortedMatches.insert(std::make_pair(tSortedMatchKey(match.range.from, match.range.to - match.range.from), &match));
        }

        for(const auto& match : sortedMatches)
        {
            if(nullptr != match.second->pMatchStr && false == match.second->pMatchStr->isEmpty())
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

        if(false == highlightingRangesMulticolorRes.highlightingRangeSet.empty())
        {
            highlightingInfoMultiColor.insert( it.key(), highlightingRangesMulticolorRes.highlightingRangeSet );
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

tTreeItemSharedPtr tItemMetadata::updateUMLInfo(const tFoundMatches& foundMatches,
                                                const tRegexScriptingMetadata& regexScriptingMetadata,
                                                tTreeItemSharedPtr pTree)
{
    tRegexScriptingMetadata::tCheckIDs checkIDs;

    for(const auto& match : foundMatches)
    {
        checkIDs.insert(match.idx);
    }

    // check if string matches all required UML attributes
    UMLInfo.bUMLConstraintsFulfilled = regexScriptingMetadata.doesContainConsistentUMLData(false, checkIDs).first;
    UMLInfo.UMLDataMap.clear();

    /*
     * What should we fill in here?
     * E.g. for the following string:
     * UI
     * IF1
     * [daimler.if1verbose] 10886 MainMultiSeat#2:RP : setIsClientConnected() [unknown:0]]
     * With the following regex:
     * ^(?<UCL>[\w]+) IF1 .*\] (?<USID>[\d]+) (?<US>[A-Za-z]+#[\d]+):(?<URS>RP) : (?<UM>[\w])\((?<UA>.*)\) \[unknown
     * We will need to fill in the following information:
     * tUMLInfo( bUMLConstraintsFulfilled = true,
     * UMLDataMap( { eUML_ID::UML_CLIENT, { eSearchResultColumn::Apid, {0, 3} },
     *             { eUML_ID::UML_SEQUENCE_ID, { eSearchResultColumn::Payload, {22, 26} }
     *             { eUML_ID::UML_SERVICE, { eSearchResultColumn::Payload, {28, 40} }
     *             { eUML_ID::UML_RESPONSE, { eSearchResultColumn::Payload, {44, 45} } )
     *  );
     */

    auto pMatchesTree = pTree != nullptr ? pTree : getMatchesTree(foundMatches);

    if(true == UMLInfo.bUMLConstraintsFulfilled) // if UML matches are sufficient
    {
        if(nullptr != pMatchesTree)
        {
            auto preVisitFunction = [&regexScriptingMetadata, this](tTreeItem* pItem)
            {
                const auto& match = *(pItem->data(static_cast<int>(eTreeColumns::eTreeColumn_FoundMatch)).get<const tFoundMatch*>());

                // for each tree element we should check, whether it is related to UML data representation.
                // That can be done with checking each tree element against regexScriptingMetadata

                // lambda, which checks whether specific match is representing a UML data
                auto isUMLData = [&match, &regexScriptingMetadata]()->std::pair<bool, tOptional_UML_IDMap*>
                {
                    std::pair<bool, tOptional_UML_IDMap*> result;
                    result.first = false;

                    auto groupIdx = match.idx;
                    auto groups = regexScriptingMetadata.getItemsVec();

                    if(groupIdx >= 0 && groupIdx < groups.size())
                    {
                        const auto& pGroupMetadata = groups[groupIdx];

                        if(nullptr != pGroupMetadata &&
                           ( false == pGroupMetadata->optionalUML_ID.optional_UML_IDMap.empty() ) )
                        {
                            result.first = true; // we got the UML data
                            result.second = &pGroupMetadata->optionalUML_ID.optional_UML_IDMap;
                        }
                    }

                    return result;
                };

                auto UMLDataRes = isUMLData();

                if(true == UMLDataRes.first) // if we have a UML data
                {
                    // let's check and fill in the areas of the string, in which it is located.

                    for(const auto& UML_IDItem : *(UMLDataRes.second))
                    {
                        if(true == UML_IDItem.second.UML_Custom_Value.isEmpty()) // if there is no client's custom value
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

                            UMLInfo.UMLDataMap[UML_IDItem.first].push_back(UMLDataItem);
                        }
                        else // otherwise
                        {
                            // let's directly assign custom client's value
                            tUMLDataItem UMLDataItem;
                            UMLDataItem.UML_Custom_Value = UML_IDItem.second.UML_Custom_Value;
                            UMLInfo.UMLDataMap[UML_IDItem.first].push_back(UMLDataItem);
                        }
                    }
                }

                return true;
            };

            pMatchesTree->visit(preVisitFunction, CTreeItem::tVisitFunction(), false, true, false);
        }
    }

    return pMatchesTree;
}

//tFoundMatch
tFoundMatch::tFoundMatch():
pMatchStr(std::make_shared<QString>()),
range(0,0),
idx(0),
msgSizeBytes(0u),
timeStamp(0u),
msgId(0)
{}

tFoundMatch::tFoundMatch( const tQStringPtr& pMatchStr_,
                          const tIntRange& range_,
                          const int& idx_,
                          const unsigned int& msgSizeBytes_,
                          const unsigned int& timeStamp_,
                          const tMsgId& msgId_):
pMatchStr((nullptr!=pMatchStr_)?pMatchStr_:std::make_shared<QString>()),
range(range_),
idx(idx_),
msgSizeBytes(msgSizeBytes_),
timeStamp(timeStamp_),
msgId(msgId_)
{}

bool tFoundMatch::operator< (const tFoundMatch& rhs) const
{
    return msgId < rhs.msgId;
}

//tFoundMatchesPackItem
tFoundMatchesPackItem::tFoundMatchesPackItem()
{

}

tFoundMatchesPackItem::tFoundMatchesPackItem( tItemMetadata&& itemMetadata_,
                                              tFoundMatches&& foundMatches_ ):
  mItemMetadata(itemMetadata_),
  mFoundMatches(foundMatches_)
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
{
}

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
        case eRegexFiltersRowType::NonCapturingGroup:
        {
            result = "Non-capturing group";
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

tRegexScriptingMetadataItemPtr parseRegexGroupName( const QString& groupName, bool bParseUMLData )
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
                        optColor.color = QColor(red, green, blue);
                    }
                }
            }
            else
            {
                QString lowerGroupNamePart = groupNamePart.toLower();

                auto foundColor = sColorsMap.find(lowerGroupNamePart);

                if(sColorsMap.end() != foundColor)
                {
                    optColor.isSet = true;
                    optColor.color = foundColor->second;
                }
            }
        }

        // parse var
        if(false == optVarName.first)
        {
            QRegularExpressionMatch varMatch = varRegex.match(groupNamePart);

            if(varMatch.hasMatch())
            {
                bool scriptedVarFound = varMatch.lastCapturedIndex() == 1;           // if 1 group found

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
            QRegularExpressionMatch varMatch = UMLRegex.match(groupNamePart);

            if(varMatch.hasMatch())
            {
                bool b_UML_ID_Found = varMatch.lastCapturedIndex() >= 1;               // if 1 or more groups found

                if(true == b_UML_ID_Found)
                {
                    eUML_ID UML_ID = eUML_ID::UML_EVENT; // any value
                    bool bUML_ID_Parsed = parseUMLIDFromString( varMatch.captured(1), UML_ID );

                    if(true == bUML_ID_Parsed)
                    {
                        const auto& key = UML_ID;
                        auto& value = optUML_ID.optional_UML_IDMap[key].UML_Custom_Value;

                        if(varMatch.lastCapturedIndex() == 2) // if 2 groups found
                        {
                            QString capturedString = varMatch.captured(2);
                            bool b_UML_Custom_Value_Found = false == capturedString.isEmpty();     // if 2 groups found

                            if(true == b_UML_Custom_Value_Found)
                            {
                                value = capturedString;
                            }
                        }
                    }
                }
            }
        }
    }

    tRegexScriptingMetadataItemPtr pItem = std::make_shared<tRegexScriptingMetadataItem>();
    pItem->highlightingColor = optColor;
    pItem->varName = optVarName;
    pItem->optionalUML_ID = optUML_ID;

    return pItem;
}

//tRegexScriptingMetadata
bool tRegexScriptingMetadata::parse(const QRegularExpression& regex, bool bParseUMLData)
{
    bool bResult = true;

    if(true == regex.isValid())
    {
        auto groupNames = regex.namedCaptureGroups();

        for(const auto& groupName : groupNames)
        {
            mItemsVec.push_back(parseRegexGroupName(groupName, bParseUMLData));
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

std::pair<bool /*status*/, QString /*status description*/>
tRegexScriptingMetadata::doesContainConsistentUMLData(bool fillInStringMsg) const
{
    return doesContainConsistentUMLData(fillInStringMsg, tCheckIDs(), true);
}

std::pair<bool /*status*/, QString /*status description*/>
tRegexScriptingMetadata::doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs) const
{
    return doesContainConsistentUMLData(fillInStringMsg, checkIDs, false);
}

std::pair<bool /*status*/, QString /*status description*/>
tRegexScriptingMetadata::doesContainConsistentUMLData(bool fillInStringMsg, const tCheckIDs& checkIDs, bool bCheckAll) const
{
    std::pair<bool /*status*/, QString /*status description*/> result;
    result.first = false;

    if(true == fillInStringMsg)
    {
        result.second.append("UML parsing result: ");
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
    else if(item.index() == tDataItem::index_of<tIntRangePtrWrapper>())
    {
        result.setValue(item.get<tIntRangePtrWrapper>());
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

bool QOptionalColor::operator== ( const QOptionalColor& rhs ) const
{
    return color == rhs.color && isSet == rhs.isSet;
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
        if( optColor.color.red() < rhs.optColor.color.red() )
        {
            bResult = true;
        }
        else if( optColor.color.red() > rhs.optColor.color.red() )
        {
            bResult = false;
        }
        else
        {
            if( optColor.color.green() < rhs.optColor.color.green() )
            {
                bResult = true;
            }
            else if( optColor.color.green() > rhs.optColor.color.green() )
            {
                bResult = false;
            }
            else
            {
                if( optColor.color.blue() < rhs.optColor.color.blue() )
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
