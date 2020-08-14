#ifndef CPP_EXTENSIONS_HPP
#define CPP_EXTENSIONS_HPP

#include <type_traits>
#include <memory>

namespace cpp_14
{
    template<bool C, class T = void>
    using enable_if_t = typename std::enable_if<C, T>::type;

    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

#endif // CPP_EXTENSIONS_HPP
