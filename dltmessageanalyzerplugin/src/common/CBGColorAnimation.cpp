/**
 * @file    CBGColorAnimation.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CBGColorAnimation class
 */

#include "CBGColorAnimation.hpp"

#include "QWidget"

#include "DMA_Plantuml.hpp"

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

PUML_PACKAGE_BEGIN(DMA_Common)
    PUML_CLASS_BEGIN_CHECKED(CBGColorAnimation)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QWidget, 1, 1, animation widget)
    PUML_CLASS_END()
PUML_PACKAGE_END()
