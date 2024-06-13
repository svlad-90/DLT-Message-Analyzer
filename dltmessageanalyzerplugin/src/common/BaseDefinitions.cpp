/**
 * @file    BaseDefinitions.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the BaseDefinitions
 */

#include "BaseDefinitions.hpp"

bool QOptionalColor::operator== ( const QOptionalColor& rhs ) const
{
    return color_code == rhs.color_code && isSet == rhs.isSet;
}
