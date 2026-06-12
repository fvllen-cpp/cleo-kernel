#pragma once

#include <stddef.h>

#include "type_traits.hpp"

namespace cleo {

// [forward], forward/move helpers

template<class T>
[[nodiscard]] constexpr cleo::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<cleo::remove_reference_t<T>&&>(t);
}

template<class T>
[[nodiscard]] constexpr T&& forward(cleo::remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}

template<class T>
[[nodiscard]] constexpr T&& forward(cleo::remove_reference_t<T>&& t) noexcept {
    static_assert(
        !cleo::is_lvalue_reference_v<T>,
        "cleo::forward must not be used to convert an rvalue to an lvalue"
    );
    return static_cast<T&&>(t);
}

// [utility.syn], in-place construction tags

struct in_place_t {
    explicit constexpr in_place_t() = default;
};

inline constexpr in_place_t in_place{};

template<class T>
struct in_place_type_t {
    explicit constexpr in_place_type_t() = default;
};

template<class T>
inline constexpr in_place_type_t<T> in_place_type{};

template<size_t I>
struct in_place_index_t {
    explicit constexpr in_place_index_t() = default;
};

template<size_t I>
inline constexpr in_place_index_t<I> in_place_index{};

// swap
template<typename T>
    requires is_move_constructible_v<T> && is_move_assignable_v<T>
constexpr void
swap(T& a, T& b) noexcept(is_nothrow_move_constructible_v<T> && is_nothrow_move_assignable_v<T>) {
    T tmp = cleo::move(a);
    a = cleo::move(b);
    b = cleo::move(tmp);
}

// [meta.unary.prop] swappable traits — live here (not type_traits.hpp) because they
// need cleo::swap, and the `using cleo::swap;` + unqualified call enables ADL.

// is_swappable_with
namespace detail {
using cleo::swap;

template<typename T, typename U, typename = void>
struct swappable_with_impl : false_type {};

template<typename T, typename U>
struct swappable_with_impl<
    T,
    U,
    void_t<
        decltype(swap(cleo::declval<T>(), cleo::declval<U>())),
        decltype(swap(cleo::declval<U>(), cleo::declval<T>()))>> : true_type {};
} // namespace detail

template<typename T, typename U>
struct is_swappable_with : detail::swappable_with_impl<T, U> {};

template<typename T, typename U>
inline constexpr bool is_swappable_with_v = is_swappable_with<T, U>::value;

// is_swappable
template<typename T>
struct is_swappable : is_swappable_with<add_lvalue_reference_t<T>, add_lvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_swappable_v = is_swappable<T>::value;

// is_nothrow_swappable_with (guard the noexcept probe behind swappability)
namespace detail {
using cleo::swap;

template<typename T, typename U, bool = is_swappable_with<T, U>::value>
struct nothrow_swappable_with_impl : false_type {};

template<typename T, typename U>
struct nothrow_swappable_with_impl<T, U, true>
    : bool_constant<
          noexcept(swap(cleo::declval<T>(), cleo::declval<U>()))
          && noexcept(swap(cleo::declval<U>(), cleo::declval<T>()))> {};
} // namespace detail

template<typename T, typename U>
struct is_nothrow_swappable_with : detail::nothrow_swappable_with_impl<T, U> {};

template<typename T, typename U>
inline constexpr bool is_nothrow_swappable_with_v = is_nothrow_swappable_with<T, U>::value;

// is_nothrow_swappable
template<typename T>
struct is_nothrow_swappable
    : is_nothrow_swappable_with<add_lvalue_reference_t<T>, add_lvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_nothrow_swappable_v = is_nothrow_swappable<T>::value;

} // namespace cleo
