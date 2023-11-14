/**
 * @file    CCustomPlotExtended.hpp
 * @author  vgoncharuk
 * @brief   Implementation of the CCustomPlotExtended class
 */

#include <QtSvg/QSvgGenerator>

#include "common/Definitions.hpp"

#include "../api/CCustomPlotExtended.hpp"

#include "DMA_Plantuml.hpp"

static int sDefaultOpacity = 0;
static int sMaxOpacity = 255;

static QString get_Plot_File_Name()
{
    static const QString result("plot");
    return result;
}

CCustomPlotExtended::CCustomPlotExtended(QWidget *pParent):
    QCustomPlot(pParent),
    mbGraphReorderingModeOn(false),
    mbGraphOpacityChangeModeOn(false),
    mUsedOpacity(sDefaultOpacity),
    mLastSelectedFolder(QString(".") + QDir::separator())
{
    setInteraction(QCP::iRangeDrag, true);
    setInteraction(QCP::iRangeZoom, true);

    connect(this, &QCustomPlot::legendClick, this, [&](QCPLegend*, QCPAbstractLegendItem* item, QMouseEvent* event)
    {
        QCPPlottableLegendItem *pItem = qobject_cast<QCPPlottableLegendItem*>(item);
        if (pItem && event)
        {
            if(event->button() == Qt::LeftButton)
            {
                if(true == mbGraphReorderingModeOn)
                {
                    if(true == pItem->plottable()->visible())
                    {
                        pItem->plottable()->setLayer(layer("main"));
                    }
                }
                else
                {
                    changeLegendItemVisibility(pItem);
                }
            }

            replot();
        }
    });

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Save as ...", this);
            connect(pAction, &QAction::triggered, [this]()
            {
                QString svgFilter = "Scalable vector graphics (*.svg)";
                QString selectedFilter;
                QString targetFilePath = QFileDialog::getSaveFileName(this, tr("Save as"),
                                                                      mLastSelectedFolder + get_Plot_File_Name(),
                                                                      svgFilter + ";;",
                                                                      &selectedFilter);

                if(false == targetFilePath.isEmpty())
                {
                    QFileInfo fileInfo(targetFilePath);

                    // try to remove the file if it exists
                    if(fileInfo.exists())
                    {
                        mLastSelectedFolder = fileInfo.dir().path() + QDir::separator();
                        QFile::remove(targetFilePath);
                    }

                    if(selectedFilter == svgFilter)
                    {
                        QSvgGenerator svgGen;

                        svgGen.setFileName(targetFilePath);
                        svgGen.setSize(QSize(width(), height())); // Set the dimensions of your SVG. Adjust as needed.
                        svgGen.setViewBox(QRect(0, 0, width(), height()));
                        svgGen.setTitle("DMA plot");
                        svgGen.setDescription("An SVG representation of a QCustomPlot plot.");

                        QCPPainter painter(&svgGen);
                        toPainter(&painter, width(), height());
                    }
                }
            });

            contextMenu.addAction(pAction);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    reset();
    replot();
}

void CCustomPlotExtended::changeLegendItemVisibility(QCPPlottableLegendItem* pItem)
{
    if(nullptr == pItem)
        return;

    bool targetVisibility = !pItem->plottable()->visible();

    if(true == targetVisibility)
    {
        pItem->plottable()->setVisible(targetVisibility);
        pItem->setTextColor(QColor(0,0,0));
    }
    else
    {
        const auto* pAxisRect = pItem->plottable()->keyAxis()->axisRect();

        uint32_t visiblePlotsNumber = 0u;
        bool bIsSingleVisiblePlot = true;

        for(const auto pGraph : pAxisRect->graphs())
        {
            if(true == pGraph->visible())
            {
                ++visiblePlotsNumber;

                if(visiblePlotsNumber > 1)
                {
                    bIsSingleVisiblePlot = false;
                    break;
                }
            }
        }

        if(false == bIsSingleVisiblePlot)
        {
            pItem->plottable()->setVisible(targetVisibility);
            pItem->setTextColor(QColor(200, 200, 200));
        }
    }
}

void CCustomPlotExtended::reset()
{
    clearGraphs();
    clearPlottables();
    clearItems();
    plotLayout()->clear();

    mUsedOpacity = sDefaultOpacity;
    mbGraphReorderingModeOn = false;
    mbGraphOpacityChangeModeOn = false;
    mPlotAxisRectDataMap.clear();
}

void CCustomPlotExtended::updateOpactity()
{
    for (const auto& axisRectPair : mPlotAxisRectDataMap)
    {
        auto* pAxisRect = axisRectPair.first;
        const auto& axisRectType = axisRectPair.second.plotViewAxisType;

        for(auto* pGraph : pAxisRect->graphs())
        {
            if(axisRectType == ePlotViewAxisType::e_LINEAR)
            {
                // Adjusting the pen (line) opacity
                QPen pen = pGraph->pen();
                QColor penColor = pen.color();
                penColor.setAlpha(mUsedOpacity ? mUsedOpacity : sMaxOpacity);
                pen.setColor(penColor);
                pGraph->setPen(pen);

                // If you also want to adjust the brush (fill) opacity
                QBrush brush = pGraph->brush();
                QColor brushColor = brush.color();
                brushColor.setAlpha(mUsedOpacity);
                brush.setColor(brushColor);
                pGraph->setBrush(brush);
            }
            else if(axisRectType == ePlotViewAxisType::e_POINT)
            {
                // turn off lines and filling
                pGraph->setPen(Qt::NoPen);
                pGraph->setBrush(Qt::NoBrush);
            }
        }
    }
}

bool CCustomPlotExtended::setAxisRectRescaleData(QCPAxisRect* pAxisRect,
                         const double& xOffsetFactor,
                         const double& yOffsetFactor,
                         const TOptional<tPlotData>& xMin,
                         const TOptional<tPlotData>& xMax,
                         const TOptional<tPlotData>& yMin,
                         const TOptional<tPlotData>& yMax)
{
    if(nullptr == pAxisRect)
    {
        qDebug() << Q_FUNC_INFO << "passed axisRect is nullptr";
        return false;
    }

    if(false == axisRects().contains(pAxisRect))
    {
        qDebug() << Q_FUNC_INFO << "axis rect not in list:" << reinterpret_cast<quintptr>(pAxisRect);
        return false;
    }

    tAxisRectRescaleData rescaleData;

    rescaleData.xOffsetFactor = xOffsetFactor;
    rescaleData.yOffsetFactor = yOffsetFactor;
    rescaleData.xMin = xMin;
    rescaleData.xMax = xMax;
    rescaleData.yMin = yMin;
    rescaleData.yMax = yMax;

    mPlotAxisRectDataMap[pAxisRect].axisRectRescaleData = rescaleData;

    return true;
}

void CCustomPlotExtended::rescaleExtended()
{
    rescaleXExtended();
    rescaleYExtended();
}

void CCustomPlotExtended::rescaleXExtended()
{
    for(auto& axisRectRescaleDataPair : mPlotAxisRectDataMap)
    {
        auto * pAxisRect = axisRectRescaleDataPair.first;

        assert(nullptr != pAxisRect);

        const auto& axisRectRescaleData = axisRectRescaleDataPair.second.axisRectRescaleData;

        auto* pBottomAxis = pAxisRect->axis(QCPAxis::atBottom);

        pBottomAxis->rescale();

        {
            // Slightly rescale X axis so that max and min points are more visible.
            QCPRange scaleRange = pBottomAxis->range();
            double offset = ( scaleRange.upper - scaleRange.lower ) * axisRectRescaleData.xOffsetFactor;
            scaleRange.upper += offset;
            scaleRange.lower -= offset;
            pBottomAxis->setRange(scaleRange);
        }

        if(true == axisRectRescaleData.xMin.isSet())
        {
            pBottomAxis->setRangeLower(axisRectRescaleData.xMin.getValue());
        }

        if(true == axisRectRescaleData.xMax.isSet())
        {
            pBottomAxis->setRangeUpper(axisRectRescaleData.xMax.getValue());
        }
    }
}

void CCustomPlotExtended::rescaleYExtended()
{
    for(auto& axisRectRescaleDataPair : mPlotAxisRectDataMap)
    {
        auto * pAxisRect = axisRectRescaleDataPair.first;

        assert(nullptr != pAxisRect);

        const auto& axisRectRescaleData = axisRectRescaleDataPair.second.axisRectRescaleData;

        auto* pLeftAxis = pAxisRect->axis(QCPAxis::atLeft);

        pLeftAxis->rescale();

        {
            // Slightly rescale Y axis so that max and min points are more visible.
            QCPRange scaleRange = pLeftAxis->range();
            double offset = ( scaleRange.upper - scaleRange.lower ) * axisRectRescaleData.yOffsetFactor;
            scaleRange.upper += offset;
            scaleRange.lower -= offset;
            pLeftAxis->setRange(scaleRange);
        }

        if(true == axisRectRescaleData.yMin.isSet())
        {
            pLeftAxis->setRangeLower(axisRectRescaleData.yMin.getValue());
        }

        if(true == axisRectRescaleData.yMax.isSet())
        {
            pLeftAxis->setRangeUpper(axisRectRescaleData.yMax.getValue());
        }
    }
}

void CCustomPlotExtended::wheelEvent(QWheelEvent *event)
{
    if(event)
    {
        if(true == mbGraphOpacityChangeModeOn)
        {
            auto delta = event->angleDelta();

            mUsedOpacity = mUsedOpacity + ( delta.y() * 0.1 ) ;
            if(mUsedOpacity < 0) mUsedOpacity = 0;
            if(mUsedOpacity > 255) mUsedOpacity = 255;

            updateOpactity();
            replot();
        }
        else
        {
            QCustomPlot::wheelEvent(event); // Call the base class handler
        }
    }
}

CCustomPlotExtended::~CCustomPlotExtended()
{}

void CCustomPlotExtended::keyPressEvent(QKeyEvent *event)
{
    if(false == event->isAutoRepeat())
    {
        if(event->key() == Qt::Key_Shift)
        {
            auto axisRectsList = axisRects();
            for(auto* pAxisRect : axisRectsList)
            {
                pAxisRect->setRangeDrag(Qt::Orientation::Vertical);
                pAxisRect->setRangeZoom(Qt::Orientation::Vertical);
            }
        }
        else if(event->key() == Qt::Key_Control)
        {
            mbGraphReorderingModeOn = true;
            mbGraphOpacityChangeModeOn = true;
        }
        else if(event->key() == Qt::Key_F)
        {
            filterGraphBySelectedItem();
        }
    }

    QCustomPlot::keyPressEvent(event); // Call the base class handler
}

void CCustomPlotExtended::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift)
    {
        auto axisRectsList = axisRects();
        for(auto* pAxisRect : axisRectsList)
        {
            pAxisRect->setRangeDrag(Qt::Orientation::Horizontal);
            pAxisRect->setRangeZoom(Qt::Orientation::Horizontal);
        }
    }
    else if (event->key() == Qt::Key_Space)
    {
        rescaleYExtended();
        replot();
    }
    else if (event->key() == Qt::Key_Control)
    {
        mbGraphReorderingModeOn = false;
        mbGraphOpacityChangeModeOn = false;
    }
    QCustomPlot::keyReleaseEvent(event); // Call the base class handler
}

void CCustomPlotExtended::resizeEvent(QResizeEvent* event)
{
    QCustomPlot::resizeEvent(event);

    for(const auto* pAxisRect : axisRects())
    {
        auto pLeftAxis = pAxisRect->axis(QCPAxis::atLeft);

        if(nullptr != pLeftAxis)
        {
            QFont labelFont = pLeftAxis->labelFont();
            auto labelSize = pLeftAxis->label().size();
            labelSize = labelSize ? labelSize : 1;
            auto fontSize = pAxisRect->height() / labelSize * 1.35;
            if(fontSize > 15.0)
                fontSize = 15.0;
            labelFont.setPointSize(fontSize);
            pLeftAxis->setLabelFont(labelFont);
        }
    }

    QCustomPlot::resizeEvent(event);
}

void CCustomPlotExtended::appendMetadata(QCPAxisRect* pAxisRect,
                                         QCPGraph* pGraph,
                                         const tPlotData& x,
                                         const tPlotData& y,
                                         const tPlotGraphMetadataMap& plotGraphMetadataMap)
{
    mPlotAxisRectDataMap[pAxisRect].plotGraphsMetadataMap[pGraph][std::make_pair(x,y)] = plotGraphMetadataMap;
}

std::pair< bool, const tPlotGraphMetadataMap* > CCustomPlotExtended::getMetadata(QCPAxisRect* pAxisRect,
                                         QCPGraph* pGraph,
                                         const tPlotData& x,
                                         const tPlotData& y)
{
    std::pair< bool, const tPlotGraphMetadataMap* > result;

    result.first = false;
    result.second = nullptr;

    auto foundAxisRect = mPlotAxisRectDataMap.find(pAxisRect);

    if(foundAxisRect != mPlotAxisRectDataMap.end())
    {
        auto foundGraph = foundAxisRect->second.plotGraphsMetadataMap.find(pGraph);

        if(foundGraph != foundAxisRect->second.plotGraphsMetadataMap.end())
        {
            auto foundPoint = foundGraph->second.find(std::make_pair(x,y));

            if(foundPoint != foundGraph->second.end())
            {
                result.first = true;
                result.second = &foundPoint->second;
            }
        }
    }

    return result;
}

bool CCustomPlotExtended::setAxisRectType(QCPAxisRect* pAxisRect, ePlotViewAxisType plotViewAxisType)
{
    if(nullptr == pAxisRect)
    {
        qDebug() << Q_FUNC_INFO << "passed axisRect is nullptr";
        return false;
    }

    if(false == axisRects().contains(pAxisRect))
    {
        qDebug() << Q_FUNC_INFO << "axis rect not in list:" << reinterpret_cast<quintptr>(pAxisRect);
        return false;
    }

    mPlotAxisRectDataMap[pAxisRect].plotViewAxisType = plotViewAxisType;

    return true;
}

std::pair<bool, ePlotViewAxisType> CCustomPlotExtended::getAxisRectType(QCPAxisRect* pAxisRect) const
{
    std::pair<bool, ePlotViewAxisType> result;
    result.first = false;
    result.second = ePlotViewAxisType::e_LINEAR;

    if(nullptr == pAxisRect)
    {
        qDebug() << Q_FUNC_INFO << "passed axisRect is nullptr";
        return result;
    }

    if(false == axisRects().contains(pAxisRect))
    {
        qDebug() << Q_FUNC_INFO << "axis rect not in list:" << reinterpret_cast<quintptr>(pAxisRect);
        return result;
    }

    auto foundAxisRect = mPlotAxisRectDataMap.find(pAxisRect);

    if(foundAxisRect != mPlotAxisRectDataMap.end())
    {
        result.first = true;
        result.second = foundAxisRect->second.plotViewAxisType;
    }

    return result;
}

void CCustomPlotExtended::filterGraphBySelectedItem()
{
    int numberOfVisibleGraphs = 0;
    QCPPlottableLegendItem* pSelectedLegendItem = nullptr;
    const QCPLegend* pSelectedLegend = nullptr;

    for(const auto& axisRectPair : mPlotAxisRectDataMap)
    {
        bool leaveCycle = false;

        const auto* pLegend = axisRectPair.second.pLegend;

        if(nullptr != pLegend)
        {
            for (int i = 0; i < pLegend->itemCount(); ++i)
            {
                QCPPlottableLegendItem* pItem = qobject_cast<QCPPlottableLegendItem*>(pLegend->item(i));
                if (nullptr != pItem)
                {
                    if(true == pItem->plottable()->visible())
                    {
                        ++numberOfVisibleGraphs;
                    }

                    if(nullptr == pSelectedLegendItem || nullptr == pSelectedLegend)
                    {
                        if(false == pItem->plottable()->selection().isEmpty())
                        {
                            pSelectedLegendItem = pItem;
                            pSelectedLegend = pLegend;
                        }
                    }

                    if(numberOfVisibleGraphs > 1 &&
                       nullptr != pSelectedLegendItem &&
                       nullptr != pSelectedLegend)
                    {
                        leaveCycle = true;
                        break;
                    }
                }
            }

            if(true == leaveCycle)
            {
                break;
            }
        }
    }

    bool bReplot = false;

    if(nullptr != pSelectedLegend)
    {
        bool bFilterOut = numberOfVisibleGraphs > 1;

        for (int i = 0; i < pSelectedLegend->itemCount(); ++i)
        {
            QCPPlottableLegendItem* pItem = qobject_cast<QCPPlottableLegendItem*>(pSelectedLegend->item(i));

            if(nullptr != pItem)
            {
                if(true == bFilterOut)
                {
                    if(true == pItem->plottable()->visible() && pItem != pSelectedLegendItem)
                    {
                        changeLegendItemVisibility(pItem);
                        bReplot = true;
                    }
                }
                else
                {
                    if(false == pItem->plottable()->visible())
                    {
                        changeLegendItemVisibility(pItem);
                        bReplot = true;
                    }
                }
            }
        }
    }
    else
    {
        for(const auto& axisRectPair : mPlotAxisRectDataMap)
        {
            const auto* pLegend = axisRectPair.second.pLegend;

            if(nullptr != pLegend)
            {
                for (int i = 0; i < pLegend->itemCount(); ++i)
                {
                    QCPPlottableLegendItem* pItem = qobject_cast<QCPPlottableLegendItem*>(pLegend->item(i));
                    if (nullptr != pItem)
                    {
                        if(false == pItem->plottable()->visible())
                        {
                            changeLegendItemVisibility(pItem);
                            bReplot = true;
                        }
                    }
                }
            }
        }
    }

    if(true == bReplot)
    {
        replot();
    }
}

bool CCustomPlotExtended::setLegendExtended(QCPAxisRect* pAxisRect,
                       QCPLegend* pLegend)
{
    if(nullptr == pAxisRect)
    {
        qDebug() << Q_FUNC_INFO << "passed axisRect is nullptr";
        return false;
    }

    if(nullptr == pLegend)
    {
        qDebug() << Q_FUNC_INFO << "passed pLegend is nullptr";
        return false;
    }

    if(false == axisRects().contains(pAxisRect))
    {
        qDebug() << Q_FUNC_INFO << "axis rect not in list:" << reinterpret_cast<quintptr>(pAxisRect);
        return false;
    }

    mPlotAxisRectDataMap[pAxisRect].pLegend = pLegend;
    return true;
}

PUML_PACKAGE_BEGIN(DMA_PlotView_API)
    PUML_CLASS_BEGIN_CHECKED(CCustomPlotExtended)
        PUML_INHERITANCE_CHECKED(QCustomPlot, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QCPLegend, 1, *, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QCPAxisRect, 1, *, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QCPGraph, 1, *, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
