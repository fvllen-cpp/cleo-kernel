#pragma once

#include "cstddef.hpp"
#include "type_traits.hpp"

namespace cleo {

template<typename T, cleo::size_t N>
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

    // constructor
    array() = default;

    // destructor
    ~array() = default;

    // operator=
    constexpr reference operator=(const array&) = default;
    constexpr reference operator=(array&&) = default;
    constexpr const_reference operator=(const array&) const = default;
    constexpr const_reference operator=(array&&) const = default;

    // element access
    [[nodiscard]] constexpr reference at(size_type pos) {
    }

    [[nodiscard]] constexpr const_reference at(size_type pos) const {
    }

    [[nodiscard]] constexpr reference operator[](size_type pos) {
    }

    [[nodiscard]] constexpr const_reference operator[](size_type pos) const {
    }

    [[nodiscard]] constexpr reference front() noexcept {
    }

    [[nodiscard]] constexpr const_reference front() const noexcept {
    }

    [[nodiscard]] constexpr reference back() noexcept {
    }

    [[nodiscard]] constexpr const_reference back() const noexcept {
    }

    [[nodiscard]] constexpr T* data() noexcept {
    }

    [[nodiscard]] constexpr const T* data() const noexcept {
    }

    // iterators
    [[nodiscard]] constexpr iterator begin() noexcept {
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept {
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    }

    [[nodiscard]] constexpr iterator end() noexcept {
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept {
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept {
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    }

    // capacity
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return size_;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return N;
    }

    // operations
    constexpr void fill(const T& value) {
    }

    constexpr void swap(array& other) noexcept {
    }

private:
    T buffer[N] = {0};
    cleo::size_t size_ = 0;
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
