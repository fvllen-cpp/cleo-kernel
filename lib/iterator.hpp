#pragma once

#include "concepts.hpp"
#include "cstddef.hpp"
#include "type_traits.hpp"

namespace cleo {

// exposition
template<typename T>
concept integer_like = cleo::integral<T> && !cleo::same_as<cleo::remove_cvref_t<T>, bool>;

template<typename T>
concept signed_integer_like = integer_like<T> && cleo::is_signed_v<T>;

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

// [incrementable_traits]
template<typename I>
struct incrementable_traits {};

template<typename T>
    requires cleo::is_object_v<T>
struct incrementable_traits<T*> {
    using difference_type = cleo::ptrdiff_t;
};

template<typename T>
struct incrementable_traits<const T> : incrementable_traits<T> {};

template<typename T>
    requires requires { typename T::difference_type; }
struct incrementable_traits<T> {
    using difference_type = typename T::difference_type;
};

template<typename T>
    requires(!requires { typename T::difference_type; }) && requires(const T& a, const T& b) {
        { a - b } -> cleo::integral;
    }
struct incrementable_traits<T> {
    using difference_type = cleo::make_signed_t<decltype(cleo::declval<T>() - cleo::declval<T>())>;
};

// [indirectly_readable_traits]
template<typename>
struct cond_value_type {};

template<typename T>
    requires cleo::is_object_v<T>
struct cond_value_type<T> {
    using value_type = cleo::remove_cv_t<T>;
};

template<typename T>
concept has_member_value_type = requires { typename T::value_type; };

template<typename T>
concept has_member_element_type = requires { typename T::element_type; };

template<typename I>
struct indirectly_readable_traits {};

template<typename T>
struct indirectly_readable_traits<T*> : cond_value_type<T> {};

template<typename I>
    requires cleo::is_array_v<I>
struct indirectly_readable_traits<I> {
    using value_type = cleo::remove_cv_t<cleo::remove_extent_t<I>>;
};

template<typename T>
struct indirectly_readable_traits<const T> : indirectly_readable_traits<T> {};

template<has_member_value_type T>
struct indirectly_readable_traits<T> : cond_value_type<typename T::value_type> {};

template<has_member_element_type T>
struct indirectly_readable_traits<T> : cond_value_type<typename T::element_type> {};

template<has_member_value_type T>
    requires has_member_element_type<T>
struct indirectly_readable_traits<T> {};

template<has_member_value_type T>
    requires has_member_element_type<T> && cleo::same_as<
                                               cleo::remove_cv_t<typename T::element_type>,
                                               cleo::remove_cv_t<typename T::value_type>>
struct indirectly_readable_traits<T> : cond_value_type<typename T::value_type> {};

namespace detail {
struct iterator_traits_primary_tag {};
} // namespace detail

// [iterator_traits]
template<typename Iter>
struct iterator_traits {
    using primary_template = detail::iterator_traits_primary_tag;
};

template<typename Iter>
    requires requires {
        typename Iter::iterator_category;
        typename Iter::value_type;
        typename Iter::difference_type;
        typename Iter::pointer;
        typename Iter::reference;
    }
struct iterator_traits<Iter> {
    using primary_template = detail::iterator_traits_primary_tag;

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

namespace detail {

template<typename I>
concept iterator_traits_from_primary =
    requires { typename cleo::iterator_traits<I>::primary_template; } &&
    cleo::
        is_same_v<typename cleo::iterator_traits<I>::primary_template, iterator_traits_primary_tag>;

template<typename I>
struct iter_traits {
    using type = cleo::iterator_traits<I>;
};

template<typename I>
    requires iterator_traits_from_primary<I>
struct iter_traits<I> {
    using type = I;
};

template<typename I>
using ITER_TRAITS = typename iter_traits<I>::type;

template<typename I, typename Traits = ITER_TRAITS<I>>
struct iter_concept_impl {};

template<typename I, typename Traits>
    requires requires { typename Traits::iterator_concept; }
struct iter_concept_impl<I, Traits> {
    using type = typename Traits::iterator_concept;
};

template<typename I, typename Traits>
    requires(!requires { typename Traits::iterator_concept; }) &&
            requires { typename Traits::iterator_category; }
struct iter_concept_impl<I, Traits> {
    using type = typename Traits::iterator_category;
};

template<typename I, typename Traits>
    requires(!requires { typename Traits::iterator_concept; }) &&
            (!requires { typename Traits::iterator_category; }) && iterator_traits_from_primary<I>
struct iter_concept_impl<I, Traits> {
    using type = cleo::random_access_iterator_tag;
};

} // namespace detail

template<typename I>
using ITER_CONCEPT = typename detail::iter_concept_impl<I>::type;

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

namespace detail {
template<typename T>
concept has_iterator_traits_value_type =
    requires { typename iterator_traits<cleo::remove_cvref_t<T>>::value_type; };

template<typename T, bool = has_iterator_traits_value_type<T>>
struct iter_value_impl : indirectly_readable_traits<cleo::remove_cvref_t<T>> {};

template<typename T>
struct iter_value_impl<T, true> : iterator_traits<cleo::remove_cvref_t<T>> {};

template<typename T>
concept has_iterator_traits_difference_type =
    requires { typename iterator_traits<cleo::remove_cvref_t<T>>::difference_type; };

template<typename T, bool = has_iterator_traits_difference_type<T>>
struct iter_difference_impl : incrementable_traits<cleo::remove_cvref_t<T>> {};

template<typename T>
struct iter_difference_impl<T, true> : iterator_traits<cleo::remove_cvref_t<T>> {};

} // namespace detail

template<typename T>
using iter_value_t = typename detail::iter_value_impl<T>::value_type;

template<dereferenceable T>
using iter_reference_t = decltype(*cleo::declval<T&>());

template<typename T>
using iter_difference_t = typename detail::iter_difference_impl<T>::difference_type;

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

// [weakly_incrementable]
template<typename I>
concept weakly_incrementable = cleo::movable<I> && requires(I i) {
    typename cleo::iter_difference_t<I>;
    requires cleo::signed_integer_like<cleo::iter_difference_t<I>>;
    { ++i } -> cleo::same_as<I&>;
    i++;
};

// [incrementable]
template<typename I>
concept incrementable = cleo::regular<I> && cleo::weakly_incrementable<I> && requires(I i) {
    { i++ } -> cleo::same_as<I>;
};

// [input_or_output_iterator]
template<typename I>
concept input_or_output_iterator = requires(I i) {
    { *i } -> cleo::can_reference;
} && cleo::weakly_incrementable<I>;

// [sentinel_for]
template<typename S, typename I>
concept sentinel_for = cleo::semiregular<S> && cleo::input_or_output_iterator<I> &&
                       __WeaklyEqualityComparableWith<S, I>;

// [sized_sentinel_for]
template<typename S, typename I>
inline constexpr bool disable_sized_sentinel_for = false;

template<typename S, typename I>
concept sized_sentinel_for =
    cleo::sentinel_for<S, I> &&
    !cleo::disable_sized_sentinel_for<cleo::remove_cv_t<S>, cleo::remove_cv_t<I>> &&
    requires(const I& i, const S& s) {
        { s - i } -> cleo::same_as<cleo::iter_difference_t<I>>;
        { i - s } -> cleo::same_as<cleo::iter_difference_t<I>>;
    };

// [input_iterator]
template<typename I>
concept input_iterator = cleo::input_or_output_iterator<I> && cleo::indirectly_readable<I> &&
                         requires { typename ITER_CONCEPT<I>; } &&
                         cleo::derived_from<ITER_CONCEPT<I>, cleo::input_iterator_tag>;

// [output_iterator]
template<typename I, typename T>
concept output_iterator = cleo::input_or_output_iterator<I> && cleo::indirectly_writable<I, T> &&
                          requires(I i, T&& t) { *i++ = cleo::forward<T>(t); };

// [forward_iterator]
template<typename I>
concept forward_iterator =
    cleo::input_iterator<I> && cleo::derived_from<ITER_CONCEPT<I>, cleo::forward_iterator_tag> &&
    cleo::incrementable<I> && cleo::sentinel_for<I, I>;

// [bidirectional_iterator]
template<typename I>
concept bidirectional_iterator =
    cleo::forward_iterator<I> &&
    cleo::derived_from<ITER_CONCEPT<I>, cleo::bidirectional_iterator_tag> && requires(I i) {
        { --i } -> cleo::same_as<I&>;
        { i-- } -> cleo::same_as<I>;
    };

// [random_access_iterator]
template<typename I>
concept random_access_iterator =
    cleo::bidirectional_iterator<I> &&
    cleo::derived_from<ITER_CONCEPT<I>, cleo::random_access_iterator_tag> &&
    cleo::totally_ordered<I> && cleo::sized_sentinel_for<I, I> &&
    requires(I i, const I j, const cleo::iter_difference_t<I> n) {
        { i += n } -> cleo::same_as<I&>;
        { j + n } -> cleo::same_as<I>;
        { n + j } -> cleo::same_as<I>;
        { i -= n } -> cleo::same_as<I&>;
        { j - n } -> cleo::same_as<I>;
        { j[n] } -> cleo::same_as<cleo::iter_reference_t<I>>;
    };

// [contiguous_iterator]
template<typename I>
concept contiguous_iterator =
    cleo::random_access_iterator<I> &&
    cleo::derived_from<ITER_CONCEPT<I>, cleo::contiguous_iterator_tag> &&
    cleo::is_lvalue_reference_v<cleo::iter_reference_t<I>> &&
    cleo::same_as<cleo::iter_value_t<I>, cleo::remove_cvref_t<cleo::iter_reference_t<I>>> &&
    requires(const I& i) {
        { cleo::to_address(i) } -> cleo::same_as<cleo::add_pointer_t<cleo::iter_reference_t<I>>>;
    };

// [indirectly_unary_invocable]
template<typename F, typename I>
concept indirectly_unary_invocable = cleo::indirectly_readable<I> && cleo::copy_constructible<F> &&
                                     cleo::invocable<F&, cleo::indirect_value_t<I>> &&
                                     cleo::invocable<F&, cleo::iter_reference_t<I>> &&
                                     cleo::common_reference_with<
                                         cleo::invoke_result_t<F&, cleo::indirect_value_t<I>>,
                                         cleo::invoke_result_t<F&, cleo::iter_reference_t<I>>>;

// [indireclty_regular_unary_invocable]
template<typename F, typename I>
concept indirectly_regular_unary_invocable =
    cleo::indirectly_readable<I> && cleo::copy_constructible<F> &&
    cleo::regular_invocable<F&, cleo::indirect_value_t<I>> &&
    cleo::regular_invocable<F&, cleo::iter_reference_t<I>> &&
    cleo::common_reference_with<
        cleo::invoke_result_t<F&, cleo::indirect_value_t<I>>,
        cleo::invoke_result_t<F&, cleo::iter_reference_t<I>>>;

// [indirect_unary_predicate]
template<typename F, typename I>
concept indirect_unary_predicate = cleo::indirectly_readable<I> && cleo::copy_constructible<F> &&
                                   cleo::predicate<F&, cleo::indirect_value_t<I>> &&
                                   cleo::predicate<F&, cleo::iter_reference_t<I>>;

// [indirect_binary_predicate]
template<typename F, typename I1, typename I2>
concept indirect_binary_predicate =
    cleo::indirectly_readable<I1> && cleo::indirectly_readable<I2> && cleo::copy_constructible<F> &&
    cleo::predicate<F&, cleo::indirect_value_t<I1>, cleo::indirect_value_t<I2>> &&
    cleo::predicate<F&, cleo::indirect_value_t<I1>, cleo::iter_reference_t<I2>> &&
    cleo::predicate<F&, cleo::iter_reference_t<I1>, cleo::indirect_value_t<I2>> &&
    cleo::predicate<F&, cleo::iter_reference_t<I1>, cleo::iter_reference_t<I2>>;

// [indirect_equivalence_relation]
template<typename F, typename I1, typename I2 = I1>
concept indirect_equivalence_relation =
    cleo::indirectly_readable<I1> && cleo::indirectly_readable<I2> && cleo::copy_constructible<F> &&
    cleo::equivalence_relation<F&, cleo::indirect_value_t<I1>, cleo::indirect_value_t<I2>> &&
    cleo::equivalence_relation<F&, cleo::indirect_value_t<I1>, cleo::iter_reference_t<I2>> &&
    cleo::equivalence_relation<F&, cleo::iter_reference_t<I1>, cleo::indirect_value_t<I2>> &&
    cleo::equivalence_relation<F&, cleo::iter_reference_t<I1>, cleo::iter_reference_t<I2>>;

// [indirect_strict_weak_order]
template<typename F, typename I1, typename I2 = I1>
concept indirect_strict_weak_order =
    cleo::indirectly_readable<I1> && cleo::indirectly_readable<I2> && cleo::copy_constructible<F> &&
    cleo::strict_weak_order<F&, cleo::indirect_value_t<I1>, cleo::indirect_value_t<I2>> &&
    cleo::strict_weak_order<F&, cleo::indirect_value_t<I1>, cleo::iter_reference_t<I2>> &&
    cleo::strict_weak_order<F&, cleo::iter_reference_t<I1>, cleo::indirect_value_t<I2>> &&
    cleo::strict_weak_order<F&, cleo::iter_reference_t<I1>, cleo::iter_reference_t<I2>>;

// [indirectly_movable]
template<typename In, typename Out>
concept indirectly_movable = cleo::indirectly_readable<In> &&
                             cleo::indirectly_writable<Out, cleo::iter_rvalue_reference_t<In>>;

// [indirectly_movable_storable]
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

// [indirect_result_t]
template<typename F, typename... Is>
    requires(cleo::indirectly_readable<Is> && ...) &&
                cleo::invocable<F, cleo::iter_reference_t<Is>...>
using indirect_result_t = cleo::invoke_result_t<F, cleo::iter_reference_t<Is>...>;

// [projected]
template<cleo::indirectly_readable I, cleo::indirectly_regular_unary_invocable<I> Proj>
struct projected {
    using value_type = cleo::remove_cvref_t<cleo::indirect_result_t<Proj&, I>>;
    cleo::indirect_result_t<Proj&, I> operator*() const;
};

template<cleo::weakly_incrementable I, typename Proj>
struct incrementable_traits<cleo::projected<I, Proj>> {
    using difference_type = cleo::iter_difference_t<I>;
};

// [indirectly_copyable]
template<typename In, typename Out>
concept indirectly_copyable =
    cleo::indirectly_readable<In> && cleo::indirectly_writable<Out, cleo::iter_reference_t<In>>;

// [indirectly_copyable_storable]
template<typename In, typename Out>
concept indirectly_copyable_storable =
    cleo::indirectly_copyable<In, Out> && cleo::indirectly_writable<Out, cleo::iter_value_t<In>&> &&
    cleo::indirectly_writable<Out, const cleo::iter_value_t<In>&> &&
    cleo::indirectly_writable<Out, cleo::iter_value_t<In>&&> &&
    cleo::indirectly_writable<Out, const cleo::iter_value_t<In>&&> &&
    cleo::copyable<cleo::iter_value_t<In>> &&
    cleo::constructible_from<cleo::iter_value_t<In>, cleo::iter_reference_t<In>> &&
    cleo::assignable_from<cleo::iter_value_t<In>&, cleo::iter_reference_t<In>>;

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

// [indirectly_comparable]
template<
    typename I1,
    typename I2,
    typename Comp,
    typename Proj1 = cleo::identity,
    typename Proj2 = cleo::identity>
concept indirectly_comparable =
    cleo::indirect_binary_predicate<Comp, cleo::projected<I1, Proj1>, cleo::projected<I2, Proj2>>;

// [permutable]
template<typename I>
concept permutable = cleo::forward_iterator<I> && cleo::indirectly_movable_storable<I, I> &&
                     cleo::indirectly_swappable<I, I>;

// [mergeable]
template<
    typename I1,
    typename I2,
    typename Out,
    typename Comp = ranges::less,
    typename Proj1 = cleo::identity,
    typename Proj2 = cleo::identity>
concept mergeable =
    cleo::input_iterator<I1> && cleo::input_iterator<I2> && cleo::weakly_incrementable<Out> &&
    cleo::indirectly_copyable<I1, Out> && cleo::indirectly_copyable<I2, Out> &&
    cleo::indirect_strict_weak_order<Comp, cleo::projected<I1, Proj1>, cleo::projected<I2, Proj2>>;

// [sortable]
template<typename I, typename Comp = ranges::less, class Proj = cleo::identity>
concept sortable =
    cleo::permutable<I> && cleo::indirect_strict_weak_order<Comp, cleo::projected<I, Proj>>;

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
    iter_move(const reverse_iterator& i) noexcept(
        cleo::is_nothrow_copy_constructible_v<Iter> &&
        noexcept(cleo::ranges::iter_move(--cleo::declval<Iter&>()))
    ) {
        auto it = i.base();
        return cleo::ranges::iter_move(--it);
    }

    template<typename Iter2>
        requires cleo::indirectly_swappable<Iter, Iter2>
    friend constexpr void
    iter_swap(const reverse_iterator& x, const reverse_iterator<Iter2>& y) noexcept(
        cleo::is_nothrow_copy_constructible_v<Iter> &&
        cleo::is_nothrow_copy_constructible_v<Iter2> &&
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
