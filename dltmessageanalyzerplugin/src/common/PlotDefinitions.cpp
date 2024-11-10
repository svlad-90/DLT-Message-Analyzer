/**
 * @file    PlotDefinitions.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the PlotDefinitions class
 */

#include <cmath>

#include <QRegularExpression>

#include "PlotDefinitions.hpp"

//////// PLOT_IDENTIFIERS ////////
const QString s_PLOT_AXIS_RECTANGLE_TYPE = "PARType";
const QString s_PLOT_AXIS_RECTANGLE_LABEL = "PARL";
const QString s_PLOT_X_MAX = "PXMx";
const QString s_PLOT_X_MIN = "PXMn";
const QString s_PLOT_Y_MAX = "PYMx";
const QString s_PLOT_Y_MIN = "PYMn";
const QString s_PLOT_X_NAME = "PXN";
const QString s_PLOT_Y_NAME = "PYN";
const QString s_PLOT_X_UNIT = "PXU";
const QString s_PLOT_Y_UNIT = "PYU";
const QString s_PLOT_X_DATA = "PXData";
const QString s_PLOT_X_TIMESTAMP = "PXT";
const QString s_PLOT_Y_DATA = "PYData";
const QString s_PLOT_Y_TIMESTAMP = "PYT";
const QString s_PLOT_GRAPH_NAME = "PGN";
const QString s_PLOT_GRAPH_METADATA = "PGMD";
const QString s_PLOT_GANTT_EVENT = "PGE";
const QString s_PLOT_GANTT_EVENT_ID = "PGEID";
const QString s_PLOT_PARAMETER_DELIMITER = "_";

QString tPlotViewIDItem::getParametersDescription()
{
    QString result = id_str;

    if(false == parameters.empty())
    {
        result.append(" + _ + ");
    }

    for(auto it = parameters.begin(); it != parameters.end(); ++it)
    {
        switch((*it)->type)
        {
        case ePlotViewParameterType::e_Mandatory:
        {
            result.append(QString("<%1>").arg((*it)->name));
        }
        break;
        case ePlotViewParameterType::e_Optional:
        {
            result.append(QString("[%1]").arg((*it)->name));
        }
        break;
        }

        if(it < parameters.end() - 1)
        {
            result.append(" + _ + ");
        }
    }
    return result;
}

void tPlotViewIDItem::addParameter(const tPlotViewIDParameterPtr& parameter)
{
    parameters.push_back(parameter);

    switch(parameter->type)
    {
    case ePlotViewParameterType::e_Mandatory:
        mandatoryParameters.push_back(parameter);
        break;
    case ePlotViewParameterType::e_Optional:
        optionalParameters.push_back(parameter);
        break;
    }
}

const tPlotViewIDParameterPtrVec& tPlotViewIDItem::getParameters() const
{
    return parameters;
}

const tPlotViewIDParameterPtrVec& tPlotViewIDItem::getMandatoryParameters() const
{
    return mandatoryParameters;
}

const tPlotViewIDParameterPtrVec& tPlotViewIDItem::getOptionalParameters() const
{
    return optionalParameters;
}

static tPlotViewIDsMap createPlotIDsMap()
{
    tPlotViewIDsMap result;

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_AXIS_RECTANGLE_TYPE;

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectType";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                              ePlotViewParameterType,
                                              const QString& value,
                                              QString& msg,
                                              bool fillInStringMsg)
            {
                bool bResult = true;
                if(value.compare( "GANTT", Qt::CaseInsensitive ) != 0 &&
                   value.compare( "BAR", Qt::CaseInsensitive ) != 0 &&
                   value.compare( "POINT", Qt::CaseInsensitive ) != 0 &&
                   value.compare( "LINEAR", Qt::CaseInsensitive ) != 0)
                {
                    if(fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. Expected values for the "
                                           "'axisRectType' parameter are: GANTT, BAR, POINT, LINEAR.").arg(value));
                    }
                    bResult = false;
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        item.description = QString("Plot axis rectangle type. %1. "
                                   "PARType_CPUC_LINEAR. Type of the specific plot axis rectangle. "
                                   "Supported types are - GANTT, POINT, LINEAR. "
                                   "If not specified, the LINEAR value is used. "
                                   "If multiple values appear - the 'last win' strategy is applied.").arg(item.getParametersDescription());

        result.insert(std::make_pair(ePlotViewID::PLOT_AXIS_RECTANGLE_TYPE, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_AXIS_RECTANGLE_LABEL;

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectLabel";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        item.description = QString("Plot axis rectangle label. %1. "
                                   "PARL_CPUC_MyChart. The label, that will be created above the corresponding plot axis rectangle. "
                                   "If not specified, the label is not created. "
                                   "If multiple values appear for the same 'axisRectName' - they are concatenated.").arg(item.getParametersDescription());

        result.insert(std::make_pair(ePlotViewID::PLOT_AXIS_RECTANGLE_LABEL, item));
    }

    auto formMinMaxParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "integer_part_value";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "real_part_value";
            pParameter->type = ePlotViewParameterType::e_Optional;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "neg";
            pParameter->type = ePlotViewParameterType::e_Optional;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                if(value.compare( "neg", Qt::CaseInsensitive ) != 0)
                {
                    if(fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. Expected value for the "
                                           "'neg' parameter is: neg.").arg(value));
                    }
                    bResult = false;
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_X_MAX;
        formMinMaxParameters(item);
        item.description = QString("Plot X Max. %1. "
                           "Sets the maximum visible value on axis X of the specified axis rectangle. "
                           "Last win. If not set, then the maximum value will be derived from the provided data. "
                           "You can omit 'real_part_value' if not needed. "
                           "Use the 'neg' keyword as the last parameter to specify negative values. "
                                   "Not applicable to Gantt charts.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_X_MAX, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_X_MIN;
        formMinMaxParameters(item);
        item.description = QString("Plot X Min. %1. "
                           "Sets the minimum visible value on axis X of the specified axis rectangle. "
                           "Last win. If not set, then the minimum value will be derived from the provided data. "
                           "You can omit 'real_part_value' if not needed. "
                           "Use the 'neg' keyword as the last parameter to specify negative values. "
                                   "Not applicable to Gantt charts.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_X_MIN, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_Y_MAX;
        formMinMaxParameters(item);
        item.description = QString("Plot Y Max. %1. "
                           "Sets the maximum visible value on axis Y of the specified axis rectangle. "
                           "Last win. If not set, then the maximum value will be derived from the provided data. "
                           "You can omit 'real_part_value' if not needed. "
                           "Use the 'neg' keyword as the last parameter to specify negative values. "
                           "Not applicable to Gantt charts.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_Y_MAX, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_Y_MIN;
        formMinMaxParameters(item);
        item.description = QString("Plot Y Min. %1. "
                           "Sets the minimum visible value on axis Y of the specified axis rectangle. "
                           "Last win. If not set, then the minimum value will be derived from the provided data. "
                           "You can omit 'real_part_value' if not needed. "
                           "Use the 'neg' keyword as the last parameter to specify negative values. "
                                   "Not applicable to Gantt charts.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_Y_MIN, item));
    }

    auto formNameParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "name";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_X_NAME;
        formNameParameters(item);
        item.description = QString("Plot X-axis name. %1. "
                           "Sets plot X-axis name for the specified plot axis rectangle. "
                           "Use camelStyleNaming. E.g. 'camelStyleNaming' will be represented as \"Camel style naming\" inside the diagram. "
                                   "If not set, the default 'Timestamp' value is used.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_X_NAME, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_Y_NAME;
        formNameParameters(item);
        item.description = QString("Plot Y-axis name. %1. "
                           "Sets plot Y-axis name for the specified plot axis rectangle. "
                           "Use camelStyleNaming. E.g. 'camelStyleNaming' will be represented as 'Camel style naming' inside the diagram. "
                            "If not set - remains empty.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_Y_NAME, item));
    }

    auto formUnitParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "unit";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_X_UNIT;
        formUnitParameters(item);
        item.description = QString("Plot X-axis unit. %1. "
                           "Sets the plot X-axis unit for the . "
                           "Use camelStyleNaming. E.g. 'camelStyleNaming' will be represented as 'Camel style naming' inside the diagram. "
                           "If not set, then the 'seconds' default value is used, as the X-axis is usually used to represent the time, "
                           "and in dlt, the timestamp is measured in seconds ( with real part, "
                           "representing milliseconds, microseconds, etc. )").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_X_UNIT, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_Y_UNIT;
        formUnitParameters(item);
        item.description = QString("Plot Y-axis unit. %1. "
                           "Sets plot Y-axis name for the specified plot axis rectangle. Use camelStyleNaming. "
                           "E.g. 'camelStyleNaming' will be represented as 'Camel style naming' inside the diagram. "
                           "If not set - remains empty.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_Y_UNIT, item));
    }

    auto formDataParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "graphId";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "value";
            pParameter->type = ePlotViewParameterType::e_Optional;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toDouble(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to double.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_X_DATA;
        formDataParameters(item);
        item.description = QString("Plot axis X data. %1. "
                                   "The content of this regex group contains the data, that will be placed at axis X "
                                   "of the specified graph id and axis rectangle. The data of EACH VALUE should "
                                   "be convertible from the string to the float or the date-time. "
                                   "For conversion to date-time see PXT parameter. "
                                   "The graphId parameter should be convertible to an integer. If an optional 'value' "
                                   "parameter was spcified, the captured data would be ignored. The 'value' param will "
                                   "be NOT considered as the 'time', even if the PXT data was provided."
                                   "Optional. If not set, the timestamp of each involved DLT message would be used. "
                                   "This parameter can be used for the Gantt chart to get the timestamp from the "
                                   "message's payload.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_X_DATA, item));
    }

    auto formTimeParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "timeFormat";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static const QString allowedCharacters = "0-9wyMdHmsf";
                QRegularExpression re("^[" + allowedCharacters + "]+$");

                bool isMatch = re.match(value).hasMatch();
                if(false == isMatch)
                {

                    bResult = false;
                    if(true == fillInStringMsg)
                    {
                        QString allRedundantChars = value;
                        allRedundantChars.remove(QRegularExpression("[" + allowedCharacters + "]"));
                        QString uniqueRedundantChars;
                        for (QChar c : allRedundantChars)
                        {
                            if (!uniqueRedundantChars.contains(c))
                                uniqueRedundantChars.append(c);
                        }

                        msg.append(QString("Provided literal should contain only \"0-9wyMdHmsf\" characters. "
                                           "The \"%1\" characters are redundant.").arg(uniqueRedundantChars));
                    }
                }

                return bResult;
            };
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_X_TIMESTAMP;
        formTimeParameters(item);
        item.description = QString("Plot X-axis time format. %1. "
                                   "Specifies, how to decode the date and time from the string out of the PXData. "
                                   "E.g. the '05-24 18:25:01.439' time format can be decoded with PXT_2Mw2dw2Hw2mw2sw3f. "
                                   "y - year. M - month. d - day. H - hour. m - minute. s - second. w - any delimiter symbol. "
                                   "f - a real part of the second ( milliseconds, microseconds, etc. ). "
                                   "Useful for custom imported logs, like Android logcat traces, "
                                   "which do not have proper dlt timestamp. Optional. If not specified - "
                                   "there would be no attempt to decode PXData as time.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_X_TIMESTAMP, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Mandatory_Non_Gantt;
        item.id_str = s_PLOT_Y_DATA;
        formDataParameters(item);
        item.description = QString("Plot Y-axis time format. %1. "
                                   "The content of this regex group contains the data, that will be placed "
                                   "at axis Y of the specified graph id and axis rectangle. The data of "
                                   "EACH VALUE should be convertible from the string to the float or the date-time. "
                                   "For conversion to date-time see PYT parameter. The graphId parameter should be "
                                   "convertible to an integer. If an optional 'value' "
                                   "parameter was spcified, the captured data would be ignored. The 'value' param will "
                                   "be NOT considered as the 'time', even if the PYT data was provided. "
                                   "Mandatory for plots in which 'PARType' is not equal to 'GANTT'.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_Y_DATA, item));
    }

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_Y_TIMESTAMP;
        formTimeParameters(item);
        item.description = QString("Plot Y-axis time format. %1. "
                                   "Specifies, how to decode the date and time from the string out of the PYData. "
                                   "E.g. the '05-24 18:25:01.439' time format can be decoded with PYT_2Mw2dw2Hw2mw2sw3f. "
                                   "y - year. M - month. d - day. H - hour. m - minute. s - second. w - any delimiter symbol. "
                                   "f - a real part of the second ( milliseconds, microseconds, etc. ). Useful for custom imported logs, "
                                   "like Android logcat traces, which do not have proper dlt timestamp. "
                                   "If not specified - there would be no attempt to decode PYData as time.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_Y_TIMESTAMP, item));
    }

    auto formGraphNameParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "graphId";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "name";
            pParameter->type = ePlotViewParameterType::e_Optional;
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Mandatory_Non_Gantt;
        item.id_str = s_PLOT_GRAPH_NAME;
        formGraphNameParameters(item);
        item.description = QString("Plot graph name. %1."
                                   "Specifies the graph name for the specified graph id. If custom_value parameter is not specified, "
                                   "then graph names would be derived from the capture content, and a separate graph on the "
                                   "plot would be created for each unique captured graph name. If custom_value parameter is "
                                   "specified, then all names of the specified graphId would be the hard-coded ones, and only "
                                   "one graph would be produced on the plot for that graph id.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_GRAPH_NAME, item));
    }

    auto formGraphMetadataParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "graphId";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "key";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "value";
            pParameter->type = ePlotViewParameterType::e_Optional;
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_GRAPH_METADATA;
        formGraphMetadataParameters(item);
        item.description = QString("Plot graph metadata. %1."
                                   "Declares a metadata item, which ( if found for a specific plot point ) "
                                   "will be added to point's representation when it is selected by the user. "
                                   "Optional. One or more. If 'value' parameter is not explicitly set, then "
                                   "captured data will be set as 'value'. If multiple 'value'-s are found in "
                                   "the analyzed string for the same 'key' - they will be concatenated. That "
                                   "includes both captured and explicitly set 'value'-s.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_GRAPH_METADATA, item));
    }

    auto formGanttParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "graphId";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "eventType";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                QString lowerValue = value.toLower();
                if(lowerValue.compare( "start", Qt::CaseInsensitive ) != 0 &&
                   lowerValue.compare( "end", Qt::CaseInsensitive ) != 0)
                {
                    if(fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. Expected values for the "
                                           "'eventType' parameter are: 'start' or 'end'.").arg(value));
                    }
                    bResult = false;
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Mandatory_Gantt;
        item.id_str = s_PLOT_GANTT_EVENT;
        formGanttParameters(item);
        item.description = QString("Declares event of a certain type ( either \"start\" or \"end\" ). %1."
                                   "Used to identify start and end points of different events, which should "
                                   "be represented on the Gantt chart. "
                                   "Mandatory. One or more.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_GANTT_EVENT, item));
    }

    auto formGanttEventIdParameters = [](tPlotViewIDItem& item)
    {
        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "axisRectName";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            item.addParameter(pParameter);
        }

        {
            tPlotViewIDParameterPtr pParameter = std::make_shared<tPlotViewIDparameter>();
            pParameter->name = "graphId";
            pParameter->type = ePlotViewParameterType::e_Mandatory;
            pParameter->validationFunction = [](const QString&,
                                                ePlotViewParameterType,
                                                const QString& value,
                                                QString& msg,
                                                bool fillInStringMsg)
            {
                bool bResult = true;
                static_cast<void>(value.toInt(&bResult));
                if(false == bResult)
                {
                    if(true == fillInStringMsg)
                    {
                        msg.append(QString("Wrong literal '%1' provided. It is not convertible to integer.").arg(value));
                    }
                }
                return bResult;
            };
            item.addParameter(pParameter);
        }
    };

    {
        tPlotViewIDItem item;
        item.id_type = ePlotViewIDType::e_Optional;
        item.id_str = s_PLOT_GANTT_EVENT_ID;
        formGanttEventIdParameters(item);
        item.description = QString("This parameter specifies an event identifier. "
                                   "If specified, the analysis process will connect each event type's "
                                   "start and end points ONLY if they have the same event identifier. "
                                   "Non-consistent events will be ignored, e.g., with start ID - 5, end ID - 8. %1."
                                   "Optional. If multiple values appear - the 'last win' strategy is applied. "
                                   "Applicable only for Gantt charts.").arg(item.getParametersDescription());
        result.insert(std::make_pair(ePlotViewID::PLOT_GANTT_EVENT_ID, item));
    }

    return result;
}

const tPlotViewIDsMap sPlotViewIDsMap = createPlotIDsMap();

QString getPlotIDAsString( const ePlotViewID& val )
{
    QString result;

    auto foundValue = sPlotViewIDsMap.find(val);

    if(foundValue != sPlotViewIDsMap.end())
    {
        result = foundValue->second.id_str;
    }

    return result;
}

bool parsePlotViewIDFromString( const QString& data, ePlotViewID& val )
{
    bool bResult = false;

    if(false == data.isEmpty())
    {
        for(const auto& item : sPlotViewIDsMap)
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

QString getPlotIDTypeAsString( const ePlotViewIDType& val )
{
    QString result;

    switch( val )
    {
    case ePlotViewIDType::e_Optional:
    {
        result = "optional";
    }
    break;
    case ePlotViewIDType::e_Mandatory_Non_Gantt:
    {
        result = "mandatory_non_gantt";
    }
    break;
    case ePlotViewIDType::e_Mandatory_Gantt:
    {
        result = "mandatory_gantt";
    }
    break;
    }

    return result;
}

QString getPlotViewParameterTypeAsString( const ePlotViewParameterType& val )
{
    QString result;

    switch( val )
    {
    case ePlotViewParameterType::e_Optional:
    {
        result = "optional";
    }
    break;
    case ePlotViewParameterType::e_Mandatory:
    {
        result = "mandatory";
    }
    break;
    }

    return result;
}

QString getAxisTypeAsString( const ePlotViewAxisType& val )
{
    QString result;

    switch( val )
    {
    case ePlotViewAxisType::e_GANTT:
    {
        result = "Gantt";
    }
    break;
    case ePlotViewAxisType::e_POINT:
    {
        result = "Point";
    }
    break;
    case ePlotViewAxisType::e_LINEAR:
    {
        result = "Linear";
    }
    break;
    }

    return result;
}

bool parseAxisTypeFromString( const QString& data, ePlotViewAxisType& val )
{
    bool bResult = false;

    if( data.compare("Gantt", Qt::CaseInsensitive) == 0 )
    {
        val = ePlotViewAxisType::e_GANTT;
        bResult = true;
    }
    else if( data.compare("Point", Qt::CaseInsensitive) == 0 )
    {
        val = ePlotViewAxisType::e_POINT;
        bResult = true;
    }
    else if( data.compare("Linear", Qt::CaseInsensitive) == 0 )
    {
        val = ePlotViewAxisType::e_LINEAR;
        bResult = true;
    }

    return bResult;
}

QRegularExpression createPlotViewRegex()
{
    auto createPlotViewRegexStr = [](  ) -> QString
    {
        QString resultRegex("^(");

        auto finalIter = sPlotViewIDsMap.end();
        --finalIter;

        for( auto it = sPlotViewIDsMap.begin(); it != sPlotViewIDsMap.end(); ++it )
        {
            const auto& Plot_IDs_MapItem = *it;

            resultRegex.append(Plot_IDs_MapItem.second.id_str);

            if(it != finalIter)
            {
                resultRegex.append("|");
            }
        }

        resultRegex.append(QString(")[%1]{0,1}([\\w\\d]*)$").arg(s_PLOT_PARAMETER_DELIMITER));

        return resultRegex;
    };

    const QString plotViewRegexStr = createPlotViewRegexStr();
    return QRegularExpression (plotViewRegexStr, QRegularExpression::CaseInsensitiveOption);
}

bool checkPlotViewParameters(QString& errorMsg, bool fillInStringMsg, const tPlotViewIDParametersMap& plotViewIDParametersMap)
{
    bool bResult = true;

    for(const auto& pair : plotViewIDParametersMap)
    {
        const auto& plotViewId = pair.first;
        bool bResultTmp = checkPlotViewParameter(errorMsg,
                                                 fillInStringMsg,
                                                 plotViewId,
                                                 pair.second.pPlotViewGroupName,
                                                 pair.second.plotViewSplitParameters);
        if(true == bResult)
        {
            bResult = bResultTmp;
        }
    }

    return bResult;
}

bool checkPlotViewParameter(QString& errorMsg,
                            bool fillInStringMsg,
                            ePlotViewID plotViewId,
                            const tQStringPtr& pPlotViewGroupName,
                            const tQStringPtrVec& plotViewSplitParameters)
{
    bool bResult = true;

    auto foundPlotID = sPlotViewIDsMap.find(plotViewId);

    if(foundPlotID != sPlotViewIDsMap.end())
    {
        assert(pPlotViewGroupName != nullptr);

        const auto& mandatoryParameters = foundPlotID->second.getMandatoryParameters();
        int32_t mandatoryParametersDiff = static_cast<int>(mandatoryParameters.size()) - static_cast<int>(plotViewSplitParameters.size());

        const auto& parameters = foundPlotID->second.getParameters();
        int32_t parametersDiff = static_cast<int>(parameters.size()) - static_cast<int>(plotViewSplitParameters.size());

        if(mandatoryParametersDiff > 0)
        {
            if(true == fillInStringMsg)
            {
                errorMsg.append(QString(" <'%1' has").arg(*pPlotViewGroupName));
                for(std::size_t i = plotViewSplitParameters.size(); i < mandatoryParameters.size(); ++i)
                {
                    errorMsg.append(QString(" '%1'").arg(mandatoryParameters[i]->name));
                    if(i < mandatoryParameters.size() - 1)
                    {
                        errorMsg.append(" and");
                    }
                }
                errorMsg.append(QString(mandatoryParametersDiff > 1 ? " mandatory parameters" : " mandatory parameter" ) + " missing>");
            }
            bResult = false;
        }
        else if(parametersDiff < 0)
        {
            if(true == fillInStringMsg)
            {
                errorMsg.append(QString(" <'%1' has '%2' parameters. Expected maximum number of parameters - '%3'>")
                               .arg(*pPlotViewGroupName).arg(plotViewSplitParameters.size()).arg(parameters.size()));
            }
            bResult = false;
        }
        else
        {
            size_t paramCounter = 0;

            for(const auto& pParameter : parameters)
            {
                if(paramCounter >= plotViewSplitParameters.size())
                {
                    break;
                }

                if(pParameter->validationFunction)
                {
                    const auto& pPlotViewParameter = plotViewSplitParameters[paramCounter];
                    assert(pPlotViewParameter != nullptr);

                    QString subMsg;
                    bool bResultTmp = pParameter->validationFunction(pParameter->name, pParameter->type, *pPlotViewParameter, subMsg, fillInStringMsg);
                    if(true == bResult)
                    {
                        bResult = bResultTmp;
                    }

                    if(false == bResult)
                    {
                        if(true == fillInStringMsg)
                        {
                            errorMsg.append(QString(" <'%1'. Parameter '%2' did not pass validation: \"%3\"").arg(*pPlotViewGroupName, pParameter->name, subMsg));
                        }
                    }
                }
                ++paramCounter;
            }
        }
    }

    return bResult;
}

QStringList splitPlotViewParameters(const QString& parameters)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return parameters.split(s_PLOT_PARAMETER_DELIMITER, QString::SplitBehavior::SkipEmptyParts);
#else
    return parameters.split(s_PLOT_PARAMETER_DELIMITER, Qt::SkipEmptyParts);
#endif

}

int32_t getPlotViewParameterIndex(ePlotViewID plotViewID, const QString& parameterName)
{
    int32_t result = -1;

    auto foundPlotViewId = sPlotViewIDsMap.find(plotViewID);

    if(foundPlotViewId != sPlotViewIDsMap.end())
    {
        const auto& parameters = foundPlotViewId->second.getParameters();
        auto foundParameter = std::find_if(parameters.begin(), parameters.end(),
        [&parameterName](const tPlotViewIDParameterPtr pParameter)
        {
            if(pParameter)
            {
                return pParameter->name.compare(parameterName, Qt::CaseInsensitive) == 0;
            }
            else
            {
                return false;
            }
        });

        if(foundParameter != parameters.end())
        {
            result = foundParameter - parameters.begin();
        }
    }

    return result;
}

bool stringsToDouble(double& res, const QString& intPartStr, const QString fracPartStr)
{
    bool bResult = false;

    bool bIsInt = false;
    int32_t intPart = intPartStr.toInt(&bIsInt);

    if(true == bIsInt)
    {
        int32_t fracPart = fracPartStr.toInt(&bIsInt);
        if(true == bIsInt)
        {
            int32_t fracPartLen = fracPartStr.length();
            res = intPart + static_cast<double>(fracPart) / pow(10, fracPartLen);
            bResult = true;
        }
    }

    return bResult;
}

const tPlotViewIDItem getPlotViewIDItem(ePlotViewID plotViewID)
{
    const auto& foundPlotViewIDItem = sPlotViewIDsMap.find(plotViewID);
    assert(foundPlotViewIDItem != sPlotViewIDsMap.end());
    return foundPlotViewIDItem->second;
}
