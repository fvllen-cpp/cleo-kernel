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

} // namespace cleo
