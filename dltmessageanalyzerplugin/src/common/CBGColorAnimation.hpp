/**
 * @file    QBGColorAnimation.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CBGColorAnimation class
 */
#ifndef CBGCOLORANIMATION_HPP
#define CBGCOLORANIMATION_HPP

#include <QObject>
#include <QColor>
#include "QPalette"

class QWidget;

/**
 * @brief The CBGColorAnimation class - the proxy class, which should be used together with QPropertyAnimation concept.
 * Allows to wrap an input widget, which is provided within a constructor, into an entity which has a "color" property.
 * BG stands for "back-ground", but actually, what will be animated depends on provided role.
 */
class CBGColorAnimation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
public:
    /**
     * @brief CBGColorAnimation - creates an instance of class
     * @param pAnimationWidget - the widget, which will be animated
     * @param role - the color role. Specifies which kind of color will be changed during animation.
     */
    CBGColorAnimation( QWidget* pAnimationWidget, const QPalette::ColorRole role );

    /**
     * @brief setColor - setter of color property. Sets new color.
     * @param color_ - the color to be set.
     */
    void setColor(const QColor& color_);

    /**
     * @brief getColor - getter of color property.
     * @return - returns currently used color.
     */
    QColor getColor() const;

private:
    QWidget* mpAnimationWidget;
    QColor mColor;
    QPalette::ColorRole mRole;
};


#endif // QBGCOLORANIMATION_HPP
