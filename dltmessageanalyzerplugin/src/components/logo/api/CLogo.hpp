/**
 * @file    CLogo.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CLogo class
 */
#pragma once

#include "QPushButton"

class CLogo : public QPushButton
{
    Q_OBJECT
    typedef QPushButton tParent;
public:
    CLogo( QWidget* pParent );
    void paintEvent(QPaintEvent *ev);
};
