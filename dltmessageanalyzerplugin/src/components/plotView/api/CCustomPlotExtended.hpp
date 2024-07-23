/**
 * @file    CCustomPlotExtended.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CPlotViewComponent class
 */

#pragma once

#include "common/PlotDefinitions.hpp"

#include "qcustomplot.h"

class CCustomPlotExtended : public QCustomPlot
{
Q_OBJECT
public:
    explicit CCustomPlotExtended(QWidget *pParent = nullptr);
    virtual ~CCustomPlotExtended() override;
    void reset();
    void updateOpactity();
    bool setAxisRectRescaleData(QCPAxisRect* pAxisRect,
                                const double& xOffsetFactor,
                                const double& yOffsetFactor,
                                const TOptional<tPlotData>& xMin,
                                const TOptional<tPlotData>& xMax,
                                const TOptional<tPlotData>& yMin,
                                const TOptional<tPlotData>& yMax);
    void rescaleExtended();
    void rescaleXExtended();
    void rescaleYExtended();

    void appendMetadata(QCPAxisRect* pAxisRect,
                        QCPAbstractPlottable* pGraph,
                        const tPlotData& x,
                        const tPlotData& y,
                        const tPlotGraphMetadataMap& plotGraphMetadataMap);

    bool setLegendExtended(QCPAxisRect* pAxisRect,
                           QCPLegend* pLegend);

    std::pair< bool, const tPlotGraphMetadataMap* > getMetadata(QCPAxisRect* pAxisRect,
                                                               QCPAbstractPlottable* pGraph,
                                                               const tPlotData& x,
                                                               const tPlotData& y);

    bool setAxisRectType(QCPAxisRect* pAxisRect, ePlotViewAxisType plotViewAxisType);
    std::pair<bool, ePlotViewAxisType> getAxisRectType(QCPAxisRect* pAxisRect) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;

private:

    void filterGraphBySelectedItem();
    void changeLegendItemVisibility(QCPPlottableLegendItem* pItem);

    bool mbGraphReorderingModeOn;
    bool mbGraphOpacityChangeModeOn;
    int mUsedOpacity;
    QString mLastSelectedFolder;

    struct tAxisRectRescaleData
    {
        double xOffsetFactor;
        double yOffsetFactor;
        TOptional<tPlotData> xMin;
        TOptional<tPlotData> xMax;
        TOptional<tPlotData> yMin;
        TOptional<tPlotData> yMax;
    };

    typedef std::map<std::pair<tPlotData, tPlotData>, tPlotGraphMetadataMap> tPlotPointMetadataMap;
    typedef std::map<QCPAbstractPlottable*, tPlotPointMetadataMap> tPlotGraphsMetadataMap;

    struct tAxisRectData
    {
        QCPLegend* pLegend;
        tAxisRectRescaleData axisRectRescaleData;
        ePlotViewAxisType plotViewAxisType = ePlotViewAxisType::e_LINEAR;
        tPlotGraphsMetadataMap plotGraphsMetadataMap;
    };

    typedef std::map<QCPAxisRect*, tAxisRectData> tPlotAxisRectDataMap;
    tPlotAxisRectDataMap mPlotAxisRectDataMap;
};
