#include "CScrollableLegend.hpp"
#include <QWheelEvent>
#include <QFontMetrics>

#include "DMA_Plantuml.hpp"

static const double sLegendHeightFactor = 0.8;
static const double sArrowHeight = 10.0;
static const double sArrowWidth = 10.0;

CScrollableLegend::CScrollableLegend()
    : QCPLegend(), mScrollOffset(0), mpSelectedItem(nullptr)
{
    setSelectableParts(QCPLegend::spNone);

    // Set default background color to white
    setBrush(QBrush(QColor(255, 255, 255, 150))); // Semi-transparent white
}

int CScrollableLegend::visibleItemCount() const
{
    QRect axisRectDimensions = parentPlot()->axisRect()->rect();
    int legendHeight = axisRectDimensions.height() * sLegendHeightFactor - (sArrowHeight*2); // Use only 80% of legend height for items
    auto itemHeightVal = itemHeight();
    auto result = legendHeight / itemHeightVal;
    result = result ? result : 1;
    return result;
}

int CScrollableLegend::itemHeight() const
{
    QFontMetrics fontMetrics(this->font());
    return fontMetrics.height() + 4; // Adjust as needed
}

void CScrollableLegend::wheelEvent(QWheelEvent *event)
{
    int totalItems = itemCount();
    int maxVisibleItems = visibleItemCount();
    if (totalItems <= maxVisibleItems)
        return;

    if (event->angleDelta().y() > 0)
        mScrollOffset = qMax(0, mScrollOffset - 1);
    else
        mScrollOffset = qMin(totalItems - maxVisibleItems, mScrollOffset + 1);

    event->accept();
    layer()->replot();
}

void CScrollableLegend::draw(QCPPainter *painter)
{
    if (!mVisible)
        return;

    // Ensure the outer rect stays consistent to avoid the shifting effect
    QRect axisRectDimensions = parentPlot()->axisRect()->rect();
    QRect myRect = outerRect();
    int totalItems = itemCount();
    auto itemHeightVal = itemHeight();
    int totalItemsHeight = totalItems * itemHeightVal;

    int legendHeight = axisRectDimensions.height() * sLegendHeightFactor;
    if (legendHeight > totalItemsHeight)
    {
        legendHeight = totalItemsHeight;
    }

    // Set consistent outer rect, avoid accumulation of rounding errors
    setOuterRect(QRect(myRect.x(), myRect.y(), myRect.width(), legendHeight));

    myRect = outerRect();

    painter->save();
    painter->setClipRect(myRect);

    int maxVisibleItems = visibleItemCount();

    int startIndex = mScrollOffset;
    int endIndex = qMin(mScrollOffset + maxVisibleItems, totalItems);

    // Update positions and visibility for each item, leaving space at top and bottom for arrows
    int arrowSpace = totalItems > maxVisibleItems ? sArrowHeight : 0; // Space for arrows
    for (int i = 0; i < totalItems; ++i)
    {
        QCPAbstractLegendItem* pLegendItem = this->item(i);
        if (pLegendItem)
        {
            if (i >= startIndex && i < endIndex)
            {
                pLegendItem->setVisible(true);
                QRect itemRect(myRect.x() + 5, myRect.y() + arrowSpace + ( (i - startIndex) * itemHeightVal ), myRect.width() - 5, itemHeightVal);
                pLegendItem->setOuterRect(itemRect);
            }
            else
            {
                pLegendItem->setVisible(false);
            }
        }
    }

    // Call base class draw method
    QCPLegend::draw(painter);

    // Draw scroll indicators if needed, outside of item display area
    if (totalItems > maxVisibleItems)
    {
        painter->setPen(Qt::white);

        // Up arrow
        if (mScrollOffset > 0)
        {
            QPointF upArrowCenter(myRect.center().x(), myRect.y() + sArrowHeight / 2);
            QPolygonF upArrow;
            upArrow << QPointF(upArrowCenter.x() - sArrowWidth / 2, upArrowCenter.y() + sArrowHeight / 2)
                    << QPointF(upArrowCenter.x() + sArrowWidth / 2, upArrowCenter.y() + sArrowHeight / 2)
                    << QPointF(upArrowCenter.x(), upArrowCenter.y() - sArrowHeight / 2);
            painter->drawPolygon(upArrow);
        }

        // Down arrow
        if (mScrollOffset < totalItems - maxVisibleItems)
        {
            QPointF downArrowCenter(myRect.center().x(), myRect.bottom() - sArrowHeight / 2);
            QPolygonF downArrow;
            downArrow << QPointF(downArrowCenter.x() - sArrowWidth / 2, downArrowCenter.y() - sArrowHeight / 2)
                      << QPointF(downArrowCenter.x() + sArrowWidth / 2, downArrowCenter.y() - sArrowHeight / 2)
                      << QPointF(downArrowCenter.x(), downArrowCenter.y() + sArrowHeight / 2);
            painter->drawPolygon(downArrow);
        }
    }

    painter->restore();
}

bool CScrollableLegend::addToLegend(QCPAbstractPlottable* pPlottable)
{
  if (!pPlottable)
  {
    qDebug() << Q_FUNC_INFO << "passed plottable is null";
    return false;
  }
  if (parentPlot() != mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "passed plottable isn't in the same QCustomPlot as this legend";
    return false;
  }

  if (!hasItemWithPlottable(pPlottable))
  {
      auto* pLegendItem = new QCPPlottableLegendItem(this, pPlottable);
      pLegendItem->setTextColor(QColor(255,255,255));

      addItem(pLegendItem);
      return true;
  }
  else
      return false;
}

void CScrollableLegend::scrollToItem(int itemIndex)
{
    int totalItems = itemCount();
    int maxVisibleItems = visibleItemCount();

    if (itemIndex < 0 || itemIndex >= totalItems)
        return;

    mScrollOffset = qBound(0, itemIndex, totalItems - maxVisibleItems);
    layer()->replot();
}

void CScrollableLegend::highlightItem(int itemIndex)
{
    int totalItems = itemCount();

    if(mpSelectedItem)
    {
       mpSelectedItem->setTextColor(mCachedColor);
    }

    if (itemIndex < 0 || itemIndex >= totalItems)
        mpSelectedItem = nullptr;

    auto* pLegendItem = item(itemIndex);

    if(pLegendItem)
    {
        mpSelectedItem = pLegendItem;
        mCachedColor = mpSelectedItem->textColor();
        mpSelectedItem->setTextColor(QColor(200,0,0));
    }

    // Replot is called outside of this function for the whole plot.
    // Thus, no replot call here.
}

PUML_PACKAGE_BEGIN(DMA_PlotView)
    PUML_CLASS_BEGIN(CScrollableLegend)
        PUML_INHERITANCE_CHECKED(QCPLegend, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
