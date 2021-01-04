#pragma once

#include <functional>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValueRef>

class ISettingItem
{
public:

    virtual ~ISettingItem() = default;

    ///////////////////////////////////////////////////////
    /// PART OF API WHICH IS IMPLEMENTED BY BASE CLASSES
    ///////////////////////////////////////////////////////

    /**
     * @brief writeData - writes data, which is stored in memory to the QJsonObject.
     * @return - instance of QJsonObject, which can be placed to JSON file.
     */
    virtual QJsonObject writeData() const = 0;

    /**
     * @brief readData - reads data from QJsonObject instance
     * @param setting - JSON setting, from which we should read the data.
     * @return - true, if data was successfully restored. False otherwise.
     */
    virtual bool readData( const QJsonValueRef& setting ) = 0;

    /**
     * @brief getKey - gets the key, which was previously assigned to this setting item
     * @return - the key string
     */
    virtual const QString& getKey() const = 0;

    typedef std::function<void(void)> tGeneralUpdateDataFunc;

    /**
     * @brief getGeneralUpdateDataFunc - reference to the update data function, to trigger update the setting on Qt level
     * @return - instance of the update data function
     */
    virtual tGeneralUpdateDataFunc getGeneralUpdateDataFunc() = 0;

    typedef std::function<void(void)> tUpdateSettingsFileFunc;

    /**
     * @brief getUpdateFileFunc - reference to the update file function, to trigger store of all settings to the JSON file
     * @return - reference to the update file function
     */
    virtual const tUpdateSettingsFileFunc& getUpdateFileFunc() = 0;

    /**
     * @brief readDataFromArray - reads data from QJsonObject instance
     * @param settingsArray - array of settings, in which we should search for our setting.
     * @return - true, if data was successfully restored. False otherwise.
     */
    bool readDataFromArray( const QJsonArray& settingsArray );
};
