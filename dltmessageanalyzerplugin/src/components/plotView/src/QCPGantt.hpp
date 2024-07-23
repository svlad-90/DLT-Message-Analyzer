/**
 * @file    QCPGantt.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the QCP Gantt classes
 */

#pragma once

#include "qcustomplot.h"

class QCPGanttBarsData
{
public:
    QCPGanttBarsData();
    QCPGanttBarsData(double sortKey, double valueStart, double valueEnd);

    inline double sortKey() const { return mSortKey; }
    inline static QCPGanttBarsData fromSortKey(double sortKey) { return QCPGanttBarsData(sortKey, 0, 0); }
    inline static bool sortKeyIsMainKey() { return true; }

    inline double mainKey() const { return mSortKey; }
    inline double mainValue() const { return mValueStart; }

    inline QCPRange valueRange() const { return QCPRange(mValueStart, mValueEnd); } // note that bar base value isn't held in each QCPGanttBarsData and thus can't/shouldn't be returned here

    inline void setSortKey(const double& val){ mSortKey= val; }
    inline void setValueStart(const double& val){ mValueStart = val; }
    inline void setValueEnd(const double& val){ mValueEnd = val; }

private:
    double mSortKey;
    double mValueStart;
    double mValueEnd;
};
Q_DECLARE_TYPEINFO(QCPGanttBarsData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPGanttBarsDataContainer

  Container for storing \ref QCPGanttBarsData data. The data is stored sorted by \a key.

  This template instantiation is the container in which QCPGanttBarsData holds its data. For details about
  the generic container, see the documentation of the class template \ref QCPDataContainer.

  \see QCPGanttBarsData, QCPBars::setData
*/
typedef QCPDataContainer<QCPGanttBarsData> QCPGanttBarsDataContainer;

class QCPGanttRow : public QCPAbstractPlottable1D<QCPGanttBarsData>
{
    Q_OBJECT
    /// \cond INCLUDE_QPROPERTIES
    Q_PROPERTY(double width READ width WRITE setWidth)
    Q_PROPERTY(WidthType widthType READ widthType WRITE setWidthType)
    /// \endcond

public:

    /*!
    Defines the ways the width of the bar can be specified. Thus it defines what the number passed
    to \ref setWidth actually means.

    \see setWidthType, setWidth
  */
    enum WidthType { wtAbsolute       ///< Bar width is in absolute pixels
                     ,wtAxisRectRatio ///< Bar width is given by a fraction of the axis rect size
                     ,wtPlotCoords    ///< Bar width is in key coordinates and thus scales with the key axis range
                   };
    Q_ENUMS(WidthType)

    explicit QCPGanttRow(QCPAxis *keyAxis, QCPAxis *valueAxis);

    // getters:
    double width() const { return mWidth; }
    WidthType widthType() const { return mWidthType; }
    QSharedPointer<QCPGanttBarsDataContainer> data() const { return mDataContainer; }

    // setters:
    void setData(QSharedPointer<QCPGanttBarsDataContainer> data);
    struct GanttItem
    {
        double valueStart;
        double valueEnd;
    };

    void setData(const QVector<double> &keys, const QVector<GanttItem> &values, bool alreadySorted=false);
    void setWidth(double width);
    void setWidthType(WidthType widthType);

    // non-property methods:
    void addData(const QVector<double> &keys, const QVector<GanttItem> &values, bool alreadySorted);
    void addData(double key, double valueStart, double valueEnd);

    // reimplemented virtual methods:
    virtual QCPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable) const Q_DECL_OVERRIDE;
    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
    virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const Q_DECL_OVERRIDE;
    virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth, const QCPRange &inKeyRange=QCPRange()) const Q_DECL_OVERRIDE;
    virtual QPointF dataPixelPosition(int index) const Q_DECL_OVERRIDE;

protected:

    // reimplemented virtual methods:
    virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const Q_DECL_OVERRIDE;

    // non-virtual methods:
    void getVisibleDataBounds(QCPGanttBarsDataContainer::const_iterator &begin, QCPGanttBarsDataContainer::const_iterator &end) const;
    QRectF getBarRect(double key, double valueStart, double valueEnd) const;
    void getPixelWidth(double key, double &lower, double &upper) const;

    friend class QCustomPlot;
    friend class QCPLegend;

private:

    // property members:
    double mWidth;
    WidthType mWidthType;
};
Q_DECLARE_METATYPE(QCPGanttRow::WidthType)
