#include "CSettingItem.hpp"

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
