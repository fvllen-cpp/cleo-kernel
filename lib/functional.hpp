#pragma once

#include "type_traits.hpp"
#include "utility.hpp"

namespace cleo {

template<typename F, typename... Args>
constexpr invoke_result_t<F, Args...>
invoke(F&& f, Args&&... args) noexcept(is_nothrow_invocable_v<F, Args...>) {
    return detail::invoke_impl(cleo::forward<F>(f), cleo::forward<Args>(args)...);
}

template<typename R, typename F, typename... Args>
constexpr R invoke_r(F&& f, Args&&... args) noexcept(is_nothrow_invocable_r_v<R, F, Args...>) {
    if constexpr (is_void_v<R>)
        detail::invoke_impl(cleo::forward<F>(f), cleo::forward<Args>(args)...);
    else
        return detail::invoke_impl(cleo::forward<F>(f), cleo::forward<Args>(args)...);
}

} // namespace cleo
