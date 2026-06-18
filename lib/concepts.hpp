#pragma once

#include "functional.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

namespace cleo {

// [same_as]
namespace detail {
template<typename T, typename U>
concept SameHelper = cleo::is_same_v<T, U>;
} // namespace detail

template<typename T, typename U>
concept same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

// [derived_from]
template<typename Derived, typename Base>
concept derived_from = cleo::is_base_of_v<Base, Derived> &&
                       cleo::is_convertible_v<const volatile Derived*, const volatile Base*>;

// [convertible_to]
template<typename From, typename To>
concept convertible_to =
    cleo::is_convertible_v<From, To> && requires { static_cast<To>(cleo::declval<From>()); };

// [common_reference_with]
template<typename T, typename U>
concept common_reference_with =
    cleo::same_as<cleo::common_reference_t<T, U>, cleo::common_reference_t<U, T>> &&
    convertible_to<T, cleo::common_reference_t<T, U>> &&
    convertible_to<U, cleo::common_reference_t<T, U>>;

// [common_with]
template<typename T, typename U>
concept common_with = same_as<cleo::common_type_t<T, U>, cleo::common_type_t<U, T>> &&
                      requires {
                          static_cast<cleo::common_type_t<T, U>>(cleo::declval<T>());
                          static_cast<cleo::common_type_t<T, U>>(cleo::declval<U>());
                      } &&
                      cleo::common_reference_with<
                          cleo::add_lvalue_reference_t<const T>,
                          cleo::add_lvalue_reference_t<const U>> &&
                      cleo::common_reference_with<
                          cleo::add_lvalue_reference_t<cleo::common_type_t<T, U>>,
                          cleo::common_reference_t<
                              cleo::add_lvalue_reference_t<const T>,
                              cleo::add_lvalue_reference_t<const U>>>;

// [integral]
template<typename T>
concept integral = cleo::is_integral_v<T>;

// [signed_integral]
template<typename T>
concept signed_integral = integral<T> && cleo::is_signed_v<T>;

// [unsigned integral]
template<typename T>
concept unsigned_integral = integral<T> && !signed_integral<T>;

// [floating_point]
template<typename T>
concept floating_point = cleo::is_floating_point_v<T>;

// [destructible]
template<typename T>
concept destructible = cleo::is_nothrow_destructible_v<T>;

// [constructible_from]
template<typename T, typename... Args>
concept constructible_from = destructible<T> && cleo::is_constructible_v<T, Args...>;

// [default_initializable]
template<typename T>
concept default_initializable = constructible_from<T> && requires {
    T{};
    ::new T;
};

// [move_constructible]
template<typename T>
concept move_constructible = constructible_from<T, T> && convertible_to<T, T>;

// [copy_constructible]
template<typename T>
concept copy_constructible =
    move_constructible<T> && constructible_from<T, T&> && convertible_to<T&, T> &&
    constructible_from<T, const T&> && convertible_to<const T&, T> &&
    constructible_from<T, const T> && convertible_to<const T, T>;

// [assignable_from]
template<class LHS, class RHS>
concept assignable_from = cleo::is_lvalue_reference_v<LHS> &&
                          common_reference_with<
                              const cleo::remove_reference_t<LHS>&,
                              const cleo::remove_reference_t<RHS>&> &&
                          requires(LHS lhs, RHS&& rhs) {
                              { lhs = cleo::forward<RHS>(rhs) } -> same_as<LHS>;
                          };

// [concept.swappable]
namespace detail {
template<typename T>
concept __class_or_enum = cleo::is_class_v<T> || cleo::is_union_v<T> || cleo::is_enum_v<T>;
}

namespace ranges {
namespace __swap {
template<typename T>
void swap(T&, T&) = delete;

template<typename T, typename U>
concept __adl_swap = (cleo::detail::__class_or_enum<cleo::remove_reference_t<T>> ||
                      cleo::detail::__class_or_enum<cleo::remove_reference_t<U>>) &&
                     requires(T&& t, U&& u) { swap(static_cast<T&&>(t), static_cast<U&&>(u)); };

struct _Swap {
private:
    template<typename T, typename U>
    static constexpr bool _S_noexcept() {
        if constexpr (__adl_swap<T, U>) {
            return noexcept(swap(cleo::declval<T>(), cleo::declval<U>()));
        }
        else {
            return cleo::is_nothrow_move_constructible_v<cleo::remove_reference_t<T>> &&
                   cleo::is_nothrow_move_assignable_v<cleo::remove_reference_t<T>>;
        }
    }

public:
    template<typename T, typename U>
        requires __adl_swap<T, U> || (cleo::same_as<T, U> && cleo::is_lvalue_reference_v<T> &&
                                      cleo::move_constructible<cleo::remove_reference_t<T>> &&
                                      cleo::assignable_from<T, cleo::remove_reference_t<T>>)
    constexpr void operator()(T&& t, U&& u) const noexcept(_S_noexcept<T, U>()) {
        if constexpr (__adl_swap<T, U>) {
            swap(static_cast<T&&>(t), static_cast<U&&>(u));
        }
        else {
            auto tmp = static_cast<cleo::remove_reference_t<T>&&>(t);
            t = static_cast<cleo::remove_reference_t<T>&&>(u);
            u = static_cast<cleo::remove_reference_t<T>&&>(tmp);
        }
    }

    template<typename T, typename U, cleo::size_t N>
        requires requires(const _Swap& swap, T& a, U& b) { swap(a, b); }
    constexpr void operator()(T (&a)[N], U (&b)[N]) const noexcept(
        noexcept(cleo::declval<const _Swap&>()(cleo::declval<T&>(), cleo::declval<U&>()))
    ) {
        for (cleo::size_t i = 0; i < N; ++i) {
            (*this)(a[i], b[i]);
        }
    }
};

} // namespace __swap

inline namespace _Cpo {
inline constexpr __swap::_Swap swap{};
} // namespace _Cpo

} // namespace ranges

// [swappable]
template<typename T>
concept swappable = requires(T& a, T& b) { ranges::swap(a, b); };

template<typename T, typename U>
concept swappable_with = common_reference_with<T, U> && requires(T&& t, U&& u) {
    ranges::swap(cleo::forward<T>(t), cleo::forward<T>(t));
    ranges::swap(cleo::forward<U>(u), cleo::forward<U>(u));
    ranges::swap(cleo::forward<T>(t), cleo::forward<U>(u));
    ranges::swap(cleo::forward<U>(u), cleo::forward<T>(t));
};

// [movable]
template<typename T>
concept movable =
    cleo::is_object_v<T> && move_constructible<T> && assignable_from<T&, T> && swappable<T>;

// [copyable]
template<typename T>
concept copyable = copy_constructible<T> && movable<T> && assignable_from<T&, T&> &&
                   assignable_from<T&, const T&> && assignable_from<T&, const T>;

// boolean_testable - exposition only
namespace detail {
template<typename T>
concept boolean_testable_impl = cleo::convertible_to<T, bool>;

template<typename T>
concept boolean_testable = boolean_testable_impl<T> && requires(T&& t) {
    { !cleo::forward<T>(t) } -> boolean_testable_impl;
};
} // namespace detail

// [equality_comparable]
template<typename T, typename U>
concept __WeaklyEqualityComparableWith =
    requires(const cleo::remove_reference_t<T>& t, const cleo::remove_reference_t<U>& u) {
        { t == u } -> detail::boolean_testable;
        { t != u } -> detail::boolean_testable;
        { u == t } -> detail::boolean_testable;
        { u != t } -> detail::boolean_testable;
    };

template<typename T, typename U, typename C = common_reference_t<const T&, const U&>>
concept _ComparisonCommonTypeWithImpl =
    cleo::same_as<common_reference_t<const T&, const U&>, common_reference_t<const U&, const T&>> &&
    requires {
        requires convertible_to<const T&, const C&> || convertible_to<T, const C&>;
        requires convertible_to<const U&, const C&> || convertible_to<U, const C&>;
    };
template<typename T, typename U>
concept __ComparisonCommonTypeWith =
    _ComparisonCommonTypeWithImpl<cleo::remove_cvref_t<T>, cleo::remove_cvref_t<U>>;

template<typename T>
concept equality_comparable = __WeaklyEqualityComparableWith<T, T>;

template<typename T, typename U>
concept equality_comparable_with =
    equality_comparable<T> && equality_comparable<U> && __ComparisonCommonTypeWith<T, U> &&
    equality_comparable<common_reference_t<
        const cleo::remove_reference_t<T>&,
        const cleo::remove_reference_t<U>&>> &&
    __WeaklyEqualityComparableWith<T, U>;

// [totally_ordered]
template<typename T, typename U>
concept __PartiallyOrderedWith =
    requires(const cleo::remove_reference_t<T>& t, const cleo::remove_reference_t<U>& u) {
        { t < u } -> detail::boolean_testable;
        { t > u } -> detail::boolean_testable;
        { t <= u } -> detail::boolean_testable;
        { t >= u } -> detail::boolean_testable;
        { u < t } -> detail::boolean_testable;
        { u > t } -> detail::boolean_testable;
        { u <= t } -> detail::boolean_testable;
        { u >= t } -> detail::boolean_testable;
    };

template<typename T>
concept totally_ordered = equality_comparable<T> && __PartiallyOrderedWith<T, T>;

template<typename T, typename U>
concept totally_ordered_with =
    totally_ordered<T> && totally_ordered<U> && equality_comparable_with<T, U> &&
    totally_ordered<common_reference_t<
        const cleo::remove_reference_t<T>&,
        const cleo::remove_reference_t<U>&>> &&
    __PartiallyOrderedWith<T, U>;

// [semiregular]
template<typename T>
concept semiregular = copyable<T> && default_initializable<T>;

// [regular]
template<typename T>
concept regular = semiregular<T> && cleo::equality_comparable<T>;

// [invocable]
template<typename F, typename... Args>
concept invocable = requires(F&& f, Args&&... args) {
    cleo::invoke(cleo::forward<F>(f), cleo::forward<Args>(args)...);
};

// [regular_invocable]
template<typename F, typename... Args>
concept regular_invocable = invocable<F, Args...>;

// [predicate]
template<typename F, typename... Args>
concept predicate =
    regular_invocable<F, Args...> && detail::boolean_testable<invoke_result_t<F, Args...>>;

// [relation]
template<typename R, typename T, typename U>
concept relation =
    predicate<R, T, T> && predicate<R, U, U> && predicate<R, T, U> && predicate<R, U, T>;

// [equivalence_relation]
template<typename R, typename T, typename U>
concept equivalence_relation = relation<R, T, U>;

// [strict_weak_order]
template<typename R, typename T, typename U>
concept strict_weak_order = relation<R, T, U>;

} // namespace cleo
