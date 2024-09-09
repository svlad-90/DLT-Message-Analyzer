/**
 * @file    QCPGantt.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the QCP Gantt classes
 */

#include "QCPGantt.hpp"

#include "DMA_Plantuml.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPGanttBarsData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPGanttBarsData
  \brief Holds the data of one single data point (one bar) for QCPGanttRow.

  The stored data is:
  \li \a key: coordinate on the key axis of this bar (this is the \a mainKey and the \a sortKey)
  \li \a valueStart: starting point coordinate of this bar on the value axis.
  \li \a valueEnd: end point coordinate of this bar on the value axis.

  The container for storing multiple data points is \ref QCPGanttBarsDataContainer. It is a typedef for
  \ref QCPDataContainer with \ref QCPGanttBarsData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.

  \see QCPGanttBarsDataContainer
*/

/* start documentation of inline functions */

/*! \fn double QCPGanttBarsData::sortKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static QCPGanttBarsData QCPGanttBarsData::fromSortKey(double sortKey)

  Returns a data point with the specified \a sortKey. All other members are set to zero.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static static bool QCPGanttBarsData::sortKeyIsMainKey()

  Since the member \a key is both the data point key coordinate and the data ordering parameter,
  this method returns true.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPGanttBarsData::mainKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPGanttBarsData::mainValue() const

  Returns the \a value member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn QCPRange QCPGanttBarsData::valueRange() const

  Returns a QCPRange with both lower and upper boundary set to \a value of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a bar data point with key and value set to zero.
*/
QCPGanttBarsData::QCPGanttBarsData() :
    mSortKey(0),
    mValueStart(0),
    mValueEnd(0)
{
}

/*!
  Constructs a bar data point with the specified \a key and \a value.
*/
QCPGanttBarsData::QCPGanttBarsData(double sortKey, double valueStart, double valueEnd) :
    mSortKey(sortKey),
    mValueStart(valueStart),
    mValueEnd(valueEnd)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPGanttRow
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPGanttRow
  \brief A plottable representing a Gantt chart row in a plot.

  To plot data, assign it with the \ref setData or \ref addData functions.

  \section qcpganttrows-appearance Changing the appearance

  The appearance of the bars is determined by the pen and the brush (\ref setPen, \ref setBrush).
  The width of the individual rows can be controlled with \ref setWidthType and \ref setWidth.

  \section qcpganttrows-usage Usage

  Like all data representing objects in QCustomPlot, the QCPGanttRow is a plottable
  (QCPAbstractPlottable). So the plottable-interface of QCustomPlot applies
  (QCustomPlot::plottable, QCustomPlot::removePlottable, etc.)
*/

/* start of documentation of inline functions */

/*! \fn QSharedPointer<QCPGanttBarsDataContainer> QCPGanttRow::data() const

  Returns a shared pointer to the internal data storage of type \ref QCPGanttBarsDataContainer. You may
  use it to directly manipulate the data, which may be more convenient and faster than using the
  regular \ref setData or \ref addData methods.
*/

/* end of documentation of inline functions */

/*!
  Constructs a gantt chart row, which uses \a keyAxis as its key axis ("x") and \a valueAxis as its value
  axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance and not have
  the same orientation. If either of these restrictions is violated, a corresponding message is
  printed to the debug output (qDebug), the construction is not aborted, though.

  The created QCPGanttRow is automatically registered with the QCustomPlot instance inferred from \a
  keyAxis. This QCustomPlot instance takes ownership of the QCPGanttRow, so do not delete it manually
  but use QCustomPlot::removePlottable() instead.
*/
QCPGanttRow::QCPGanttRow(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPAbstractPlottable1D<QCPGanttBarsData>(keyAxis, valueAxis),
    mWidth(0.75),
    mWidthType(wtPlotCoords)
{
    // modify inherited properties from abstract plottable:
    mPen.setColor(Qt::blue);
    mPen.setStyle(Qt::SolidLine);
    mBrush.setColor(QColor(40, 50, 255, 30));
    mBrush.setStyle(Qt::SolidPattern);
    mSelectionDecorator->setBrush(QBrush(QColor(160, 0, 0)));
    QPen selectionDecoratorPen(QColor(150, 0, 0));
    selectionDecoratorPen.setStyle(Qt::DashLine);
    mSelectionDecorator->setPen(selectionDecoratorPen);
}

/*! \overload

  Replaces the current data container with the provided \a data container.

  Since a QSharedPointer is used, multiple QCPGanttRow may share the same data container safely.
  Modifying the data in the container will then affect all bars that share the container. Sharing
  can be achieved by simply exchanging the data containers wrapped in shared pointers.

  If you do not wish to share containers, but create a copy from an existing container, rather use
  the \ref QCPDataContainer<DataType>::set method on the bar's data container directly.

  \see addData
*/
void QCPGanttRow::setData(QSharedPointer<QCPGanttBarsDataContainer> data)
{
    mDataContainer = data;
}

/*! \overload

  Replaces the current data with the provided points in \a keys and \a values. The provided
  vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.

  If you can guarantee that the passed data points are sorted by \a keys in ascending order, you
  can set \a alreadySorted to true, to improve performance by saving a sorting run.

  \see addData
*/
void QCPGanttRow::setData(const QVector<double> &keys, const QVector<GanttItem> &values, bool alreadySorted)
{
    mDataContainer->clear();
    addData(keys, values, alreadySorted);
}

/*!
  Sets the width of the bars.

  How the number passed as \a width is interpreted (e.g. screen pixels, plot coordinates,...),
  depends on the currently set width type, see \ref setWidthType and \ref WidthType.
*/
void QCPGanttRow::setWidth(double width)
{
    mWidth = width;
}

/*!
  Sets how the width of the bars is defined. See the documentation of \ref WidthType for an
  explanation of the possible values for \a widthType.

  The default value is \ref wtPlotCoords.

  \see setWidth
*/
void QCPGanttRow::setWidthType(QCPGanttRow::WidthType widthType)
{
    mWidthType = widthType;
}

/*! \overload

  Adds the provided points in \a keys and \a values to the current data. The provided vectors
  should have equal length. Else, the number of added points will be the size of the smallest
  vector.

  If you can guarantee that the passed data points are sorted by \a keys in ascending order, you
  can set \a alreadySorted to true, to improve performance by saving a sorting run.

  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPGanttRow::addData(const QVector<double> &keys, const QVector<GanttItem> &values, bool alreadySorted)
{
    if (keys.size() != values.size())
        qDebug() << Q_FUNC_INFO << "keys and values have different sizes:" << keys.size() << values.size();
    const int n = qMin(keys.size(), values.size());
    QVector<QCPGanttBarsData> tempData(n);
    QVector<QCPGanttBarsData>::iterator it = tempData.begin();
    const QVector<QCPGanttBarsData>::iterator itEnd = tempData.end();
    int i = 0;
    while (it != itEnd)
    {
        it->setSortKey( keys[i] );
        it->setValueStart( values[i].valueStart );
        it->setValueEnd( values[i].valueEnd );
        ++it;
        ++i;
    }
    mDataContainer->add(tempData, alreadySorted); // don't modify tempData beyond this to prevent copy on write
}

/*! \overload
  Adds the provided data point as \a key and \a value to the current data.

  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPGanttRow::addData(double key, double valueStart, double valueEnd)
{
    mDataContainer->add(QCPGanttBarsData(key, valueStart, valueEnd));
}

/*!
  \copydoc QCPPlottableInterface1D::selectTestRect
*/
QCPDataSelection QCPGanttRow::selectTestRect(const QRectF &rect, bool onlySelectable) const
{
    QCPDataSelection result;
    if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
      return result;
    if (!mKeyAxis || !mValueAxis)
      return result;

    QCPGanttBarsDataContainer::const_iterator visibleBegin, visibleEnd;
    getVisibleDataBounds(visibleBegin, visibleEnd);

    for (QCPGanttBarsDataContainer::const_iterator it=visibleBegin; it!=visibleEnd; ++it)
    {
      if (rect.intersects(getBarRect(it->sortKey(), it->valueRange().lower, it->valueRange().upper)))
        result.addDataRange(QCPDataRange(int(it-mDataContainer->constBegin()), int(it-mDataContainer->constBegin()+1)), false);
    }
    result.simplify();
    return result;
}

/*!
  Implements a selectTest specific to this plottable's point geometry.

  If \a details is not 0, it will be set to a \ref QCPDataSelection, describing the closest data
  point to \a pos.

  \seebaseclassmethod \ref QCPAbstractPlottable::selectTest
*/
double QCPGanttRow::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
    Q_UNUSED(details)
    if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
        return -1;
    if (!mKeyAxis || !mValueAxis)
        return -1;

    if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()) || mParentPlot->interactions().testFlag(QCP::iSelectPlottablesBeyondAxisRect))
    {
        // get visible data range:
        QCPGanttBarsDataContainer::const_iterator visibleBegin, visibleEnd;
        getVisibleDataBounds(visibleBegin, visibleEnd);
        for (QCPGanttBarsDataContainer::const_iterator it=visibleBegin; it!=visibleEnd; ++it)
        {
            if (getBarRect(it->sortKey(), it->valueRange().lower, it->valueRange().upper).contains(pos))
            {
                if (details)
                {
                    int pointIndex = int(it-mDataContainer->constBegin());
                    details->setValue(QCPDataSelection(QCPDataRange(pointIndex, pointIndex+1)));
                }
                return mParentPlot->selectionTolerance()*0.99;
            }
        }
    }
    return -1;
}

/* inherits documentation from base class */
QCPRange QCPGanttRow::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
    /* Note: If this QCPGanttRow uses absolute pixels as width (or is in a QCPGanttRow with spacing in
  absolute pixels), using this method to adapt the key axis range to fit the bars into the
  currently visible axis range will not work perfectly. Because in the moment the axis range is
  changed to the new range, the fixed pixel widths/spacings will represent different coordinate
  spans than before, which in turn would require a different key range to perfectly fit, and so on.
  The only solution would be to iteratively approach the perfect fitting axis range, but the
  mismatch isn't large enough in most applications, to warrant this here. If a user does need a
  better fit, he should call the corresponding axis rescale multiple times in a row.
  */
    QCPRange range;
    range = mDataContainer->keyRange(foundRange, inSignDomain);

    // determine exact range of bars by including bar width and barsgroup offset:
    if (foundRange && mKeyAxis)
    {
        double lowerPixelWidth, upperPixelWidth, keyPixel;
        // lower range bound:
        getPixelWidth(range.lower, lowerPixelWidth, upperPixelWidth);
        keyPixel = mKeyAxis.data()->coordToPixel(range.lower) + lowerPixelWidth;
        const double lowerCorrected = mKeyAxis.data()->pixelToCoord(keyPixel);
        if (!qIsNaN(lowerCorrected) && qIsFinite(lowerCorrected) && range.lower > lowerCorrected)
            range.lower = lowerCorrected;
        // upper range bound:
        getPixelWidth(range.upper, lowerPixelWidth, upperPixelWidth);
        keyPixel = mKeyAxis.data()->coordToPixel(range.upper) + upperPixelWidth;
        const double upperCorrected = mKeyAxis.data()->pixelToCoord(keyPixel);
        if (!qIsNaN(upperCorrected) && qIsFinite(upperCorrected) && range.upper < upperCorrected)
            range.upper = upperCorrected;
    }
    return range;
}

/* inherits documentation from base class */
QCPRange QCPGanttRow::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
    return mDataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

/* inherits documentation from base class */
QPointF QCPGanttRow::dataPixelPosition(int index) const
{
    if (index >= 0 && index < mDataContainer->size())
    {
        QCPAxis *keyAxis = mKeyAxis.data();
        QCPAxis *valueAxis = mValueAxis.data();
        if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return {}; }

        const QCPDataContainer<QCPGanttBarsData>::const_iterator it = mDataContainer->constBegin()+index;
        const double valuePixel = it->valueRange().upper;
        const double keyPixel = keyAxis->coordToPixel(it->sortKey());
        if (keyAxis->orientation() == Qt::Horizontal)
            return {keyPixel, valuePixel};
        else
            return {valuePixel, keyPixel};
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
        return {};
    }
}

/* inherits documentation from base class */
void QCPGanttRow::draw(QCPPainter *painter)
{
    if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    if (mDataContainer->isEmpty()) return;

    QCPGanttBarsDataContainer::const_iterator visibleBegin, visibleEnd;
    getVisibleDataBounds(visibleBegin, visibleEnd);

    // loop over and draw segments of unselected/selected data:
    QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
    getDataSegments(selectedSegments, unselectedSegments);
    allSegments << unselectedSegments << selectedSegments;
    for (int i=0; i<allSegments.size(); ++i)
    {
        bool isSelectedSegment = i >= unselectedSegments.size();
        QCPGanttBarsDataContainer::const_iterator begin = visibleBegin;
        QCPGanttBarsDataContainer::const_iterator end = visibleEnd;
        mDataContainer->limitIteratorsToDataRange(begin, end, allSegments.at(i));
        if (begin == end)
            continue;

        for (QCPGanttBarsDataContainer::const_iterator it=begin; it!=end; ++it)
        {
            // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
            if (QCP::isInvalidData(it->key, it->value))
                qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "of drawn range invalid." << "Plottable name:" << name();
#endif
            // draw bar:
            if (isSelectedSegment && mSelectionDecorator)
            {
                mSelectionDecorator->applyBrush(painter);
                mSelectionDecorator->applyPen(painter);
            } else
            {
                painter->setBrush(mBrush);
                painter->setPen(mPen);
            }
            applyDefaultAntialiasingHint(painter);
            painter->drawPolygon(getBarRect(it->sortKey(), it->valueRange().lower, it->valueRange().upper));
        }
    }

    // draw other selection decoration that isn't just line/scatter pens and brushes:
    if (mSelectionDecorator)
        mSelectionDecorator->drawDecoration(painter, selection());
}

/* inherits documentation from base class */
void QCPGanttRow::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
    // draw filled rect:
    applyDefaultAntialiasingHint(painter);
    painter->setBrush(mBrush);
    painter->setPen(mPen);
    QRectF r = QRectF(0, 0, rect.width()*0.67, rect.height()*0.67);
    r.moveCenter(rect.center());
    painter->drawRect(r);
}

/*!  \internal

  called by \ref draw to determine which data (key) range is visible at the current key axis range
  setting, so only that needs to be processed. It also takes into account the bar width.

  \a begin returns an iterator to the lowest data point that needs to be taken into account when
  plotting. Note that in order to get a clean plot all the way to the edge of the axis rect, \a
  lower may still be just outside the visible range.

  \a end returns an iterator one higher than the highest visible data point. Same as before, \a end
  may also lie just outside of the visible range.

  if the plottable contains no data, both \a begin and \a end point to constEnd.
*/
void QCPGanttRow::getVisibleDataBounds(QCPGanttBarsDataContainer::const_iterator &begin, QCPGanttBarsDataContainer::const_iterator &end) const
{
    if (!mKeyAxis)
    {
        qDebug() << Q_FUNC_INFO << "invalid key axis";
        begin = mDataContainer->constEnd();
        end = mDataContainer->constEnd();
        return;
    }
    if (mDataContainer->isEmpty())
    {
        begin = mDataContainer->constEnd();
        end = mDataContainer->constEnd();
        return;
    }

    // get visible data range as QMap iterators
    begin = mDataContainer->findBegin(mKeyAxis.data()->range().lower);
    end = mDataContainer->findEnd(mKeyAxis.data()->range().upper);
    double lowerPixelBound = mKeyAxis.data()->coordToPixel(mKeyAxis.data()->range().lower);
    double upperPixelBound = mKeyAxis.data()->coordToPixel(mKeyAxis.data()->range().upper);
    bool isVisible = false;
    // walk left from begin to find lower bar that actually is completely outside visible pixel range:
    QCPGanttBarsDataContainer::const_iterator it = begin;
    while (it != mDataContainer->constBegin())
    {
        --it;
        const QRectF barRect = getBarRect(it->sortKey(), it->valueRange().lower, it->valueRange().upper);
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
            isVisible = ((!mKeyAxis.data()->rangeReversed() && barRect.right() >= lowerPixelBound) || (mKeyAxis.data()->rangeReversed() && barRect.left() <= lowerPixelBound));
        else // keyaxis is vertical
            isVisible = ((!mKeyAxis.data()->rangeReversed() && barRect.top() <= lowerPixelBound) || (mKeyAxis.data()->rangeReversed() && barRect.bottom() >= lowerPixelBound));
        if (isVisible)
            begin = it;
        else
            break;
    }
    // walk right from ubound to find upper bar that actually is completely outside visible pixel range:
    it = end;
    while (it != mDataContainer->constEnd())
    {
        const QRectF barRect = getBarRect(it->sortKey(), it->valueRange().lower, it->valueRange().upper);
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
            isVisible = ((!mKeyAxis.data()->rangeReversed() && barRect.left() <= upperPixelBound) || (mKeyAxis.data()->rangeReversed() && barRect.right() >= upperPixelBound));
        else // keyaxis is vertical
            isVisible = ((!mKeyAxis.data()->rangeReversed() && barRect.bottom() >= upperPixelBound) || (mKeyAxis.data()->rangeReversed() && barRect.top() <= upperPixelBound));
        if (isVisible)
            end = it+1;
        else
            break;
        ++it;
    }
}

/*! \internal

  Returns the rect in pixel coordinates of a single bar with the specified \a key and \a value. The
  rect is shifted according to the bar stacking (see \ref moveAbove) and base value (see \ref
  setBaseValue), and to have non-overlapping border lines with the bars stacked below.
*/
QRectF QCPGanttRow::getBarRect(double key, double valueStart, double valueEnd) const
{
    QCPAxis *keyAxis = mKeyAxis.data();
    QCPAxis *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return {}; }

    double lowerPixelWidth, upperPixelWidth;
    getPixelWidth(key, lowerPixelWidth, upperPixelWidth);
    double basePixel = valueAxis->coordToPixel(valueStart);
    double valuePixel = valueAxis->coordToPixel(valueEnd);
    double keyPixel = keyAxis->coordToPixel(key);
    if (keyAxis->orientation() == Qt::Horizontal)
    {
        return QRectF(QPointF(keyPixel+lowerPixelWidth, valuePixel), QPointF(keyPixel+upperPixelWidth, basePixel)).normalized();
    }
    else
    {
        return QRectF(QPointF(basePixel, keyPixel+lowerPixelWidth), QPointF(valuePixel, keyPixel+upperPixelWidth)).normalized();
    }
}

/*! \internal

  This function is used to determine the width of the bar at coordinate \a key, according to the
  specified width (\ref setWidth) and width type (\ref setWidthType).

  The output parameters \a lower and \a upper return the number of pixels the bar extends to lower
  and higher keys, relative to the \a key coordinate (so with a non-reversed horizontal axis, \a
  lower is negative and \a upper positive).
*/
void QCPGanttRow::getPixelWidth(double key, double &lower, double &upper) const
{
    lower = 0;
    upper = 0;
    switch (mWidthType)
    {
    case wtAbsolute:
    {
        upper = mWidth*0.5*mKeyAxis.data()->pixelOrientation();
        lower = -upper;
        break;
    }
    case wtAxisRectRatio:
    {
        if (mKeyAxis && mKeyAxis.data()->axisRect())
        {
            if (mKeyAxis.data()->orientation() == Qt::Horizontal)
                upper = mKeyAxis.data()->axisRect()->width()*mWidth*0.5*mKeyAxis.data()->pixelOrientation();
            else
                upper = mKeyAxis.data()->axisRect()->height()*mWidth*0.5*mKeyAxis.data()->pixelOrientation();
            lower = -upper;
        } else
            qDebug() << Q_FUNC_INFO << "No key axis or axis rect defined";
        break;
    }
    case wtPlotCoords:
    {
        if (mKeyAxis)
        {
            double keyPixel = mKeyAxis.data()->coordToPixel(key);
            upper = mKeyAxis.data()->coordToPixel(key+mWidth*0.5)-keyPixel;
            lower = mKeyAxis.data()->coordToPixel(key-mWidth*0.5)-keyPixel;
            // no need to qSwap(lower, higher) when range reversed, because higher/lower are gained by
            // coordinate transform which includes range direction
        } else
            qDebug() << Q_FUNC_INFO << "No key axis defined";
        break;
    }
    }
}
/* end of 'src/plottables/plottable-bars.cpp' */

PUML_PACKAGE_BEGIN(DMA_PlotView)
    PUML_CLASS_BEGIN(QCPGanttBarsData)
    PUML_CLASS_END()
    PUML_CLASS_BEGIN(QCPGanttRow)
    PUML_INHERITANCE_CHECKED(QCPAbstractPlottable1D<QCPGanttBarsData>, extends)
    PUML_COMPOSITION_DEPENDENCY_CHECKED(QCPGanttBarsData, 1, many, contains)
    PUML_AGGREGATION_DEPENDENCY_CHECKED(QCPAxis, 1, 2, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
