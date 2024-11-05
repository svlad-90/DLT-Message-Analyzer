#pragma once

#include "qcustomplot.h"
#include <QMouseEvent> // Include for mouse event handling
#include <QSet>        // To keep track of selected items

class CScrollableLegend : public QCPLegend
{
    Q_OBJECT
public:
    explicit CScrollableLegend();
    bool addToLegend(QCPAbstractPlottable* pPlottable);
    void scrollToItem(int itemIndex);
    void highlightItem(int itemIndex);

protected:
    int mScrollOffset; // Offset for scrolling
    QCPAbstractLegendItem* mpSelectedItem;
    QColor mCachedColor;

    // Overridden methods
    void wheelEvent(QWheelEvent *event) override;
    void draw(QCPPainter *painter) override;

private:
    int visibleItemCount() const;
    int itemHeight() const;
};
