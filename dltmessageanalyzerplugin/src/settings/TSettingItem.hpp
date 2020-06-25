#ifndef TSETTINGITEM_HPP
#define TSETTINGITEM_HPP

#include <QString>
#include "CSettingItem.hpp"

template<typename T>
class TSettingItem : public CSettingItem
{
    public:

    typedef T tData;
    typedef std::function<void(const tData&)> tUpdateDataFunc;

    typedef std::function<QJsonObject(const tData&)> tWriteDataFunction;
    typedef std::function<bool(const QJsonValueRef&, tData&, const tData& /*def value*/)> tReadDataFunction;

    /**
     * @brief TSettingItem - constructor of setting item
     * @param key - key, which should be used to read and write the data
     */
    TSettingItem ( const QString& key,
                   const tData& defaultValue,
                   const tWriteDataFunction& writeDataFunc,
                   const tReadDataFunction& readDataFunc,
                   const tUpdateDataFunc& updateDataFunc,
                   const tUpdateSettingsFileFunc& updateFieFunc ):
        CSettingItem(key, updateFieFunc),
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
     * @brief getUpdateDataFunc - ( implementation of ISettingItem ) - reference to the update data function, to trigger update
     * @return - reference to the update data function
     */
    tGeneralUpdateDataFunc getGeneralUpdateDataFunc() override
    {
        auto result = [this]()
        {
            if(mUpdateDataFunc)
            {
                mUpdateDataFunc.operator()(mData);
            }
        };
        return result;
    }

    /**
     * @brief setData - sets the data to memory.
     * Calls the update data func & update file func.
     * @param data - data to be remembered.
     */
    void setData( const tData& data )
    {
        bool bUpdate = mData != data;

        if(true == bUpdate)
        {
            mData = data;

            if(mUpdateDataFunc)
            {
                mUpdateDataFunc(mData);
            }
        }

        const auto updateFileFunc = getUpdateFileFunc();

        if(updateFileFunc)
        {
            updateFileFunc();
        }
    }

    /**
     * @brief setDataSilent - sets the data to memory without emitting any signals.
     * Does not neither the update data func nor the update file func.
     * @param data - data to be remembered.
     */
    void setDataSilent( const tData& data )
    {
        bool bUpdate = mData != data;

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

    private:

    tData mDefaultValue;
    tData mData;
    tUpdateDataFunc mUpdateDataFunc;
    tWriteDataFunction mWriteDataFunc;
    tReadDataFunction mReadDataFunc;
};

#endif // TSETTINGITEM_HPP
