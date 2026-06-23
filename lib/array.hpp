#pragma once

#include "cstddef.hpp"
#include "iterator.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

namespace cleo {

template<typename T, cleo::size_t N>
    requires(N > 0)
class array {
public:
    using value_type = T;
    using size_type = cleo::size_t;
    using difference_type = cleo::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = cleo::reverse_iterator<iterator>;
    using const_reverse_iterator = cleo::reverse_iterator<const_iterator>;

    // operator=
    constexpr array& operator=(const array&) = default;
    constexpr array& operator=(array&&) = default;

    // element access
    [[nodiscard]] constexpr reference at(size_type pos) noexcept {
        return (*this)[pos];
    }

    [[nodiscard]] constexpr const_reference at(size_type pos) const noexcept {
        return (*this)[pos];
    }

    [[nodiscard]] constexpr reference operator[](size_type pos) {
        return buffer[pos];
    }

    [[nodiscard]] constexpr const_reference operator[](size_type pos) const {
        return buffer[pos];
    }

    [[nodiscard]] constexpr reference front() noexcept {
        return buffer[0];
    }

    [[nodiscard]] constexpr const_reference front() const noexcept {
        return buffer[0];
    }

    [[nodiscard]] constexpr reference back() noexcept {
        return buffer[N - 1];
    }

    [[nodiscard]] constexpr const_reference back() const noexcept {
        return buffer[N - 1];
    }

    [[nodiscard]] constexpr T* data() noexcept {
        return buffer;
    }

    [[nodiscard]] constexpr const T* data() const noexcept {
        return buffer;
    }

    // iterators
    [[nodiscard]] constexpr iterator begin() noexcept {
        return buffer;
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return buffer;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return buffer;
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        return buffer + N;
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return buffer + N;
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return buffer + N;
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    // capacity
    [[nodiscard]] constexpr bool empty() const noexcept {
        return N == 0;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return N;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return N;
    }

    // operations
    constexpr void fill(const T& value) {
        for (size_type i = 0; i < N; ++i) {
            buffer[i] = value;
        }
    }

    constexpr void swap(array& other) noexcept(cleo::is_nothrow_swappable_v<T>)
        requires cleo::is_swappable_v<T>
    {
        for (size_type i = 0; i < N; ++i) {
            cleo::swap(buffer[i], other.buffer[i]);
        }
    }

private:
    T buffer[N] = {};
};

// non-member functions

// helper classes
template<typename T>
struct tuple_size;

template<typename T, cleo::size_t N>
struct tuple_size<cleo::array<T, N>> : integral_constant<cleo::size_t, N> {};

template<typename T>
constexpr cleo::size_t tuple_size_v = tuple_size<T>::value;

} // namespace cleo
