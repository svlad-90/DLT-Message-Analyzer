/**
 * @file    PlotDefinitions.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the PlotDefinitions.hpp class
 */
#ifndef PLOT_DEFINITIONS_HPP
#define PLOT_DEFINITIONS_HPP

#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

#include "TOptional.hpp"
#include "BaseDefinitions.hpp"

#include "QString"
#include "QStringList"
#include "QColor"

////////////////////// PLOT DEFINITIONS //////////////////////
typedef double tPlotData;
typedef std::vector<tPlotData> tPlotDataVec;

typedef int64_t tEventId;

typedef tQStringPtrWrapper tPlotGraphMetadataKey;
typedef tQStringPtrWrapper tPlotGraphMetadataValue;
typedef std::unordered_map<tPlotGraphMetadataKey, tPlotGraphMetadataValue> tPlotGraphMetadataMap;

enum class ePlotViewID
{
    PLOT_AXIS_RECTANGLE_TYPE = 0,
    PLOT_AXIS_RECTANGLE_LABEL,
    PLOT_X_MAX,
    PLOT_X_MIN,
    PLOT_Y_MAX,
    PLOT_Y_MIN,
    PLOT_X_NAME,
    PLOT_Y_NAME,
    PLOT_X_UNIT,
    PLOT_Y_UNIT,
    PLOT_GRAPH_NAME,  // this enum entry should have less value than all which have graph id parameter
    PLOT_GRAPH_METADATA,
    PLOT_X_TIMESTAMP, // this enum entry should have less value than PLOT_X_DATA
    PLOT_X_DATA,
    PLOT_Y_TIMESTAMP, // this enum entry should have less value than PLOT_Y_DATA
    PLOT_Y_DATA,
    PLOT_GANTT_EVENT_ID,
    PLOT_GANTT_EVENT
};

QString getPlotIDAsString( const ePlotViewID& val );
bool parsePlotViewIDFromString( const QString& data, ePlotViewID& val );

///////////////tPlotViewIDParameters////////////////
/*
 * All declarations, related to the tPlotViewIDParameters structure.
 * This one is parsed from the regex
 */
struct tPlotViewIDParametersItem
{
    QOptionalColor optColor;
    tQStringPtr pPlotViewGroupName;
    tQStringPtrVec plotViewSplitParameters;
};

typedef std::map<ePlotViewID, tPlotViewIDParametersItem> tPlotViewIDParametersMap;

struct tPlotViewIDParameters
{
    tPlotViewIDParametersMap plotViewIDParametersMap;
};
///////////////tPlotViewIDParameters ( end ) ///////

enum class ePlotViewIDType
{
    e_Optional,
    e_Mandatory_Non_Gantt,
    e_Mandatory_Gantt
};

QString getPlotIDTypeAsString( const ePlotViewIDType& val );

enum class ePlotViewParameterType
{
    e_Optional,
    e_Mandatory
};

QString getPlotViewParameterTypeAsString( const ePlotViewParameterType& val );

typedef std::function<bool(const QString& /*name*/,
                           ePlotViewParameterType /*type*/,
                           const QString& /*value*/,
                           QString& /*msg*/,
                           bool /*fillInStringMsg*/)>
    tParameterValidationFunc;

struct tPlotViewIDparameter
{
    QString name;
    ePlotViewParameterType type;
    tParameterValidationFunc validationFunction;
};

typedef std::shared_ptr<tPlotViewIDparameter> tPlotViewIDParameterPtr;
typedef std::vector<tPlotViewIDParameterPtr> tPlotViewIDParameterPtrVec;

struct tPlotViewIDItem
{
    ePlotViewIDType id_type;
    QString id_str;
    QString description;

    QString getParametersDescription();
    void addParameter(const tPlotViewIDParameterPtr& parameter);
    const tPlotViewIDParameterPtrVec& getParameters() const;
    const tPlotViewIDParameterPtrVec& getMandatoryParameters() const;
    const tPlotViewIDParameterPtrVec& getOptionalParameters() const;

private:
    tPlotViewIDParameterPtrVec parameters;
    tPlotViewIDParameterPtrVec mandatoryParameters;
    tPlotViewIDParameterPtrVec optionalParameters;
};

typedef std::map<ePlotViewID, tPlotViewIDItem> tPlotViewIDsMap;
extern const tPlotViewIDsMap sPlotViewIDsMap;
////////////////////// PLOT DEFINITIONS (END) ////////////////

enum class ePlotViewAxisType
{
    e_GANTT = 0,
    e_POINT,
    e_LINEAR
};

QString getAxisTypeAsString( const ePlotViewAxisType& val );
bool parseAxisTypeFromString( const QString& data, ePlotViewAxisType& val );

QRegularExpression createPlotViewRegex();
bool checkPlotViewParameters(QString& errorMsg, bool fillInStringMsg, const tPlotViewIDParametersMap& plotViewIDParametersMap);
bool checkPlotViewParameter(QString& errorMsgs,
                            bool fillInStringMsg,
                            ePlotViewID plotViewId,
                            const tQStringPtr& pPlotViewGroupName,
                            const tQStringPtrVec& plotViewSplitParameters);
QStringList splitPlotViewParameters(const QString& parameters);
int32_t getPlotViewParameterIndex(ePlotViewID plotViewID, const QString& parameterName);

bool stringsToDouble(double& res, const QString& intPartStr, const QString fracPartStr);

const tPlotViewIDItem getPlotViewIDItem(ePlotViewID plotViewID);

// parameters parsing

template<ePlotViewID T>
struct tPlotParametersParser;

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE>
{
    const ePlotViewID plotViewId = ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        ePlotViewAxisType axisRectType = ePlotViewAxisType::e_LINEAR;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                  fillInStringMsg,
                                  plotViewId,
                                  pPlotViewGroupName,
                                  splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];
            ePlotViewAxisType axisType = ePlotViewAxisType::e_LINEAR;
            assert(splitParameters[1] != nullptr);
            const auto& axisTypeStr = *splitParameters[1];
            if(true == parseAxisTypeFromString(axisTypeStr, axisType))
            {
                result.axisRectType = axisType;
                result.bParsingSuccessful = true;
            }
            else
            {
                if(true == fillInStringMsg)
                {
                    result.errors.append(QString(" < Unknown axis type '%1'>").arg(axisTypeStr));
                }
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_AXIS_RECTANGLE_LABEL>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_AXIS_RECTANGLE_LABEL;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        QString axisRectLabel;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                  fillInStringMsg,
                                  plotViewId,
                                  pPlotViewGroupName,
                                  splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.axisRectLabel = *splitParameters[1];

            result.bParsingSuccessful = true;
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_X_MAX>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_X_MAX;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        double value;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                   fillInStringMsg,
                                   plotViewId,
                                   pPlotViewGroupName,
                                   splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);

            tQStringPtr pFracPartStr = nullptr;

            if(splitParameters.size() >= 3)
            {
                assert(splitParameters[2] != nullptr);
                pFracPartStr = splitParameters[2];
            }

            if(true == stringsToDouble(result.value, *splitParameters[1], pFracPartStr ? *pFracPartStr : "00"))
            {
                result.bParsingSuccessful = true;
                if(splitParameters.size() >= 4)
                {
                    assert(splitParameters[3] != nullptr);
                    if(splitParameters[3]->compare( "neg", Qt::CaseInsensitive ) == 0)
                    {
                        result.value = -(result.value);
                    }
                }
            }
            else
            {
                if(true == fillInStringMsg)
                {
                    result.errors.append(" < Was not able to parse the max value. Please, check the provided data. >");
                }
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_X_MIN> :
public tPlotParametersParser<ePlotViewID::PLOT_X_MAX>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_X_MIN;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_Y_MAX> :
public tPlotParametersParser<ePlotViewID::PLOT_X_MAX>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_Y_MAX;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_Y_MIN> :
public tPlotParametersParser<ePlotViewID::PLOT_X_MAX>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_Y_MIN;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_X_NAME>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_X_NAME;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        QString value;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                  fillInStringMsg,
                                  plotViewId,
                                  pPlotViewGroupName,
                                  splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.value = *splitParameters[1];

            result.bParsingSuccessful = true;
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_Y_NAME> :
public tPlotParametersParser<ePlotViewID::PLOT_X_NAME>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_Y_NAME;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_X_UNIT> :
public tPlotParametersParser<ePlotViewID::PLOT_X_NAME>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_X_UNIT;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_Y_UNIT> :
public tPlotParametersParser<ePlotViewID::PLOT_X_NAME>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_Y_UNIT;
    }
};

typedef uint32_t tGraphId;

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_GRAPH_NAME>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_GRAPH_NAME;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        tGraphId graphId;
        TOptional<QString> graphName;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                  fillInStringMsg,
                                  plotViewId,
                                  pPlotViewGroupName,
                                  splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.graphId = splitParameters[1]->toInt(&result.bParsingSuccessful);

            if(false == result.bParsingSuccessful)
            {
                result.errors.append(QString(" <Failed to convert graphId '%1' to integer>").arg(*splitParameters[1]));
            }
            else if( splitParameters.size() > 2)
            {
                assert(splitParameters[2] != nullptr);
                result.graphName.setValue(*splitParameters[2]);
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_GRAPH_METADATA>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_GRAPH_METADATA;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        tGraphId graphId;
        QString key;
        TOptional<QString> value;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                   fillInStringMsg,
                                   plotViewId,
                                   pPlotViewGroupName,
                                   splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.graphId = splitParameters[1]->toInt(&result.bParsingSuccessful);

            if(false == result.bParsingSuccessful)
            {
                result.errors.append(QString(" <Failed to convert graphId '%1' to integer>").arg(*splitParameters[1]));
            }
            else if( splitParameters.size() > 2)
            {
                assert(splitParameters[2] != nullptr);
                result.key = *splitParameters[2];

                if(splitParameters.size() > 3)
                {
                    assert(splitParameters[3] != nullptr);
                    result.value.setValue(*splitParameters[3]);
                }
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_X_DATA>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_X_DATA;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        tGraphId graphId;
        TOptional<tPlotData> value;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                   fillInStringMsg,
                                   plotViewId,
                                   pPlotViewGroupName,
                                   splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.graphId = splitParameters[1]->toInt(&result.bParsingSuccessful);

            if(false == result.bParsingSuccessful)
            {
                result.errors.append(QString(" <Failed to convert graphId '%1' to integer>").arg(*splitParameters[1]));
            }
            else
            {
                if(splitParameters.size() > 2)
                {
                    assert(splitParameters[2] != nullptr);

                    result.value.setValue(splitParameters[2]->toDouble(&result.bParsingSuccessful));

                    if(false == result.bParsingSuccessful)
                    {
                        result.errors.append(QString(" <Failed to convert value '%1' to double>").arg(*splitParameters[2]));
                    }
                }
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_Y_DATA> :
public tPlotParametersParser<ePlotViewID::PLOT_X_DATA>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_Y_DATA;
    }
};

struct tTimestampParserBase
{
    enum class eTimestampItemType
    {
        separator = 0,
        year,
        month,
        day,
        hour,
        minute,
        second,
        fraction
    };

    typedef uint32_t tNumberOfSymbols;
    typedef std::vector<std::pair<eTimestampItemType, tNumberOfSymbols>> tTimestampDataVec;

    struct tParsedData
    {
        tTimestampDataVec timestampDataVec;
        QString regularExpressionStr;
        QString dateTimeFormat;
        int32_t minValueLength = 0;
    };
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_X_TIMESTAMP>:
public tTimestampParserBase
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_X_TIMESTAMP;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        tParsedData parsedData;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                   fillInStringMsg,
                                   plotViewId,
                                   pPlotViewGroupName,
                                   splitParameters))
        {
            assert(splitParameters[0] != nullptr);

            TOptional<eTimestampItemType> timestampItemType;
            TOptional<QString> numberOfSymbolsStr;

            for(const auto& ch : *splitParameters[0])
            {
                if(ch == 'w')
                {
                    timestampItemType.setValue(eTimestampItemType::separator);
                }
                else if(ch == 'y')
                {
                    timestampItemType.setValue(eTimestampItemType::year);
                }
                else if(ch == 'M')
                {
                    timestampItemType.setValue(eTimestampItemType::month);
                }
                else if(ch == 'd')
                {
                    timestampItemType.setValue(eTimestampItemType::day);
                }
                else if(ch == 'H')
                {
                    timestampItemType.setValue(eTimestampItemType::hour);
                }
                else if(ch == 'm')
                {
                    timestampItemType.setValue(eTimestampItemType::minute);
                }
                else if(ch == 's')
                {
                    timestampItemType.setValue(eTimestampItemType::second);
                }
                else if(ch == 'f')
                {
                    timestampItemType.setValue(eTimestampItemType::fraction);
                }
                else if(ch.digitValue() != -1)
                {
                    if(true == numberOfSymbolsStr.isSet())
                    {
                        numberOfSymbolsStr.getWriteableValue().append(ch);
                    }
                    else
                    {
                        numberOfSymbolsStr.setValue(QString(ch));
                    }
                }
                else
                {
                    result.errors.append(QString(" <Unexpected character - '%1' in timestamp definition '%2'>")
                                             .arg(ch, *splitParameters[0]));
                    return result;
                }

                if(true == timestampItemType.isSet())
                {
                    TOptional<tNumberOfSymbols> numberOfSymbolsOpt;

                    if(false == numberOfSymbolsStr.isSet())
                    {
                        numberOfSymbolsOpt.setValue(1);
                    }
                    else
                    {
                        bool convertedToDigit = false;

                        auto numberOfSymbols = numberOfSymbolsStr.getValue().toInt(&convertedToDigit);

                        if(true == convertedToDigit)
                        {
                            numberOfSymbolsOpt.setValue(numberOfSymbols);
                        }
                        else
                        {
                            result.errors.append(QString(" <Was not able to convert '%1' to int for the timestamp definition '%2'>")
                                                     .arg(numberOfSymbolsStr.getValue(), *splitParameters[0]));
                            return result;
                        }
                    }

                    result.parsedData.minValueLength += numberOfSymbolsOpt.getValue();

                    result.parsedData.timestampDataVec.push_back(std::make_pair(timestampItemType.getValue(),
                                                                  numberOfSymbolsOpt.getValue()));

                    timestampItemType.reset();
                    numberOfSymbolsStr.reset();
                }
            }

            if(false == result.parsedData.timestampDataVec.empty())
            {
                typedef tPlotParametersParser<ePlotViewID::PLOT_X_TIMESTAMP>::eTimestampItemType eTimestampItemType;

                for(const auto& timestampPair : result.parsedData.timestampDataVec)
                {
                    switch(timestampPair.first)
                    {
                    case eTimestampItemType::separator:
                    {
                        result.parsedData.regularExpressionStr.append(QString(".{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(":");
                    }
                    break;
                    case eTimestampItemType::year:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 'y'));
                    }
                    break;
                    case eTimestampItemType::month:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 'M'));
                    }
                    break;
                    case eTimestampItemType::day:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 'd'));
                    }
                    break;
                    case eTimestampItemType::hour:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 'H'));
                    }
                    break;
                    case eTimestampItemType::minute:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 'm'));
                    }
                    break;
                    case eTimestampItemType::second:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 's'));
                    }
                    break;
                    case eTimestampItemType::fraction:
                    {
                        result.parsedData.regularExpressionStr.append(QString("[\\d]{%1}").arg(timestampPair.second));
                        result.parsedData.dateTimeFormat.append(QString(timestampPair.second, 'z'));
                    }
                    break;
                    }
                }

                if(true == result.parsedData.regularExpressionStr.isEmpty())
                {
                    result.errors.append(QString(" <Was not able to create regular expression from the line '%1'>")
                                             .arg(*splitParameters[0]));
                }

                if(true == result.parsedData.dateTimeFormat.isEmpty())
                {
                    result.errors.append(QString(" <Was not able to parse date time format from the line '%1'>")
                                             .arg(*splitParameters[0]));
                }

                if(false == result.parsedData.regularExpressionStr.isEmpty() &&
                   false == result.parsedData.dateTimeFormat.isEmpty())
                {
                    result.bParsingSuccessful = true;
                }
            }
            else
            {
                result.errors.append(QString(" <Was not able to parse timestamp definitiong out of the '%1' string>")
                                         .arg(*splitParameters[0]));
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_Y_TIMESTAMP> :
    public tPlotParametersParser<ePlotViewID::PLOT_X_TIMESTAMP>
{
    tPlotParametersParser()
    {
        plotViewId = ePlotViewID::PLOT_Y_TIMESTAMP;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_GANTT_EVENT>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_GANTT_EVENT;

    enum class eGanttEventType
    {
        START = 0,
        END
    };

    static std::pair<bool, eGanttEventType> parseGanttEventType(const QString& val)
    {
        std::pair<bool, eGanttEventType> result;
        result.first = false;

        QString lowerVal = val.toLower();

        if(lowerVal == "start")
        {
            result.first = true;
            result.second = eGanttEventType::START;
        }
        else if(lowerVal == "end")
        {
            result.first = true;
            result.second = eGanttEventType::END;
        }

        return result;
    }

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        tGraphId graphId;
        eGanttEventType eventType;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                   fillInStringMsg,
                                   plotViewId,
                                   pPlotViewGroupName,
                                   splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.graphId = splitParameters[1]->toInt(&result.bParsingSuccessful);

            if(false == result.bParsingSuccessful)
            {
                result.errors.append(QString(" <Failed to convert graphId '%1' to integer>").arg(*splitParameters[1]));
            }
            else
            {
                assert(splitParameters[2] != nullptr);
                auto ganttEventTypeParsingResult = parseGanttEventType(*splitParameters[2]);

                if(true == ganttEventTypeParsingResult.first)
                {
                    result.bParsingSuccessful = true;
                    result.eventType = ganttEventTypeParsingResult.second;
                }
                else
                {
                    result.errors.append(QString(" <Failed to parse eventType from the '%1' value>").arg(*splitParameters[2]));
                }
            }
        }

        return result;
    }
};

template<>
struct tPlotParametersParser<ePlotViewID::PLOT_GANTT_EVENT_ID>
{
    ePlotViewID plotViewId = ePlotViewID::PLOT_GANTT_EVENT_ID;

    struct tParsingResult
    {
        bool bParsingSuccessful = false;
        QString errors;
        QString axisRectName;
        tGraphId graphId;
    };

    tParsingResult parse(bool fillInStringMsg,
                         const tQStringPtr& pPlotViewGroupName,
                         const tQStringPtrVec& splitParameters)
    {
        tParsingResult result;

        if(checkPlotViewParameter(result.errors,
                                   fillInStringMsg,
                                   plotViewId,
                                   pPlotViewGroupName,
                                   splitParameters))
        {
            assert(splitParameters[0] != nullptr);
            result.axisRectName = *splitParameters[0];

            assert(splitParameters[1] != nullptr);
            result.graphId = splitParameters[1]->toInt(&result.bParsingSuccessful);

            if(false == result.bParsingSuccessful)
            {
                result.errors.append(QString(" <Failed to convert graphId '%1' to integer>").arg(*splitParameters[1]));
            }
        }

        return result;
    }
};

#endif // PLOT_DEFINITIONS_HPP
