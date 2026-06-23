#pragma once

#include "concepts.hpp"
#include "memory.hpp"
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

// reference_wrapper
namespace detail {
template<typename T>
constexpr T& FUN(T& t) noexcept {
    return t;
}

template<typename T>
void FUN(T&&) = delete;
} // namespace detail

template<typename T>
class reference_wrapper {
public:
    using type = T;

    template<
        typename U,
        typename =
            decltype(detail::FUN<T>(cleo::declval<U>()), cleo::enable_if_t<!cleo::is_same_v<reference_wrapper, cleo::remove_cvref_t<U>>>())>
    constexpr reference_wrapper(U&& u) noexcept(noexcept(detail::FUN<T>(cleo::forward<U>(u))))
        : ptr_(cleo::addressof(detail::FUN<T>(cleo::forward<U>(u)))) {
    }

    reference_wrapper(const reference_wrapper&) noexcept = default;

    reference_wrapper& operator=(const reference_wrapper& x) noexcept = default;

    constexpr operator T&() const noexcept {
        return *ptr_;
    }

    constexpr T& get() const noexcept {
        return *ptr_;
    }

    template<typename... ArgTypes>
    constexpr cleo::invoke_result_t<T&, ArgTypes...>
    operator()(ArgTypes&&... args) const noexcept(cleo::is_nothrow_invocable_v<T&, ArgTypes...>) {
        return cleo::invoke(get(), cleo::forward<ArgTypes>(args)...);
    }

private:
    T* ptr_;
};

template<typename T>
reference_wrapper(T&) -> reference_wrapper<T>;

// [identity]
struct identity {
    using is_transparent = void;

    template<typename T>
    constexpr T&& operator()(T&& t) const noexcept {
        return cleo::forward<T>(t);
    }
};

// [ranges.less]
namespace ranges {
struct less {
    using is_transparent = void;

    template<typename T, typename U>
        requires cleo::totally_ordered_with<T, U>
    constexpr bool operator()(T&& t, U&& u) const {
        return static_cast<bool>(cleo::forward<T>(t) < cleo::forward<U>(u));
    }
};
} // namespace ranges

} // namespace cleo
