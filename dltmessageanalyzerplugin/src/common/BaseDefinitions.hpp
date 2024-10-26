/**
 * @file    BaseDefinitions.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the BaseDefinitions
 */

#ifndef BASE_DEFINITIONS_HPP
#define BASE_DEFINITIONS_HPP

#include <memory>
#include <vector>

#include <QHash>
#include <QString>
#include <QMetaType>
#include <QColor>

typedef std::shared_ptr<QString> tQStringPtr;
typedef std::vector<tQStringPtr> tQStringPtrVec;

struct tQStringPtrWrapper
{
    tQStringPtrWrapper();
    tQStringPtrWrapper(const tQStringPtr& pString_);
    bool operator== ( const tQStringPtrWrapper& rVal ) const;
    bool operator!= ( const tQStringPtrWrapper& rVal ) const;
    bool operator< ( const tQStringPtrWrapper& rVal ) const;
    tQStringPtr pString = nullptr;
};

namespace std {
template<> struct hash<tQStringPtrWrapper> {
    std::size_t operator()(const tQStringPtrWrapper& pS) const noexcept {
        return pS.pString ? (size_t) qHash(*pS.pString) : 0u;
    }
};
}

Q_DECLARE_METATYPE(tQStringPtrWrapper)

struct QOptionalColor
{
    bool isSet = false;
    QRgb color_code = RGB_MASK;
    bool operator== ( const QOptionalColor& rhs ) const;
};
typedef QVector<QOptionalColor> QOptionalColorVec;

#endif // BASE_DEFINITIONS_HPP
