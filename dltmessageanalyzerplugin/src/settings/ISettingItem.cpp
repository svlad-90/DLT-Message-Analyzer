#include "ISettingItem.hpp"

#include "QDebug"

bool ISettingItem::readDataFromArray( const QJsonArray& settingsArray )
{
    bool bResult = false;

    const QString& key = getKey();

    for(auto item : settingsArray) // no references returned
    {
        if(true == item.isObject())
        {
            auto object = item.toObject();

            {
                auto foundSetting = object.find(key);
                if(foundSetting != object.end())
                {
                    bResult = readData(foundSetting.value());

                    if(true == bResult )
                    {
                        const auto& generalUpdateDataFunc = getGeneralUpdateDataFunc();

                        if(generalUpdateDataFunc)
                        {
                            generalUpdateDataFunc();
                        }

                        break;
                    }
                }
            }
        }
    }

    return bResult;
}
