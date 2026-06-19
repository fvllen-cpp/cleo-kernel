#pragma once

#include "utility.hpp"

namespace cleo {

template<typename E>
class unexpected {
public:
    // Default constructor
    constexpr unexpected(const unexpected&) = default;

    constexpr unexpected(unexpected&&) = default;

    // Constructor
    constexpr unexpected(const E& err) noexcept
        : error_(cleo::forward<E>(err)) {
    }

    // Constructor
    constexpr unexpected(E&& err) noexcept
        : error_(cleo::forward<E>(err)) {
    }

    // error
    constexpr const E& error() const& noexcept {
    }

    constexpr E& error() & noexcept {
    }

    constexpr const E&& error() const&& noexcept {
    }

    constexpr E&& error() && noexcept {
    }

    // swap
    constexpr void swap(unexpected& other) noexcept(cleo::is_nothrow_swappable_v<E>);

    template<class E2>
    friend constexpr bool operator==(unexpected& x, cleo::unexpected<E2>& y);

    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)));

private:
    E error_;
};

// Deduction guide
template<typename E>
unexpected(E) -> unexpected<E>;

template<typename T, typename E>
class expected {
public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = cleo::unexpected<E>;

    template<typename U>
    using rebind = expected<U, E>;

    // [expected.object.cons], constructors
    constexpr expected() noexcept {
    }

    constexpr expected(const expected& other) noexcept {
    }

    constexpr expected(expected&& other) noexcept {
    }

    constexpr expected(const T& value) noexcept {
    }

    constexpr expected(T&& value) {
    }

    constexpr expected(const cleo::unexpected<E>& unex_) noexcept
        : unexpected_(cleo::forward<E>(unex_)) {
    }

    constexpr expected(const cleo::unexpected<E>&& unex_) noexcept
        : unexpected_(cleo::forward<E>(unex_)) {
    }

    template<typename U, typename G>
    constexpr explicit expected(const expected<U, G>& other);

    template<typename U = cleo::remove_cv_t<T>>
    constexpr explicit(!cleo::is_convertible_v<U, T>) expected(U&& v);

    template<typename G>
    constexpr explicit(!cleo::is_convertible_v<const G&, E>) expected(const cleo::unexpected<G>& e);

    template<typename G>
    constexpr explicit(!cleo::is_convertible_v<G, E>) expected(cleo::unexpected<G>&& e);

    template<class... Args>
    constexpr expected(cleo::in_place_t in_place, Args&&...) noexcept {
    }

    template<typename U, class... Args>
    constexpr explicit expected(cleo::in_place_t, cleo::initializer_list<U> il, Args&&... args);

    constexpr explicit expected(cleo::in_place_t) noexcept;

    template<class... Args>
    constexpr explicit expected(cleo::unexpect_t, Args&&... args);

    // [expected.object.dtor], destructor
    constexpr ~expected() {
        if (has_value)
            value_.~T();
        else
            unexpected_.~E();
    }

    //

private:
    union {
        value_type value_;
        unexpected_type unexpected_;
    };

    bool has_value = false;
};

} // namespace cleo
