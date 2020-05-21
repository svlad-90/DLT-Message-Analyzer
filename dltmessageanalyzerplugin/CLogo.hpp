/**
 * @file    CLogo.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CLogo class
 */
#ifndef CLOGO_HPP
#define CLOGO_HPP

#include "QPushButton"

class CLogo : public QPushButton
{
    Q_OBJECT
    typedef QPushButton tParent;
public:
    CLogo( QWidget* pParent );
    void paintEvent(QPaintEvent *ev);
};

#endif // CLOGO_HPP
