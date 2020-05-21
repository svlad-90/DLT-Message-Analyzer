/**
 * @file    CLogo.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CLogo class
 */

#include "QPaintEvent"
#include "QPainter"
#include "CLogo.hpp"

CLogo::CLogo( QWidget* pParent ):
    tParent(pParent)
{

}

void CLogo::paintEvent(QPaintEvent *ev)
{
    tParent::paintEvent(ev);

    // draw a blue border inside the button
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const int border_width = 3;
    painter.setPen(QPen(QColor(255,255,255), border_width));
    painter.drawRect(QRect(ev->rect().x() + 1, ev->rect().y() + 1, ev->rect().width() - 2, ev->rect().height() - 2));
}
