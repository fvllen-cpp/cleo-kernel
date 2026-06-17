#pragma once

namespace cleo {

template<typename T>
[[nodiscard]] constexpr T* addressof(T& r) noexcept {
    return __builtin_addressof(r);
}

template<typename T>
const T* addressof(const T&&) = delete;

} // namespace cleo
