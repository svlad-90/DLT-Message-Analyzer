#ifndef TOPTIONAL_HPP
#define TOPTIONAL_HPP

/**
 * @brief - very very very poor replacement of std::optional & boost::optional
 * Created, as we need to support C++11 as of now.
 * Also Qt has no replacement, and we do not want to mix Qt with boost.
 * Once we will switch to C++17 this one will be definitely replaced to std::optional
 */

#include "assert.h"

template<typename T>
class TOptional
{

public:
    TOptional():
        mData(),
        mbIsSet(false)
    {}

    explicit TOptional(const T& val):
    mData(val),
    mbIsSet(true)
    {}

    void setValue(const T& val)
    {
        mData = val;
        mbIsSet = true;
    }

    const T& getValue() const
    {
        assert(mbIsSet);
        return mData;
    }

    T& getWriteableValue()
    {
        assert(mbIsSet);
        return mData;
    }

    bool isSet() const
    {
        return mbIsSet;
    }

    void reset()
    {
        mData = T();
        mbIsSet = false;
    }

private:
    T mData;
    bool mbIsSet;
};

#endif // TOPTIONAL_HPP
