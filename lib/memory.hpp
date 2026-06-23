#pragma once

#include "type_traits.hpp"

namespace cleo {

// [addressof]
template<typename T>
[[nodiscard]] constexpr T* addressof(T& r) noexcept {
    return __builtin_addressof(r);
}

template<typename T>
const T* addressof(const T&&) = delete;

// [to_address]
template<typename T>
constexpr T* to_address(T* p) noexcept
    requires(!cleo::is_function_v<T>)
{
    return p;
}

template<typename Ptr>
constexpr auto to_address(const Ptr& p) noexcept {
    if constexpr (requires { cleo::pointer_traits<Ptr>::to_address(p); })
        return cleo::pointer_traits<T>::to_address(p);
    else
        return cleo::to_address(p.operator->());
}

} // namespace cleo
