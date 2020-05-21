/**
 * @file    CBGColorAnimation.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CBGColorAnimation class
 */

#include "CBGColorAnimation.hpp"

#include "QWidget"

CBGColorAnimation::CBGColorAnimation( QWidget* pAnimationWidget, const QPalette::ColorRole role ):
    mpAnimationWidget(pAnimationWidget), mColor(255,255,255),
    mRole(role)
{

}

void CBGColorAnimation::setColor(const QColor& color_)
{
    if(nullptr != mpAnimationWidget)
    {
        QPalette palette = mpAnimationWidget->palette();
        palette.setColor( mRole, color_ );
        mpAnimationWidget->setPalette(palette);
    }
}

QColor CBGColorAnimation::getColor() const
{
    QColor result;

    if(nullptr != mpAnimationWidget)
    {
        result = mpAnimationWidget->palette().color(mRole);
    }

    return result;
}
