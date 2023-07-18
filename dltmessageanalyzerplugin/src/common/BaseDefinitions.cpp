/**
 * @file    BaseDefinitions.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the BaseDefinitions
 */

#include "BaseDefinitions.hpp"

bool QOptionalColor::operator== ( const QOptionalColor& rhs ) const
{
    return color == rhs.color && isSet == rhs.isSet;
}
