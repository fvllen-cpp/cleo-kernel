#pragma once

#include "cstddef.hpp"

namespace cleo {

// [meta.help] Helper classes

template<typename T, T v>
struct integral_constant {
    static constexpr T value = v;

    using value_type = T;
    using type = integral_constant<T, v>;

    constexpr operator value_type() const noexcept {
        return value;
    }

    constexpr value_type operator()() const noexcept {
        return value;
    }
};

using false_type = integral_constant<bool, false>;
using true_type = integral_constant<bool, true>;

// [meta.trans.cv], const-volatile modifications

// remove_cv
template<typename T>
struct remove_cv {
    using type = T;
};

template<typename T>
struct remove_cv<const T> {
    using type = T;
};

template<typename T>
struct remove_cv<volatile T> {
    using type = T;
};

template<typename T>
struct remove_cv<const volatile T> {
    using type = T;
};

template<typename T>
using remove_cv_t = typename remove_cv<T>::type;

// remove_const
template<typename T>
struct remove_const {
    using type = T;
};

template<typename T>
struct remove_const<const T> {
    using type = T;
};

template<typename T>
using remove_const_t = typename remove_const<T>::type;

// remove_volatile
template<typename T>
struct remove_volatile {
    using type = T;
};

template<typename T>
struct remove_volatile<volatile T> {
    using type = T;
};

template<typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

// [meta.trans.ref], reference modifications

// remove_reference
template<typename T>
struct remove_reference {
    using type = T;
};

template<typename T>
struct remove_reference<T&> {
    using type = T;
};

template<typename T>
struct remove_reference<T&&> {
    using type = T;
};

template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

// add_lvalue_reference
namespace detail {
template<typename T>
struct type_identity {
    using type = T;
};

template<typename T>
auto try_add_lvalue_reference(int) -> type_identity<T&>;

template<typename T>
auto try_add_lvalue_reference(...) -> type_identity<T>;
} // namespace detail

template<typename T>
struct add_lvalue_reference : decltype(detail::try_add_lvalue_reference<T>(0)) {};

template<typename T>
using add_lvalue_reference_t = add_lvalue_reference<T>::type;

// add_rvalue_reference
namespace detail {
template<typename T>
auto try_add_rvalue_reference(int) -> type_identity<T&&>;

template<typename T>
auto try_add_rvalue_reference(...) -> type_identity<T>;
} // namespace detail

template<typename T>
struct add_rvalue_reference : decltype(detail::try_add_rvalue_reference<T>(0)) {};

template<typename T>
using add_rvalue_reference_t = add_rvalue_reference<T>::type;

// [declval], declval
template<typename T>
add_rvalue_reference_t<T> declval() noexcept;

// [meta.rel], type relations

// is_same
template<typename T, typename U>
struct is_same : false_type {};

template<typename T>
struct is_same<T, T> : true_type {};

template<typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

// is_base_of
template<typename Base, typename Derived>
struct is_base_of : integral_constant<bool, __is_base_of(Base, Derived)> {};

template<typename Base, typename Derived>
constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

// is_virtual_base_of

// is_convertible
namespace detail {
template<typename T>
auto test_returnable(int) -> decltype(void(static_cast<T (*)()>(nullptr)), true_type{});

template<typename>
auto test_returnable(...) -> false_type;

template<typename From, typename To>
auto test_implicitly_convertible(int)
    -> decltype(void(cleo::declval<void (&)(To)>()(cleo::declval<From>())), true_type{});

template<typename, typename>
auto test_implicitly_convertible(...) -> false_type;
} // namespace detail

template<typename From, typename To>
struct is_convertible : integral_constant<
                            bool,
                            (decltype(detail::test_returnable<To>(0))::value
                             && decltype(detail::test_implicitly_convertible<From, To>(0))::value)
                                || (is_void<From>::value && is_void<To>::value)> {};

template<typename From, typename To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

// is_nothrow_convertible
template<typename From, typename To>
struct is_nothrow_convertible : cleo::conjuction<is_void<From>, is_void<To>> {};

template<typename From, class To>
    requires requires {
        static_cast<To (*)()>(nullptr);
        { cleo::declval<void (&)(To) noexcept>()(cleo::declval<From>()) } noexcept;
    }
struct is_nothrow_convertible<From, To> : true_type {};

template<typename From, typename To>
constexpr bool is_nothrow_convertible_v = is_nothrow_convertible<From, To>::value;

// is_layout_compatible
template<typename T, typename U>
constexpr bool is_layout_compatible_v = is_layout_compatible<T, U>::value;

// is_pointer_interconvertible_base_of
template<typename Base, typename Derived>
constexpr bool is_pointer_interconvertible_base_of_v =
    is_pointer_intercovertible_base_of<Base, Derived>::value;

// is_invocable
template<typename Fn, typename... ArgTypes>
constexpr bool is_invocable_v = is_invocable<Fn, ArgTypes...>::value;

// is_invocable_r
template<typename R, typename Fn, typename... ArgTypes>
constexpr bool is_invocable_r_v = is_invocable_r<R, Fn, ArgTypes...>::value;

// is_nothrow_invocable
template<typename Fn, typename... ArgTypes>
constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<Fn, ArgTypes...>::value;

// is_nothrow_invocable_r
template<typename R, typename Fn, typename... ArgTypes>
constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, Fn, ArgTypes...>::value;

// is_applicable
template<typename Fn, typename Tuple>
constexpr bool is_applicable_v = is_applicable<Fn, Tuple>::value;

// is_nothrow_applicable
template<typename Fn, typename Tuple>
constexpr bool is_nothrow_applicable_v = is_nothrow_applicable<Fn, Tuple>::value;

// [meta.unary.cat], Primary type categories

// is_void
template<typename T>
struct is_void : is_same<void, remove_cv_t<T>> {};

template<typename T>
constexpr bool is_void_v = is_void<T>::value;

// is_null_pointer
template<typename T>
struct is_null_pointer : is_same<cleo::nullptr_t, remove_cv_t<T>> {};

template<typename T>
constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

// is_pointer
template<typename T>
struct is_pointer : false_type {};

template<typename T>
struct is_pointer<T*> : true_type {};

template<typename T>
struct is_pointer<T* const> : true_type {};

template<typename T>
struct is_pointer<T* volatile> : true_type {};

template<typename T>
struct is_pointer<T* const volatile> : true_type {};

// is_lvalue_reference
template<typename T>
struct is_lvalue_reference_helper : false_type {};

template<typename T>
struct is_lvalue_reference_helper<T&> : true_type {};

template<typename T>
struct is_lvalue_reference : is_lvalue_reference_helper<remove_cv_t<T>> {};

template<typename T>
inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

// is_rvalue_reference
template<typename T>
struct is_rvalue_reference_helper : false_type {};

template<typename T>
struct is_rvalue_reference_helper<T&&> : true_type {};

template<typename T>
struct is_rvalue_reference : is_rvalue_reference_helper<remove_cv_t<T>> {};

template<typename T>
inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

// is_union
template<typename T>
struct is_union : integral_constant<bool, __is_union(T)> {};

template<typename T>
constexpr bool is_union_v = is_union<T>::value;

// is_class
namespace detail {
template<typename T>
integral_constant<bool, !is_union<T>::value> test(int T::*);

template<typename>
false_type test(...);
} // namespace detail

template<typename T>
struct is_class : decltype(detail::test<T>(nullptr)) {};

template<typename T>
constexpr bool is_class_v = is_class<T>::value;

// is_function

// [meta.unary.prop], Type properties
template<typename T>
struct is_const;

template<typename T>
struct is_volatile;

template<typename T>
struct is_trivially_copyable : integral_constant<bool, __is_trivially_copyable(T)> {};

template<typename T>
struct is_standard_layout;

template<typename T>
struct is_empty;

template<typename T>
struct is_polymorphic;

template<typename T>
struct is_abstract;

template<typename T>
struct is_final;

template<typename T>
struct is_aggregate;

template<typename T>
struct is_structural;

template<typename T>
struct is_signed;

template<typename T>
struct is_unsigned;

template<typename T>
struct is_bounded_array;

template<typename T>
struct is_unbounded_array;

template<typename T>
struct is_scoped_enum;

template<typename T, typename... Args>
struct is_constructible : integral_constant<bool, __is_constructible(T, Args...)> {};

template<typename T>
struct is_default_constructible;

template<typename T>
struct is_copy_constructable;

template<typename T>
struct is_move_constructable;

template<typename T, typename U>
struct is_assignable;

template<typename T>
struct is_copy_assignable;

template<typename T>
struct is_move_assignable;

template<typename T, typename U>
struct is_swappable_with;

template<typename T>
struct is_swappable;

template<typename T>
struct is_destructible;

template<typename T, typename... Args>
struct is_trivially_constructible
    : integral_constant<bool, __is_trivially_constructible(T, Args...)> {};

template<typename T>
struct is_trivially_default_constructible;

template<typename T>
struct is_trivially_copy_constructible;

template<typename T>
struct is_trivially_move_constructible;

template<typename T, typename U>
struct is_trivially_assignable;

template<typename T>
struct is_trivially_copy_assignable;

template<typename T>
struct is_trivially_move_assignable;

template<typename T>
struct is_trivially_destructible : integral_constant<bool, __is_trivially_destructible(T)> {};

template<typename T, typename... Args>
struct is_nothrow_constructible;

template<typename T>
struct is_nothrow_default_constructible;

template<typename T>
struct is_nothrow_copy_constructible;

template<typename T>
struct is_nothrow_move_constructible;

template<typename T, typename U>
struct is_nothrow_assignable;

template<typename T>
struct is_nothrow_copy_assignable;

template<typename T>
struct is_nothrow_move_assignable;

template<typename T, typename U>
struct is_nothrow_swappable_with;

template<typename T>
struct is_nothrow_swappable;

template<typename T>
struct is_nothrow_destructible;

template<typename T>
struct is_implicit_lifetime;

template<typename T>
struct has_virtual_destructor;

template<typename T>
struct has_unique_object_representations;

template<typename T, typename U>
struct reference_constructs_from_temporary;

template<typename T, typename U>
struct reference_converts_from_temporary;

} // namespace cleo
