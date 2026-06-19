#pragma once

#include "concepts.hpp"
#include "cstddef.hpp"
#include "type_traits.hpp"

namespace cleo {

// iterator tag chain
struct input_iterator_tag {};

struct output_iterator_tag {};

struct forward_iterator_tag : input_iterator_tag {};

struct bidirectional_iterator_tag : forward_iterator_tag {};

struct random_access_iterator_tag : bidirectional_iterator_tag {};

struct contiguous_iterator_tag : random_access_iterator_tag {};

// exposition helpers
template<typename T>
using with_reference = T&;

template<typename T>
concept can_reference = requires { typename with_reference<T>; };

template<typename T>
concept dereferenceable = requires(T& t) {
    { *t } -> can_reference;
};

// [iterator_traits]
template<typename Iter>
struct iterator_traits {
    using iterator_category = typename Iter::iterator_category;
    using value_type = typename Iter::value_type;
    using difference_type = typename Iter::difference_type;
    using pointer = typename Iter::pointer;
    using reference = typename Iter::reference;
};

template<typename T>
    requires cleo::is_object_v<T>
struct iterator_traits<T*> {
    using iterator_concept = cleo::contiguous_iterator_tag;
    using iterator_category = cleo::random_access_iterator_tag;
    using value_type = cleo::remove_cv_t<T>;
    using difference_type = cleo::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
};

// [iterator.cust.move], ranges::iter_move

namespace ranges {
namespace detail {
void iter_move() = delete;

template<typename T>
concept has_custom_iter_move = requires(T&& it) { iter_move(cleo::forward<T>(it)); };

template<typename T>
concept can_dereference = requires(T&& it) { *cleo::forward<T>(it); };

template<typename T>
using deref_t = decltype(*cleo::declval<T>());

template<typename T>
concept derefs_to_lvalue = can_dereference<T> && cleo::is_lvalue_reference_v<deref_t<T>>;

struct iter_move_fn {
    // 1
    template<typename T>
        requires has_custom_iter_move<T>
    constexpr decltype(auto)
    operator()(T&& it) const noexcept(noexcept(iter_move(cleo::forward<T>(it)))) {
        return iter_move(cleo::forward<T>(it));
    }

    // 2
    template<typename T>
        requires(!has_custom_iter_move<T>) && derefs_to_lvalue<T>
    constexpr decltype(auto)
    operator()(T&& it) const noexcept(noexcept(cleo::move(*cleo::forward<T>(it)))) {
        return cleo::move(*cleo::forward<T>(it));
    }

    // 3
    template<typename T>
        requires(!has_custom_iter_move<T>) && can_dereference<T> && (!derefs_to_lvalue<T>)
    constexpr decltype(auto) operator()(T&& it) const noexcept(noexcept(*cleo::forward<T>(it))) {
        return *cleo::forward<T>(it);
    }
};
} // namespace detail

inline namespace cpo {
inline constexpr detail::iter_move_fn iter_move{};
}
} // namespace ranges

// types

template<typename T>
using iter_value_t = typename iterator_traits<remove_cvref_t<T>>::value_type;

template<dereferenceable T>
using iter_reference_t = decltype(*cleo::declval<T&>());

template<typename T>
using iter_difference_t = typename iterator_traits<remove_cvref_t<T>>::difference_type;

template<dereferenceable T>
using iter_rvalue_reference_t = decltype(ranges::iter_move(cleo::declval<T&>()));

// [indirectly_readable]
template<typename In>
concept __IndirectlyReadableImpl =
    requires(const In in) {
        typename iter_value_t<In>;
        typename iter_reference_t<In>;
        typename iter_rvalue_reference_t<In>;
        { *in } -> cleo::same_as<iter_reference_t<In>>;
        { ranges::iter_move(in) } -> cleo::same_as<iter_rvalue_reference_t<In>>;
    } && cleo::common_reference_with<iter_reference_t<In>&&, iter_value_t<In>&> &&
    cleo::common_reference_with<iter_reference_t<In>&&, iter_rvalue_reference_t<In>&&> &&
    cleo::common_reference_with<iter_rvalue_reference_t<In>&&, const iter_value_t<In>&>;

template<typename In>
concept indirectly_readable = __IndirectlyReadableImpl<cleo::remove_cvref_t<In>>;

// [indirect_value_t]
template<cleo::indirectly_readable T>
using indirect_value_t = cleo::iter_value_t<T>&;

// [iter_common_reference_t]
template<cleo::indirectly_readable T>
using iter_common_reference_t =
    cleo::common_reference_t<cleo::iter_reference_t<T>, cleo::indirect_value_t<T>>;

// [iter_const_reference_t]
template<cleo::indirectly_readable T>
using iter_const_reference_t =
    cleo::common_reference_t<const cleo::iter_value_t<T>&&, cleo::iter_reference_t<T>>;

// [reverse_iterator]
template<typename Iter>
class reverse_iterator {
public:
    using iterator_type = Iter;
    using iterator_concept = cleo::bidirectional_iterator_tag;
    using iterator_category = typename cleo::iterator_traits<Iter>::iterator_category;
    using value_type = cleo::iter_value_t<Iter>;
    using difference_type = cleo::iter_difference_t<Iter>;
    using pointer = typename cleo::iterator_traits<Iter>::pointer;
    using reference = cleo::iter_reference_t<Iter>;

    // constructor
    constexpr reverse_iterator()
        : current_() {
    }

    constexpr explicit reverse_iterator(iterator_type x)
        : current_(x) {
    }

    template<typename U>
        requires(!cleo::is_same_v<U, Iter>) && cleo::convertible_to<U, Iter>
    constexpr reverse_iterator(const reverse_iterator<U>& other)
        : current_(other.base()) {
    }

    // destructor
    // implicit declaration

    template<typename U>
        requires(!cleo::is_same_v<U, Iter>) && cleo::convertible_to<U, Iter> &&
                cleo::assignable_from<Iter&, U>
    constexpr reverse_iterator& operator=(const reverse_iterator<U>& other) {
        current_ = other.base();
        return *this;
    }

    constexpr iterator_type base() const {
        return current_;
    }

    constexpr reference operator*() const {
        Iter tmp = current_;
        return *--tmp;
    }

    constexpr pointer operator->() const
        requires(cleo::is_pointer_v<Iter> || requires(const Iter i) { i.operator->(); })
    {
        Iter it = current_;
        --it;

        if constexpr (cleo::is_pointer_v<Iter>) {
            return it;
        }
        else {
            return it.operator->();
        }
    }

    constexpr reverse_iterator& operator++() {
        --current_;
        return *this;
    }

    constexpr reverse_iterator& operator--() {
        ++current_;
        return *this;
    }

    constexpr reverse_iterator operator++(int) {
        reverse_iterator it = *this;
        --current_;
        return it;
    }

    constexpr reverse_iterator operator--(int) {
        reverse_iterator it = *this;
        --current_;
        return it;
    }

    constexpr reverse_iterator operator+(difference_type n) const {
        return reverse_iterator(base() - n);
    }

    constexpr reverse_iterator operator-(difference_type n) const {
        return reverse_iterator(base() + n);
    }

    constexpr reverse_iterator& operator+=(difference_type n) {
        current_ -= n;
        return *this;
    }

    constexpr reverse_iterator& operator-=(difference_type n) {
        current_ += n;
        return *this;
    }

private:
    Iter current_;
};

} // namespace cleo
