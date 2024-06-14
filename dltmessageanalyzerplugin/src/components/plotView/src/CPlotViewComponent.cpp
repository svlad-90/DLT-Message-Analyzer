/**
 * @file    CPlotViewComponent.hpp
 * @author  vgoncharuk
 * @brief   Implementation of the CPlotViewComponent class
 */

#include <QPushButton>
#include "components/log/api/CLog.hpp"

#include "../api/CPlotViewComponent.hpp"
#include "components/searchView/api/ISearchResultModel.hpp"
#include "components/plotView/api/CCustomPlotExtended.hpp"

#include "DMA_Plantuml.hpp"

CPlotViewComponent::CPlotViewComponent( CCustomPlotExtended* pPlot,
                                        QPushButton* pCreatePlotButton,
                                        std::shared_ptr<ISearchResultModel> pSearchResultModel,
                                        const tSettingsManagerPtr& pSettingsManager ):
QObject(),
CSettingsManagerClient(pSettingsManager),
mpPlot(pPlot),
mpCreatePlotButton(pCreatePlotButton),
mpSearchResultModel(pSearchResultModel)
{
}

const char* CPlotViewComponent::getName() const
{
    return "CPlotViewComponent";
}

CCustomPlotExtended* CPlotViewComponent::getPlot()
{
    return mpPlot;
}

DMA::tSyncInitOperationResult CPlotViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(mpCreatePlotButton != nullptr && mpPlot != nullptr)
        {
            connect(mpCreatePlotButton, &QPushButton::clicked, this, [this]()
            {
                generatePlot();
            });

            mpPlot->setInteraction(QCP::iSelectPlottables, true);

            // add logic here, when it will be needed
            result.bIsOperationSuccessful = true;
            result.returnCode = 0;
        }
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

DMA::tSyncInitOperationResult CPlotViewComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        // add logic here, when it will be needed
        result.bIsOperationSuccessful = true;
        result.returnCode = 0;
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

double interpolateY(QCPGraph *graph, double xValue)
{
    if (!graph || graph->data()->size() == 0)
        return std::numeric_limits<double>::quiet_NaN();  // Return NaN if graph is null or empty.

    // First, find two points (X1, Y1) and (X2, Y2) such that X1 <= xValue <= X2
    QCPDataContainer<QCPGraphData>::const_iterator it = graph->data()->findBegin(xValue, false);

    if (it == graph->data()->constEnd())
        --it;  // If we're past the end, take the last point.

    QCPDataContainer<QCPGraphData>::const_iterator itPrev = (it == graph->data()->constBegin()) ? it : it - 1;

    if (it->key == xValue)
    {
        // If xValue coincides with a data point, return its y value directly
        return it->value;
    }

    if (itPrev == it || it->key < xValue)
    {
        // We're either at the beginning of the data or past the end.
        // In either case, we can't interpolate.
        return 0.0;
    }

    // Linear interpolation formula
    double slope = (it->value - itPrev->value) / (it->key - itPrev->key);
    return itPrev->value + slope * (xValue - itPrev->key);
}

void stackData(QCPGraph* previousGraph, QVector<double>& xData, QVector<double>& yData)
{
    assert(xData.size() == yData.size());

    for(int i = 0; i < xData.size(); ++i)
    {
        auto prevGraphInterpolatedY = interpolateY(previousGraph, xData[i]);
        yData[i] += prevGraphInterpolatedY;
    }
}

void generateAxisRectGantt(const std::pair<ISearchResultModel::tPlotAxisName, ISearchResultModel::tPlotAxisItem> plotAxisPair,
                      CCustomPlotExtended* pPlot,
                      const uint32_t& rowCounter,
                      CPlotViewComponent*)
{
    // TBD
    const auto& plotAxis = plotAxisPair.second;
    QCPAxisRect *pAxisRect = new QCPAxisRect(pPlot);
    pAxisRect->setMinimumSize(QSize(0, 400));
    QCPLegend *pLegend = new QCPLegend;
    pLegend->setVisible(true);
    pLegend->setBorderPen(QPen(QColor(0,0,0,230)));
    pLegend->setBrush(QBrush(QColor(255,255,255,230)));  // Background color
    pLegend->setSelectableParts(QCPLegend::spItems);     // Make legend items selectable
    pAxisRect->insetLayout()->addElement(pLegend, Qt::AlignRight|Qt::AlignTop);  // Adjust position as needed
    pLegend->setLayer(QLatin1String("legend"));

    pAxisRect->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);  // Position of legend
    auto* pLeftAxis = pAxisRect->axis(QCPAxis::atLeft);
    auto* pBottomAxis = pAxisRect->axis(QCPAxis::atBottom);

    pPlot->plotLayout()->addElement(rowCounter, 0, pAxisRect);
    pAxisRect->axis(QCPAxis::atBottom)->setLayer("axes");
    pAxisRect->axis(QCPAxis::atBottom)->grid()->setLayer("grid");
    // bring bottom and main axis rect closer together:
    pAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    pAxisRect->setMargins(QMargins(0, 0, 0, 0));

    {
        QString xLabel;

        if(true == plotAxis.xName.isSet())
        {
            xLabel.append(plotAxis.xName.getValue());
        }

        if(true == plotAxis.xUnit.isSet())
        {
            if(false == xLabel.isEmpty())
            {
                xLabel.append(", ");
            }
            xLabel.append(plotAxis.xUnit.getValue());
        }

        if(false == xLabel.isEmpty())
        {
            pBottomAxis->setLabel(xLabel);
        }
    }

    {
        QString yLabel;

        if(true == plotAxis.yName.isSet())
        {
            yLabel.append(plotAxis.yName.getValue());
        }

        if(true == plotAxis.yUnit.isSet())
        {
            if(false == yLabel.isEmpty())
            {
                yLabel.append(", ");
            }
            yLabel.append(plotAxis.yUnit.getValue());
        }

        if(false == yLabel.isEmpty())
        {
            pLeftAxis->setLabel(yLabel);
        }
    }

    if(true == plotAxis.axisType.isSet())
    {
        pPlot->setAxisRectType(pAxisRect, plotAxis.axisType.getValue());
    }
    else
    {
        pPlot->setAxisRectType(pAxisRect, ePlotViewAxisType::e_LINEAR);
    }
}

void generateAxisRect(const std::pair<ISearchResultModel::tPlotAxisName, ISearchResultModel::tPlotAxisItem> plotAxisPair,
                      CCustomPlotExtended* pPlot,
                      const uint32_t& rowCounter,
                      CPlotViewComponent* pPlotViewComponent)
{
    const auto& plotAxis = plotAxisPair.second;
    QCPAxisRect *pAxisRect = new QCPAxisRect(pPlot);
    pAxisRect->setMinimumSize(QSize(0, 200));
    QCPLegend *pLegend = new QCPLegend;
    pLegend->setVisible(true);
    pLegend->setBorderPen(QPen(QColor(0,0,0,230)));
    pLegend->setBrush(QBrush(QColor(255,255,255,230)));  // Background color
    pLegend->setSelectableParts(QCPLegend::spItems);     // Make legend items selectable
    pAxisRect->insetLayout()->addElement(pLegend, Qt::AlignRight|Qt::AlignTop);  // Adjust position as needed
    pLegend->setLayer(QLatin1String("legend"));

    auto* pLeftAxis = pAxisRect->axis(QCPAxis::atLeft);
    auto* pBottomAxis = pAxisRect->axis(QCPAxis::atBottom);

    pPlot->plotLayout()->addElement(rowCounter, 0, pAxisRect);
    pAxisRect->axis(QCPAxis::atBottom)->setLayer("axes");
    pAxisRect->axis(QCPAxis::atBottom)->grid()->setLayer("grid");

    {
        if (plotAxis.axisLabel.isSet())
        {
            QCPTextElement *pTitleLabel = new QCPTextElement(pPlot, plotAxis.axisLabel.getValue(), QFont("sans", 12, QFont::Bold));
            pTitleLabel->setVisible(true);
            pAxisRect->insetLayout()->addElement(pTitleLabel, QRectF(0.005, 0.0, 0.0, 0.0));
            pTitleLabel->setLayer(QLatin1String("axis_rect_label"));
            pTitleLabel->setSelectable(false);
        }
    }

    // bring bottom and main axis rect closer together:
    pAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    pAxisRect->setMargins(QMargins(0, 0, 0, 0));

    {
        QString xLabel;

        if(true == plotAxis.xName.isSet())
        {
            xLabel.append(plotAxis.xName.getValue());
        }

        if(true == plotAxis.xUnit.isSet())
        {
            if(false == xLabel.isEmpty())
            {
                xLabel.append(", ");
            }
            xLabel.append(plotAxis.xUnit.getValue());
        }

        if(false == xLabel.isEmpty())
        {
            pBottomAxis->setLabel(xLabel);
        }
    }

    {
        QString yLabel;

        if(true == plotAxis.yName.isSet())
        {
            yLabel.append(plotAxis.yName.getValue());
        }

        if(true == plotAxis.yUnit.isSet())
        {
            if(false == yLabel.isEmpty())
            {
                yLabel.append(", ");
            }
            yLabel.append(plotAxis.yUnit.getValue());
        }

        if(false == yLabel.isEmpty())
        {
            pLeftAxis->setLabel(yLabel);
        }
    }

    if(true == plotAxis.axisType.isSet())
    {
        pPlot->setAxisRectType(pAxisRect, plotAxis.axisType.getValue());
    }
    else
    {
        pPlot->setAxisRectType(pAxisRect, ePlotViewAxisType::e_LINEAR);
    }

    QList<QCPGraph*> graphOrder;
    QOptionalColor prevUsedColor;

    for(const auto& plotGraphItemPair : plotAxis.plotGraphItemMap)
    {
        const auto& plotGraphItem = plotGraphItemPair.second;

        for(const auto& plotGraphSubItemPair : plotGraphItem.plotGraphSubItemMap)
        {
            const auto& plotGraphSubItem = plotGraphSubItemPair.second;

            std::shared_ptr< std::map<std::pair<tPlotData, tPlotData>, tMsgId> > pxToMsgIdMap =
                std::make_shared<std::map<std::pair<tPlotData, tPlotData>, tMsgId>>();

            auto* pGraph = pPlot->addGraph();

            pGraph->addToLegend(pLegend);

            pGraph->setName(plotGraphSubItemPair.first);
            graphOrder.prepend(pGraph);
            pGraph->setKeyAxis(pAxisRect->axis(QCPAxis::atBottom));
            pGraph->setValueAxis(pAxisRect->axis(QCPAxis::atLeft));

            QVector<double> xData, yData;

            for(const auto& dataItem : plotGraphSubItem.dataItems)
            {
                xData.push_back(dataItem.getX());
                yData.push_back(dataItem.getY());
                pxToMsgIdMap->insert(std::make_pair(std::make_pair(dataItem.getX(), dataItem.getY()), dataItem.getMsgId()));
                pPlot->appendMetadata(pAxisRect,
                                       pGraph,
                                       dataItem.getX(),
                                       dataItem.getY(),
                                       dataItem.getPlotGraphMetadataMap());
            }

            pGraph->setData(xData, yData);

            {
                QOptionalColor usedXColor;
                QOptionalColor usedYColor;

                if(true == plotGraphSubItem.xOptColor.isSet &&
                    true == prevUsedColor.isSet &&
                    plotGraphSubItem.xOptColor.color_code == prevUsedColor.color_code)
                {
                    usedXColor.isSet = true;
                    usedXColor.color_code = getChartColor().rgb();
                }
                else
                {
                    usedXColor = plotGraphSubItem.xOptColor;
                }

                if(true == plotGraphSubItem.yOptColor.isSet &&
                    true == prevUsedColor.isSet &&
                    plotGraphSubItem.yOptColor.color_code == prevUsedColor.color_code)
                {
                    usedYColor.isSet = true;
                    usedYColor.color_code = getChartColor().rgb();
                }
                else
                {
                    usedYColor = plotGraphSubItem.yOptColor;
                }

                QOptionalColor usedColor;

                if( ( true == usedXColor.isSet &&
                     false == usedYColor.isSet ) ||
                    ( true == usedXColor.isSet &&
                     true == usedYColor.isSet ) )
                {
                    usedColor = usedXColor;
                }
                else if(false == usedXColor.isSet &&
                         true == usedYColor.isSet)
                {
                    usedColor = usedYColor;
                }
                else
                {
                    usedColor.color_code = getChartColor().rgb();
                    usedColor.isSet = true;
                }

                pGraph->setBrush(QColor(usedColor.color_code));
                pGraph->setPen(QColor(usedColor.color_code));

                auto getAxisTypeResult = pPlot->getAxisRectType(pAxisRect);
                if(true == getAxisTypeResult.first && getAxisTypeResult.second == ePlotViewAxisType::e_POINT)
                {
                    pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(0,0,0), QColor(usedColor.color_code), 6));
                }
                else
                {
                    pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::white, Qt::black, 6));
                }

                prevUsedColor = usedColor;
            }

            pGraph->setSelectable(QCP::SelectionType::stSingleData);

            auto* pSelectionDecorator = new QCPSelectionDecorator();
            pSelectionDecorator->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, Qt::red, 15),
                                                 QCPScatterStyle::ScatterProperty::spAll);
            pGraph->setSelectionDecorator(pSelectionDecorator);

            QCPItemTracer* pItemTracer = new QCPItemTracer(pPlot);
            pItemTracer->setSize(0);
            pItemTracer->setStyle(QCPItemTracer::tsCircle); // set style to circle
            pItemTracer->setLayer("overlay");
            pItemTracer->setPen(QPen(Qt::red));
            pItemTracer->setBrush(Qt::red);
            pItemTracer->setClipAxisRect(pAxisRect);
            pItemTracer->position->setAxisRect(pAxisRect);
            pItemTracer->position->setAxes(pGraph->keyAxis(), pGraph->valueAxis());
            QCPItemText* pTextLabel = new QCPItemText(pPlot);
            pTextLabel->setVisible(false);
            pTextLabel->setLayer("overlay");
            pTextLabel->setBrush(QColor(240,240,240));
            pTextLabel->setPen(QColor(0,0,0));
            pTextLabel->setColor(QColor(0,0,0));
            pTextLabel->setClipAxisRect(pAxisRect);
            pTextLabel->position->setParentAnchor(pItemTracer->position);
            pTextLabel->position->setAxisRect(pAxisRect);
            pTextLabel->position->setAxes(pGraph->keyAxis(), pGraph->valueAxis());

            pPlotViewComponent->connect(pGraph, static_cast<void (QCPAbstractPlottable::*)(const QCPDataSelection &)>(&QCPAbstractPlottable::selectionChanged),
            pPlotViewComponent, [pPlotViewComponent, pPlot, pAxisRect, pGraph, pxToMsgIdMap, pItemTracer, pTextLabel, pLeftAxis](const QCPDataSelection &selection)
            {
                if(false == selection.isEmpty())
                {
                    QCPDataRange dataRange = selection.dataRange();

                    // For simplicity, let's just handle the beginning of the range (you might want to handle the entire range if you allow multi-selection)
                    int index = dataRange.begin();

                    if (index >= 0 && index < pGraph->data()->size()) // Check if index is valid
                    {
                        QCPGraphDataContainer::const_iterator it = pGraph->data()->at(index);
                        double x = it->key;
                        double y = it->value;

                        pItemTracer->position->setCoords(x, y);
                        pTextLabel->setVisible(true);

                        QString labelText;

                        labelText.append("value: " + QString::number(pItemTracer->position->value()));

                        auto metadata = pPlot->getMetadata(pAxisRect,
                                                            pGraph,
                                                            x,
                                                            y);

                        int numberOfLines = 1;

                        if(true == metadata.first)
                        {
                            assert(nullptr != metadata.second);

                            for(const auto& metadataPair : *metadata.second)
                            {
                                if(nullptr != metadataPair.first.pString &&
                                    nullptr != metadataPair.second.pString)
                                {
                                    labelText.append("\n");

                                    if(false == metadataPair.first.pString->isEmpty())
                                    {
                                        labelText.append(*metadataPair.first.pString + ": ");
                                    }

                                    labelText.append(*metadataPair.second.pString);
                                    ++numberOfLines;
                                }
                            }
                        }

                        pTextLabel->setText(labelText);
                        pTextLabel->position->setCoords(0, ( numberOfLines + 1 ) * -10);

                        {
                            // Get top and bottom pixel positions
                            auto topPixelPos = pTextLabel->top->pixelPosition();
                            auto bottomPixelPos = pTextLabel->bottom->pixelPosition();

                            // Convert pixel positions to coordinate values
                            double topCoord = pLeftAxis->pixelToCoord(topPixelPos.y());
                            double bottomCoord = pLeftAxis->pixelToCoord(bottomPixelPos.y());

                            // Get y-axis range
                            double yAxisLower = pLeftAxis->range().lower;
                            double yAxisUpper = pLeftAxis->range().upper;

                            bool isTextFullyVisible = (topCoord <= yAxisUpper) && (bottomCoord >= yAxisLower);

                            if(false == isTextFullyVisible)
                            {
                                pTextLabel->position->setCoords(0, ( numberOfLines + 1 ) * 10);
                            }
                        }

                        auto foundMsgId = pxToMsgIdMap->find(std::make_pair(x, y));

                        if(foundMsgId != pxToMsgIdMap->end())
                        {
                            emit pPlotViewComponent->messageIdSelected(foundMsgId->second);

                            pPlot->setFocus();
                        }
                    }
                }
                else
                {
                    if(nullptr != pTextLabel)
                    {
                        pTextLabel->setVisible(false);
                    }
                }
            });
        }
    }

    for (int i = 0; i < graphOrder.size(); ++i)
    {
        graphOrder[i]->setLayer(pPlot->layer("main"));
    }

    pBottomAxis->setNumberFormat("f");
    pBottomAxis->setNumberPrecision(6);

    pPlot->setAxisRectRescaleData(pAxisRect,
                                   0.05,
                                   0.1,
                                   plotAxis.xMin,
                                   plotAxis.xMax,
                                   plotAxis.yMin,
                                   plotAxis.yMax);

    pPlot->rescaleExtended();

    pAxisRect->setRangeDrag(Qt::Orientation::Horizontal);
    pAxisRect->setRangeZoom(Qt::Orientation::Horizontal);

    pPlot->setLegendExtended(pAxisRect, pLegend);
}

void CPlotViewComponent::generatePlot()
{
    auto plotContent = mpSearchResultModel->createPlotContent();

    if(nullptr != mpPlot)
    {
        mpPlot->reset();
        mpPlot->setAutoAddPlottableToLegend(false);

        uint32_t rowCounter = 0;

        for(const auto& plotAxisPair : plotContent.plotAxisMap)
        {
            if(true == plotAxisPair.second.axisType.isSet() &&
               plotAxisPair.second.axisType.getValue() == ePlotViewAxisType::e_GANTT)
            {
                generateAxisRectGantt(plotAxisPair, mpPlot, rowCounter, this);
            }
            else
            {
                generateAxisRect(plotAxisPair, mpPlot, rowCounter, this);
            }
            ++rowCounter;
        }

        auto axisRects = mpPlot->axisRects();
        for(const auto* pAxisRectFrom : axisRects)
        {
            for(const auto* pAxisRectTo : axisRects)
            {
                if(pAxisRectFrom != pAxisRectTo)
                {
                    QObject::connect(pAxisRectFrom->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)),
                                     pAxisRectTo->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
                }
            }
        }

        mpPlot->plotLayout()->setRowSpacing(0);
        mpPlot->updateOpactity();
        mpPlot->replot();

        QSize currentSize = mpPlot->size();
        QResizeEvent resizeEvent(currentSize, currentSize);
        QCoreApplication::sendEvent(mpPlot, &resizeEvent);
    }
}

PUML_PACKAGE_BEGIN(DMA_PlotView_API)
    PUML_CLASS_BEGIN(CPlotViewComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CCustomPlotExtended, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ISearchResultModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QPushButton, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
