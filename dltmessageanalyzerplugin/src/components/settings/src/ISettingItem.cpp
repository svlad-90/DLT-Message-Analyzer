#include "ISettingItem.hpp"

#include "QDebug"

#include "DMA_Plantuml.hpp"

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

PUML_PACKAGE_BEGIN(DMA_Settings)
    PUML_ABSTRACT_CLASS_BEGIN_CHECKED(ISettingItem)
        PUML_PURE_VIRTUAL_METHOD(+, QJsonObject writeData() const)
        PUML_PURE_VIRTUAL_METHOD(+, bool readData( const QJsonValueRef& setting ))
        PUML_PURE_VIRTUAL_METHOD(+, const QString& getKey())
        PUML_PURE_VIRTUAL_METHOD(+, tGeneralUpdateDataFunc getGeneralUpdateDataFunc())
        PUML_PURE_VIRTUAL_METHOD(+, const tUpdateSettingsFileFunc& getUpdateFileFunc())
    PUML_ABSTRACT_CLASS_END()
PUML_PACKAGE_END()
