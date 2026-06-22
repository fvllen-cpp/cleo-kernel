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
// iter_move
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
} // namespace cpo
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

// [indirectly_writable]
template<typename Out, typename T>
concept indirectly_writable = requires(Out&& out, T&& value) {
    *out = cleo::forward<T>(value);
    *cleo::forward<Out>(out) = cleo::forward<T>(value);
    const_cast<const cleo::iter_reference_t<Out>&&>(*out) = cleo::forward<T>(value);
    const_cast<const cleo::iter_reference_t<Out>&&>(*cleo::forward<Out>(out)) =
        cleo::forward<T>(value);
};

// [indirectly_movable]
template<typename In, typename Out>
concept indirectly_movable = cleo::indirectly_readable<In> &&
                             cleo::indirectly_writable<Out, cleo::iter_rvalue_reference_t<In>>;

template<typename In, typename Out>
concept indirectly_movable_storable =
    cleo::indirectly_movable<In, Out> && cleo::indirectly_writable<Out, cleo::iter_value_t<In>> &&
    cleo::movable<cleo::iter_value_t<In>> &&
    cleo::constructible_from<cleo::iter_value_t<In>, cleo::iter_rvalue_reference_t<In>> &&
    cleo::assignable_from<cleo::iter_value_t<In>&, cleo::iter_rvalue_reference_t<In>>;

// [iterator.cust.swap], ranges::iter_swap
namespace ranges {
namespace detail {
void iter_swap() = delete;

template<typename T, typename U>
concept has_custom_iter_swap =
    (cleo::detail::__class_or_enum<cleo::remove_cvref_t<T>> ||
     cleo::detail::__class_or_enum<cleo::remove_cvref_t<U>>) &&
    requires(T&& t, U&& u) { iter_swap(cleo::forward<T>(t), cleo::forward<U>(u)); };

template<typename T, typename U>
concept can_ranges_swap_deref =
    requires(T&& t, U&& u) { cleo::ranges::swap(*cleo::forward<T>(t), *cleo::forward<U>(u)); };

template<typename T, typename U>
concept can_exchange_iter_swap =
    cleo::indirectly_movable_storable<cleo::remove_cvref_t<T>, cleo::remove_cvref_t<U>> &&
    cleo::indirectly_movable_storable<cleo::remove_cvref_t<U>, cleo::remove_cvref_t<T>>;

struct iter_swap_fn {
    template<typename T, typename U>
        requires has_custom_iter_swap<T, U>
    constexpr void operator()(T&& t, U&& u) const noexcept(
        noexcept(iter_swap(cleo::forward<T>(t), cleo::forward<U>(u)))
    ) {
        iter_swap(cleo::forward<T>(t), cleo::forward<U>(u));
    }

    template<typename T, typename U>
        requires(!has_custom_iter_swap<T, U>) && can_ranges_swap_deref<T, U>
    constexpr void operator()(T&& t, U&& u) const noexcept(
        noexcept(cleo::ranges::swap(*cleo::forward<T>(t), *cleo::forward<U>(u)))
    ) {
        cleo::ranges::swap(*cleo::forward<T>(t), *cleo::forward<U>(u));
    }

    template<typename T, typename U>
        requires(!has_custom_iter_swap<T, U>) && (!can_ranges_swap_deref<T, U>) &&
                can_exchange_iter_swap<T, U>
    constexpr void operator()(T&& t, U&& u) const {
        auto&& x = cleo::forward<T>(t);
        auto&& y = cleo::forward<U>(u);

        using X = cleo::remove_cvref_t<T>;
        cleo::iter_value_t<X> tmp(cleo::ranges::iter_move(x));
        *x = cleo::ranges::iter_move(y);
        *y = cleo::move(tmp);
    }
};
} // namespace detail

inline namespace cpo {
inline constexpr detail::iter_swap_fn iter_swap{};
} // namespace cpo
} // namespace ranges

// [indirectly_swappable]
template<typename Iter1, typename Iter2 = Iter1>
concept indirectly_swappable =
    cleo::indirectly_readable<Iter1> && cleo::indirectly_readable<Iter2> &&
    requires(const Iter1 it1, const Iter2 it2) {
        cleo::ranges::iter_swap(it1, it1);
        cleo::ranges::iter_swap(it2, it2);
        cleo::ranges::iter_swap(it1, it2);
        cleo::ranges::iter_swap(it2, it1);
    };

// [reverse_iterator]
template<typename Iter>
class reverse_iterator {
public:
    using iterator_type = Iter;
    using iterator_concept = cleo::random_access_iterator_tag;
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
        requires(!cleo::is_same_v<U, Iter>) && cleo::convertible_to<const U&, Iter>
    constexpr reverse_iterator(const reverse_iterator<U>& other)
        : current_(other.base()) {
    }

    // destructor
    // implicit declaration

    template<typename U>
        requires(!cleo::is_same_v<U, Iter>) && cleo::convertible_to<const U&, Iter> &&
                cleo::assignable_from<Iter&, const U&>
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

    constexpr reference operator[](difference_type n) const {
        return *(*this + n);
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
        ++current_;
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

    friend constexpr cleo::iter_rvalue_reference_t<Iter>
    iter_move(const reverse_iterator& i) noexcept {
        auto it = i.base();
        return cleo::ranges::iter_move(--it);
    }

    template<typename Iter2>
        requires cleo::indirectly_swappable<Iter, Iter2>
    friend constexpr void
    iter_swap(const reverse_iterator& x, const reverse_iterator<Iter2>& y) noexcept(
        noexcept(cleo::ranges::iter_swap(--cleo::declval<Iter&>(), --cleo::declval<Iter2&>()))
    ) {
        auto it_x = x.base();
        auto it_y = y.base();
        ranges::iter_swap(--it_x, --it_y);
    }

private:
    Iter current_;
};

template<typename Iter1, typename Iter2>
constexpr bool operator==(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs) {
    return lhs.base() == rhs.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator!=(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs) {
    return lhs.base() != rhs.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator<(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs) {
    return rhs.base() < lhs.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator<=(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs) {
    return rhs.base() <= lhs.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator>(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs) {
    return rhs.base() > lhs.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator>=(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs) {
    return rhs.base() >= lhs.base();
}

template<typename Iter>
constexpr reverse_iterator<Iter>
operator+(cleo::iter_difference_t<Iter> n, const reverse_iterator<Iter>& it) {
    return reverse_iterator<Iter>(it.base() - n);
}

template<typename Iter1, typename Iter2>
constexpr auto operator-(const reverse_iterator<Iter1>& lhs, const reverse_iterator<Iter2>& rhs)
    -> decltype(rhs.base() - lhs.base()) {
    return rhs.base() - lhs.base();
}

template<typename Iter>
constexpr reverse_iterator<Iter> make_reverse_iterator(Iter i) {
    return reverse_iterator<Iter>(i);
}

} // namespace cleo
