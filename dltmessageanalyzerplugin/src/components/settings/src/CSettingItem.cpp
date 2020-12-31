#include "CSettingItem.hpp"

#include "DMA_Plantuml.hpp"

CSettingItem::CSettingItem ( const QString& key,
           const tUpdateSettingsFileFunc& updateFielFunc ):
    mKey(key),
    mUpdateFielFunc(updateFielFunc)
{}

const QString& CSettingItem::getKey()
{
    return mKey;
}

const ISettingItem::tUpdateSettingsFileFunc& CSettingItem::getUpdateFileFunc()
{
    return mUpdateFielFunc;
}

PUML_PACKAGE_BEGIN(DMA_Settings)
    PUML_ABSTRACT_CLASS_BEGIN_CHECKED(CSettingItem)
    PUML_INHERITANCE_CHECKED(ISettingItem, partially implements)
    PUML_OVERRIDE_METHOD(+, const QString& getKey())
    PUML_OVERRIDE_METHOD(+, const tUpdateSettingsFileFunc& getUpdateFileFunc())
    PUML_ABSTRACT_CLASS_END()

    PUML_CLASS_BEGIN(TSettingItem<T>)
        PUML_INHERITANCE_CHECKED(CSettingItem, extends)
        PUML_OVERRIDE_METHOD(+, QJsonObject writeData() const)
        PUML_OVERRIDE_METHOD(+, bool readData( const QJsonValueRef& setting ))
        PUML_OVERRIDE_METHOD(+, tGeneralUpdateDataFunc getGeneralUpdateDataFunc())
    PUML_CLASS_END()
PUML_PACKAGE_END()
