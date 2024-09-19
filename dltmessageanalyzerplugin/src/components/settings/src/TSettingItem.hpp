#pragma once

#include <QString>

#include "common/Definitions.hpp"
#include "common/cpp_extensions.hpp"
#include "common/TOptional.hpp"
#include "CSettingItem.hpp"

#include "QDebug"
#include "QJsonDocument"

template<typename T>
class TSettingItem : public CSettingItem
{
    public:

    typedef T tData;
    typedef std::function<void(const tData& /*old data*/, const tData& /*new data*/)> tUpdateDataFunc;

    typedef std::function<QJsonObject(const tData&)> tWriteDataFunction;
    typedef std::function<bool(const QJsonValueRef&, tData&, const tData& /*def value*/)> tReadDataFunction;

    /**
     * @brief TSettingItem - constructor
     * @param key - key of the setting
     * @param defaultValue - default value of the setting
     * @param writeDataFunc - function, which writes item to JSON
     * @param readDataFunc - function, which reads data from JSON
     * @param updateDataFunc - function which updates data to interested client's within the system
     * @param updateFileFunc - function which triggers update of the corresponding file
     */
    TSettingItem ( const QString& key,
                   const tData& defaultValue,
                   const tWriteDataFunction& writeDataFunc,
                   const tReadDataFunction& readDataFunc,
                   const tUpdateDataFunc& updateDataFunc,
                   const tUpdateSettingsFileFunc& updateFileFunc ):
        CSettingItem(key, updateFileFunc),
        mDefaultValue(defaultValue),
        mData(defaultValue),
        mUpdateDataFunc(updateDataFunc),
        mWriteDataFunc(writeDataFunc),
        mReadDataFunc(readDataFunc)
    {}

    /**
     * @brief writeData - ( implementation of ISettingItem ) - writes data, which is stored in memory to the QJsonObject.
     * @return - instance of QJsonObject, which can be placed to JSON file.
     */
    QJsonObject writeData() const override
    {
        QJsonObject result;

        if(mWriteDataFunc)
        {
            result = mWriteDataFunc(mData);

            // Uncomment to see content of all written flags in the console
            //QJsonDocument doc(result);
            //QString strJson(doc.toJson(QJsonDocument::Compact));
            //qDebug() << QString("Write key - %1, data - %2").arg(getKey()).arg(strJson);
        }

        return result;
    }

    /**
     * @brief readData - ( implementation of ISettingItem ) - reads data from QJsonObject instance
     * @param setting - JSON setting, from which we should read the data.
     */
    bool readData( const QJsonValueRef& setting ) override
    {
        bool bResult = false;

        if(mReadDataFunc)
        {
            bResult = mReadDataFunc(setting, mData, mDefaultValue);
        }

        return bResult;
    }

    /**
     * @brief getUpdateDataFunc - ( implementation of ISettingItem ) - reference
     * to the update data function, to trigger update
     * Note! Does not pass the 'old data' to an update.
     * @return - reference to the update data function
     */
    tGeneralUpdateDataFunc getGeneralUpdateDataFunc() override
    {
        auto result = [this]()
        {
            if(mUpdateDataFunc)
            {
                mUpdateDataFunc.operator()(tData(), mData);
            }
        };
        return result;
    }

    /**
     * @brief setData - sets the data to memory.
     * Calls the update data func & update file func.
     * @param data - data to be remembered.
     * @param bForce - whether we should update data if new value is equal to an old one
     */
    virtual void setData( const tData& data, bool bForce = false )
    {
        bool bUpdate = true == bForce || mData != data;

        if(true == bUpdate)
        {
            tData oldData = mData;
            mData = data;

            if(mUpdateDataFunc)
            {
                mUpdateDataFunc(oldData, data);
            }
        }

        const auto& updateFileFunc = getUpdateFileFunc();

        if(updateFileFunc)
        {
            updateFileFunc();
        }
    }

    /**
     * @brief setDataSilent - sets the data to memory without emitting any signals.
     * Does not neither the update data func nor the update file func.
     * @param data - data to be remembered.
     * @param bForce - whether we should update data if new value is equal to an old one
     */
    virtual void setDataSilent( const tData& data, bool bForce = false )
    {
        bool bUpdate = true == bForce || mData != data;

        if(true == bUpdate)
        {
            mData = data;
        }
    }

    /**
     * @brief getData - gets the data from memory. Does not read it from the JSON file.
     * @return - the const reference to the data item.
     */
    const tData& getData() const
    {
        return mData;
    }

    protected:

    tData mDefaultValue;
    tData mData;
    tUpdateDataFunc mUpdateDataFunc;
    tWriteDataFunction mWriteDataFunc;
    tReadDataFunction mReadDataFunc;
};

template<typename T>
class TRangedSettingItem: public TSettingItem<T>
{
public:

    typedef TSettingItem<T> tParent;

    typedef T tData;
    typedef typename tParent::tUpdateDataFunc tUpdateDataFunc;
    typedef typename tParent::tWriteDataFunction tWriteDataFunction;
    typedef typename tParent::tReadDataFunction tReadDataFunction;
    typedef CSettingItem::tUpdateSettingsFileFunc tUpdateSettingsFileFunc;

    typedef tRange<T> tAllowedRange;
    typedef TOptional<tAllowedRange> tOptionalAllowedRange;

    /**
     * @brief TSettingItem - constructor
     * @param key - key of the setting
     * @param defaultValue - default value of the setting
     * @param allowedRange - allowed range of the setting
     * @param writeDataFunc - function, which writes item to JSON
     * @param readDataFunc - function, which reads data from JSON
     * @param updateDataFunc - function which updates data to interested client's within the system
     * @param updateFileFunc - function which triggers update of the corresponding file
     */

    TRangedSettingItem ( const QString& key,
                   const tData& defaultValue,
                   const tOptionalAllowedRange& allowedRange,
                   const tWriteDataFunction& writeDataFunc,
                   const tReadDataFunction& readDataFunc,
                   const tUpdateDataFunc& updateDataFunc,
                   const tUpdateSettingsFileFunc& updateFileFunc ):
        tParent(key, defaultValue, writeDataFunc, readDataFunc, updateDataFunc, updateFileFunc),
        mAllowedRange(allowedRange)
    {}

    /**
     * @brief readData - ( implementation of ISettingItem ) - reads data from QJsonObject instance
     * @param setting - JSON setting, from which we should read the data.
     */
    bool readData( const QJsonValueRef& setting ) override
    {
        bool bResult = false;

        if(tParent::mReadDataFunc)
        {
            bResult = tParent::mReadDataFunc(setting, tParent::mData, tParent::mDefaultValue);

            if(true == bResult)
            {
                tParent::mData = normalizeData(tParent::mData);
            }
        }

        return bResult;
    }

    /**
     * @brief setData - sets the data to memory.
     * Calls the update data func & update file func.
     * @param data - data to be remembered.
     * @param bForce - whether we should update data if new value is equal to an old one
     */
    void setData( const typename tParent::tData& data, bool bForce = false ) override
    {
        bool bUpdate = true == bForce || tParent::mData != data;

        if(true == bUpdate)
        {
            const auto& normalizedValue = normalizeData(data);

            tData oldData = tParent::mData;
            tParent::mData = normalizedValue;

            if(tParent::mUpdateDataFunc)
            {
                tParent::mUpdateDataFunc(oldData, tParent::mData);
            }
        }

        const auto& updateFileFunc = tParent::getUpdateFileFunc();

        if(updateFileFunc)
        {
            updateFileFunc();
        }
    }

    /**
     * @brief setDataSilent - sets the data to memory without emitting any signals.
     * Does not neither the update data func nor the update file func.
     * @param data - data to be remembered.
     * @param bForce - whether we should update data if new value is equal to an old one
     */
    void setDataSilent( const typename tParent::tData& data, bool bForce = false ) override
    {
        bool bUpdate = true == bForce || tParent::mData != data;

        if(true == bUpdate)
        {
            const auto& normalizedValue = normalizeData(data);
            tParent::mData = normalizedValue;
        }
    }

    const tOptionalAllowedRange& getAllowedTange() const
    {
        return mAllowedRange;
    }

private:

    const T& normalizeData(const T& val)
    {
        if(true == mAllowedRange.isSet())
        {
            const auto& value = mAllowedRange.getValue();

            if(val < value.from)
            {
                return value.from;
            }
            else if(val > value.to)
            {
                return value.to;
            }
        }

        return val;
    }

private:

    tOptionalAllowedRange mAllowedRange;
};
