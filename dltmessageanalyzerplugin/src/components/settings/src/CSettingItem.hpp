#pragma once

#include "ISettingItem.hpp"

class CSettingItem : public ISettingItem
{
public:

    /**
    * @brief CSettingItem - constructor of setting item
    * @param key - key, which should be used to read and write the data
    */
    CSettingItem ( const QString& key, const tUpdateSettingsFileFunc& updateFielFunc );

    /**
    * @brief getKey - ( implementation of ISettingItem ) - gets the key, which was previously assigned to this setting item
    * @return - the key string
    */
    const QString& getKey() override;

    /**
    * @brief getUpdateFileFunc - ( implementation of ISettingItem ) - reference to the update file function,
    * to trigger store of all settings to the JSON file
    * @return - reference to the update file function
    */
    const tUpdateSettingsFileFunc& getUpdateFileFunc() override;

    private:

    QString mKey;
    tUpdateSettingsFileFunc mUpdateFielFunc;
};
