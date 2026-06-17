#pragma once

#include "cstddef.hpp"

namespace cleo {

// forward declarations
template<typename T>
class reference_wrapper;

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

template<bool B>
using bool_constant = integral_constant<bool, B>;

using false_type = integral_constant<bool, false>;
using true_type = integral_constant<bool, true>;

namespace detail {
template<typename T>
struct type_identity {
    using type = T;
};
} // namespace detail

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

// add_const
template<typename T>
struct add_const {
    using type = const T;
};

template<typename T>
using add_const_t = typename add_const<T>::type;

// add_volatile
template<typename T>
struct add_volatile {
    using type = volatile T;
};

template<typename T>
using add_volatile_t = typename add_volatile<T>::type;

// add_cv
template<typename T>
struct add_cv {
    using type = const volatile T;
};

template<typename T>
using add_cv_t = typename add_cv<T>::type;

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
auto try_add_lvalue_reference(int) -> type_identity<T&>;

template<typename T>
auto try_add_lvalue_reference(...) -> type_identity<T>;
} // namespace detail

template<typename T>
struct add_lvalue_reference : decltype(detail::try_add_lvalue_reference<T>(0)) {};

template<typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

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
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

// [meta.trans.ptr], pointer transformations
// remove_pointer
template<typename T>
struct remove_pointer {
    using type = T;
};

template<typename T>
struct remove_pointer<T*> {
    using type = T;
};

template<typename T>
struct remove_pointer<T* const> {
    using type = T;
};

template<typename T>
struct remove_pointer<T* volatile> {
    using type = T;
};

template<typename T>
struct remove_pointer<T* const volatile> {
    using type = T;
};

template<typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

// add_pointer
namespace detail {
template<typename T>
auto try_add_pointer(int) -> type_identity<remove_reference_t<T>*>;

template<typename T>
auto try_add_pointer(...) -> type_identity<T>;
} // namespace detail

template<typename T>
struct add_pointer : decltype(detail::try_add_pointer<T>(0)) {};

template<typename T>
using add_pointer_t = typename add_pointer<T>::type;

// [declval], declval
template<typename T>
add_rvalue_reference_t<T> declval() noexcept;

// [meta.trans.arr], array transformations
template<typename T>
struct remove_extent {
    using type = T;
};

template<typename T>
struct remove_extent<T[]> {
    using type = T;
};

template<typename T, size_t N>
struct remove_extent<T[N]> {
    using type = T;
};

template<typename T>
using remove_extent_t = typename remove_extent<T>::type;

template<typename T>
struct remove_all_extents {
    using type = T;
};

template<typename T>
struct remove_all_extents<T[]> {
    using type = typename remove_all_extents<T>::type;
};

template<typename T, size_t N>
struct remove_all_extents<T[N]> {
    using type = typename remove_all_extents<T>::type;
};

template<typename T>
using remove_all_extents_t = typename remove_all_extents<T>::type;

// [meta.rel], type relations (no dependency on the unary categories)

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

// is_integral
namespace detail {
template<typename T>
struct is_integral_helper : false_type {};

template<>
struct is_integral_helper<bool> : true_type {};

template<>
struct is_integral_helper<char> : true_type {};

template<>
struct is_integral_helper<signed char> : true_type {};

template<>
struct is_integral_helper<unsigned char> : true_type {};

template<>
struct is_integral_helper<char8_t> : true_type {};

template<>
struct is_integral_helper<char16_t> : true_type {};

template<>
struct is_integral_helper<char32_t> : true_type {};

template<>
struct is_integral_helper<wchar_t> : true_type {};

template<>
struct is_integral_helper<short> : true_type {};

template<>
struct is_integral_helper<unsigned short> : true_type {};

template<>
struct is_integral_helper<int> : true_type {};

template<>
struct is_integral_helper<unsigned int> : true_type {};

template<>
struct is_integral_helper<long> : true_type {};

template<>
struct is_integral_helper<unsigned long> : true_type {};

template<>
struct is_integral_helper<long long> : true_type {};

template<>
struct is_integral_helper<unsigned long long> : true_type {};
} // namespace detail

template<typename T>
struct is_integral : detail::is_integral_helper<remove_cv_t<T>> {};

template<typename T>
constexpr bool is_integral_v = is_integral<T>::value;

// is_floating_point
template<typename T>
struct is_floating_point : integral_constant<
                               bool,
                               is_same_v<float, remove_cv_t<T>> || is_same_v<double, remove_cv_t<T>>
                                   || is_same_v<long double, remove_cv_t<T>>> {};

template<typename T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;

// is_array
template<typename T>
struct is_array : false_type {};

template<typename T>
struct is_array<T[]> : true_type {};

template<typename T, size_t N>
struct is_array<T[N]> : true_type {};

template<typename T>
constexpr bool is_array_v = is_array<T>::value;

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

template<typename T>
constexpr bool is_pointer_v = is_pointer<T>::value;

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

// is_enum
template<typename T>
struct is_enum : integral_constant<bool, __is_enum(T)> {};

template<typename T>
constexpr bool is_enum_v = is_enum<T>::value;

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
template<typename T>
struct is_function : integral_constant<bool, __is_function(T)> {};

template<typename T>
constexpr bool is_function_v = is_function<T>::value;

// is_member_function_pointer (primitive: a pointer-to-member whose member is a function)
template<typename T>
struct is_member_function_pointer_helper : false_type {};

template<typename T, typename U>
struct is_member_function_pointer_helper<T U::*> : is_function<T> {};

template<typename T>
struct is_member_function_pointer : is_member_function_pointer_helper<remove_cv_t<T>> {};

template<typename T>
constexpr bool is_member_function_pointer_v = is_member_function_pointer<T>::value;

// is_member_object_pointer
template<typename T>
struct is_member_object_pointer_helper : false_type {};

template<typename T, typename U>
struct is_member_object_pointer_helper<T U::*> : bool_constant<!is_function_v<T>> {};

template<typename T>
struct is_member_object_pointer : is_member_object_pointer_helper<remove_cv_t<T>> {};

template<typename T>
constexpr bool is_member_object_pointer_v = is_member_object_pointer<T>::value;

// [meta.unary.comp], Composite type categories

// is_reference
template<typename T>
struct is_reference : false_type {};

template<typename T>
struct is_reference<T&> : true_type {};

template<typename T>
struct is_reference<T&&> : true_type {};

template<typename T>
constexpr bool is_reference_v = is_reference<T>::value;

// is_member_pointer
template<typename T>
struct is_member_pointer_helper : false_type {};

template<typename T, typename U>
struct is_member_pointer_helper<T U::*> : true_type {};

template<typename T>
struct is_member_pointer : is_member_pointer_helper<remove_cv_t<T>> {};

template<typename T>
constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

// is_arithmetic
template<typename T>
struct is_arithmetic
    : integral_constant<bool, is_integral<T>::value || is_floating_point<T>::value> {};

template<typename T>
constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

// is_fundamental
template<typename T>
struct is_fundamental : integral_constant<
                            bool,
                            is_arithmetic<T>::value || is_void<T>::value
                                || is_same<nullptr_t, remove_cv_t<T>>::value> {};

template<typename T>
constexpr bool is_fundamental_v = is_fundamental<T>::value;

// is_scalar
template<typename T>
struct is_scalar : integral_constant<
                       bool,
                       is_arithmetic<T>::value || is_enum<T>::value || is_pointer<T>::value
                           || is_member_pointer<T>::value || is_null_pointer<T>::value> {};

template<typename T>
constexpr bool is_scalar_v = is_scalar<T>::value;

// is_object
template<typename T>
struct is_object
    : integral_constant<
          bool,
          is_scalar<T>::value || is_array<T>::value || is_union<T>::value || is_class<T>::value> {};

template<typename T>
constexpr bool is_object_v = is_object<T>::value;

// is_compound
template<typename T>
struct is_compound : integral_constant<bool, !is_fundamental<T>::value> {};

template<typename T>
constexpr bool is_compound_v = is_compound<T>::value;

// [meta.rel], type relations that depend on the unary categories

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
struct is_nothrow_convertible : bool_constant<is_void_v<From> && is_void_v<To>> {};

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
struct is_layout_compatible : integral_constant<bool, __is_layout_compatible(T, U)> {};

template<typename T, typename U>
constexpr bool is_layout_compatible_v = is_layout_compatible<T, U>::value;

// is_pointer_interconvertible_base_of
template<typename Base, typename Derived>
struct is_pointer_interconvertible_base_of
    : integral_constant<bool, __is_pointer_interconvertible_base_of(Base, Derived)> {};

template<typename Base, typename Derived>
constexpr bool is_pointer_interconvertible_base_of_v =
    is_pointer_interconvertible_base_of<Base, Derived>::value;

// [meta.unary.prop], Type properties

// is_const
template<typename T>
struct is_const : false_type {};

template<typename T>
struct is_const<const T> : true_type {};

template<typename T>
constexpr bool is_const_v = is_const<T>::value;

// is_volatile
template<typename T>
struct is_volatile : false_type {};

template<typename T>
struct is_volatile<volatile T> : true_type {};

template<typename T>
constexpr bool is_volatile_v = is_volatile<T>::value;

// is_trivially_copyable
template<typename T>
struct is_trivially_copyable : integral_constant<bool, __is_trivially_copyable(T)> {};

template<typename T>
constexpr bool is_trivially_copyable_v = is_trivially_copyable<T>::value;

// is_standard_layout
template<typename T>
struct is_standard_layout : integral_constant<bool, __is_standard_layout(T)> {};

template<typename T>
constexpr bool is_standard_layout_v = is_standard_layout<T>::value;

// is_empty
template<typename T>
struct is_empty : integral_constant<bool, __is_empty(T)> {};

template<typename T>
constexpr bool is_empty_v = is_empty<T>::value;

// is_polymorphic
template<typename T>
struct is_polymorphic : integral_constant<bool, __is_polymorphic(T)> {};

template<typename T>
constexpr bool is_polymorphic_v = is_polymorphic<T>::value;

// is_abstract
template<typename T>
struct is_abstract : integral_constant<bool, __is_abstract(T)> {};

template<typename T>
constexpr bool is_abstract_v = is_abstract<T>::value;

// is_final
template<typename T>
struct is_final : integral_constant<bool, __is_final(T)> {};

template<typename T>
constexpr bool is_final_v = is_final<T>::value;

// is_aggregate
template<typename T>
struct is_aggregate : integral_constant<bool, __is_aggregate(T)> {};

template<typename T>
constexpr bool is_aggregate_v = is_aggregate<T>::value;

// is_signed
namespace detail {
template<typename T, bool = is_arithmetic<T>::value>
struct is_signed : integral_constant<bool, T(-1) < T(0)> {};

template<typename T>
struct is_signed<T, false> : false_type {};
} // namespace detail

template<typename T>
struct is_signed : detail::is_signed<T>::type {};

template<typename T>
constexpr bool is_signed_v = is_signed<T>::value;

// is_unsigned
namespace detail {
template<typename T, bool = is_arithmetic_v<T>>
struct is_unsigned : integral_constant<bool, T(0) < T(-1)> {};

template<typename T>
struct is_unsigned<T, false> : false_type {};
} // namespace detail

template<typename T>
struct is_unsigned : detail::is_unsigned<T>::type {};

template<typename T>
constexpr bool is_unsigned_v = is_unsigned<T>::value;

// is_trivial
template<typename T>
struct is_trivial : integral_constant<bool, __is_trivial(T)> {};

template<typename T>
constexpr bool is_trivial_v = is_trivial<T>::value;

// is_bounded_array
template<typename T>
struct is_bounded_array : false_type {};

template<typename T, size_t N>
struct is_bounded_array<T[N]> : true_type {};

template<typename T>
constexpr bool is_bounded_array_v = is_bounded_array<T>::value;

// is_unbounded_array
template<typename T>
struct is_unbounded_array : false_type {};

template<typename T>
struct is_unbounded_array<T[]> : true_type {};

template<typename T>
constexpr bool is_unbounded_array_v = is_unbounded_array<T>::value;

// is_scoped_enum
namespace detail {
void test_conversion(...);          // selected when E is complete and scoped
void test_conversion(int) = delete; // selected when E is complete and unscoped

template<typename E>
concept is_scoped_enum_impl =
    is_enum_v<E> &&                             // checked first
    requires { detail::test_conversion(E{}); }; // ill-formed before overload resolution
                                                // when E is incomplete
} // namespace detail

template<typename T>
struct is_scoped_enum : bool_constant<detail::is_scoped_enum_impl<T>> {};

template<typename T>
constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// is_constructible
template<typename T, typename... Args>
struct is_constructible : integral_constant<bool, __is_constructible(T, Args...)> {};

template<typename T, typename... Args>
inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

// is_default_constructible
template<typename T>
struct is_default_constructible : is_constructible<T> {};

template<typename T>
inline constexpr bool is_default_constructible_v = is_default_constructible<T>::value;

// is_copy_constructible
template<typename T>
struct is_copy_constructible : is_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};

template<typename T>
inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

// is_move_constructible
template<typename T>
struct is_move_constructible : is_constructible<T, add_rvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

// is_assignable
template<typename T, typename U>
struct is_assignable : integral_constant<bool, __is_assignable(T, U)> {};

template<typename T, typename U>
constexpr bool is_assignable_v = is_assignable<T, U>::value;

// is_copy_assignable
template<typename T>
struct is_copy_assignable
    : is_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>> {};

template<typename T>
inline constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

// is_move_assignable
template<typename T>
struct is_move_assignable : is_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

// is_destructible
template<typename T>
struct is_destructible : integral_constant<bool, __is_destructible(T)> {};

template<typename T>
constexpr bool is_destructible_v = is_destructible<T>::value;

// is_trivially_constructible
template<typename T, typename... Args>
struct is_trivially_constructible
    : integral_constant<bool, __is_trivially_constructible(T, Args...)> {};

template<typename T, typename... Args>
inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T, Args...>::value;

// is_trivially_default_constructible
template<typename T>
struct is_trivially_default_constructible : is_trivially_constructible<T> {};

template<typename T>
inline constexpr bool is_trivially_default_constructible_v =
    is_trivially_default_constructible<T>::value;

// is_trivially_copy_constructible
template<typename T>
struct is_trivially_copy_constructible
    : is_trivially_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};

template<typename T>
inline constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<T>::value;

// is_trivially_move_constructible
template<typename T>
struct is_trivially_move_constructible : is_trivially_constructible<T, add_rvalue_reference_t<T>> {
};

template<typename T>
inline constexpr bool is_trivially_move_constructible_v = is_trivially_move_constructible<T>::value;

// is_trivially_assignable
template<typename T, typename U>
struct is_trivially_assignable : integral_constant<bool, __is_trivially_assignable(T, U)> {};

template<typename T, typename U>
constexpr bool is_trivially_assignable_v = is_trivially_assignable<T, U>::value;

// is_trivially_copy_assignable
template<typename T>
struct is_trivially_copy_assignable
    : is_trivially_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>> {};

template<typename T>
inline constexpr bool is_trivially_copy_assignable_v = is_trivially_copy_assignable<T>::value;

// is_trivially_move_assignable
template<typename T>
struct is_trivially_move_assignable
    : is_trivially_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_trivially_move_assignable_v = is_trivially_move_assignable<T>::value;

// is_trivially_destructible
template<typename T>
struct is_trivially_destructible : integral_constant<bool, __is_trivially_destructible(T)> {};

template<typename T>
constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;

// is_nothrow_constructible
template<typename T, typename... Args>
struct is_nothrow_constructible : integral_constant<bool, __is_nothrow_constructible(T, Args...)> {
};

template<typename T, typename... Args>
inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Args...>::value;

// is_nothrow_default_constructible
template<typename T>
struct is_nothrow_default_constructible : is_nothrow_constructible<T> {};

template<typename T>
inline constexpr bool is_nothrow_default_constructible_v =
    is_nothrow_default_constructible<T>::value;

// is_nothrow_copy_constructible
template<typename T>
struct is_nothrow_copy_constructible
    : is_nothrow_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};

template<typename T>
inline constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<T>::value;

// is_nothrow_move_constructible
template<typename T>
struct is_nothrow_move_constructible : is_nothrow_constructible<T, add_rvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;

// is_nothrow_assignable
template<typename T, typename U>
struct is_nothrow_assignable : integral_constant<bool, __is_nothrow_assignable(T, U)> {};

template<typename T, typename U>
constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<T, U>::value;

// is_nothrow_copy_assignable
template<typename T>
struct is_nothrow_copy_assignable
    : is_nothrow_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>> {};

template<typename T>
inline constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<T>::value;

// is_nothrow_move_assignable
template<typename T>
struct is_nothrow_move_assignable
    : is_nothrow_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>> {};

template<typename T>
inline constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;

// is_nothrow_destructible
template<typename T>
struct is_nothrow_destructible : integral_constant<bool, __is_nothrow_destructible(T)> {};

template<typename T>
constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<T>::value;

// is_implicit_lifetime (C++23; intrinsic only on newer clang)
#if __has_builtin(__builtin_is_implicit_lifetime)
template<typename T>
struct is_implicit_lifetime : integral_constant<bool, __builtin_is_implicit_lifetime(T)> {};

template<typename T>
constexpr bool is_implicit_lifetime_v = is_implicit_lifetime<T>::value;
#endif

// has_virtual_destructor
template<typename T>
struct has_virtual_destructor : integral_constant<bool, __has_virtual_destructor(T)> {};

template<typename T>
constexpr bool has_virtual_destructor_v = has_virtual_destructor<T>::value;

// has_unique_object_representations
template<typename T>
struct has_unique_object_representations
    : integral_constant<bool, __has_unique_object_representations(T)> {};

template<typename T>
constexpr bool has_unique_object_representations_v = has_unique_object_representations<T>::value;

// [meta.unary.prop.query], Type property queries
// alignment_of
template<typename T>
struct alignment_of : integral_constant<cleo::size_t, alignof(T)> {};

template<typename T>
constexpr cleo::size_t alignment_of_v = alignment_of<T>::value;

// rank
template<typename T>
struct rank : public integral_constant<cleo::size_t, 0> {};

template<typename T>
struct rank<T[]> : public integral_constant<cleo::size_t, rank<T>::value + 1> {};

template<typename T, cleo::size_t N>
struct rank<T[N]> : public integral_constant<cleo::size_t, rank<T>::value + 1> {};

template<typename T>
constexpr cleo::size_t rank_v = rank<T>::value;

// extent
template<typename T, unsigned N = 0>
struct extent : integral_constant<cleo::size_t, 0> {};

template<typename T>
struct extent<T[], 0> : integral_constant<cleo::size_t, 0> {};

template<typename T, unsigned N>
struct extent<T[], N> : extent<T, N - 1> {};

template<typename T, cleo::size_t I>
struct extent<T[I], 0> : integral_constant<cleo::size_t, I> {};

template<typename T, cleo::size_t I, unsigned N>
struct extent<T[I], N> : extent<T, N - 1> {};

template<typename T, unsigned N = 0>
constexpr cleo::size_t extent_v = extent<T, N>::value;

// [meta.rel], type relations that depend on the unary categories

// reference_constructs_from_temporary
template<typename T, typename U>
struct reference_constructs_from_temporary
    : integral_constant<bool, __reference_constructs_from_temporary(T, U)> {};

template<typename T, typename U>
inline constexpr bool reference_constructs_from_temporary_v =
    reference_constructs_from_temporary<T, U>::value;

// reference_converts_from_temporary
template<typename T, typename U>
struct reference_converts_from_temporary
    : integral_constant<bool, __reference_converts_from_temporary(T, U)> {};

template<typename T, typename U>
inline constexpr bool reference_converts_from_temporary_v =
    reference_converts_from_temporary<T, U>::value;

// [meta.trans.sign]
namespace detail {
template<
    cleo::size_t Size,
    bool = (Size == sizeof(signed char)),
    bool = (Size == sizeof(signed short)),
    bool = (Size == sizeof(signed int)),
    bool = (Size == sizeof(signed long)),
    bool = (Size == sizeof(signed long long))>
struct signed_integer_by_size {};

template<cleo::size_t Size, bool Short, bool Int, bool Long, bool LongLong>
struct signed_integer_by_size<Size, true, Short, Int, Long, LongLong> {
    using type = signed char;
};

template<cleo::size_t Size, bool Int, bool Long, bool LongLong>
struct signed_integer_by_size<Size, false, true, Int, Long, LongLong> {
    using type = signed short;
};

template<cleo::size_t Size, bool Long, bool LongLong>
struct signed_integer_by_size<Size, false, false, true, Long, LongLong> {
    using type = signed int;
};

template<cleo::size_t Size, bool LongLong>
struct signed_integer_by_size<Size, false, false, false, true, LongLong> {
    using type = signed long;
};

template<cleo::size_t Size>
struct signed_integer_by_size<Size, false, false, false, false, true> {
    using type = signed long long;
};

template<
    cleo::size_t Size,
    bool = (Size == sizeof(unsigned char)),
    bool = (Size == sizeof(unsigned short)),
    bool = (Size == sizeof(unsigned int)),
    bool = (Size == sizeof(unsigned long)),
    bool = (Size == sizeof(unsigned long long))>
struct unsigned_integer_by_size {};

template<cleo::size_t Size, bool Short, bool Int, bool Long, bool LongLong>
struct unsigned_integer_by_size<Size, true, Short, Int, Long, LongLong> {
    using type = unsigned char;
};

template<cleo::size_t Size, bool Int, bool Long, bool LongLong>
struct unsigned_integer_by_size<Size, false, true, Int, Long, LongLong> {
    using type = unsigned short;
};

template<cleo::size_t Size, bool Long, bool LongLong>
struct unsigned_integer_by_size<Size, false, false, true, Long, LongLong> {
    using type = unsigned int;
};

template<cleo::size_t Size, bool LongLong>
struct unsigned_integer_by_size<Size, false, false, false, true, LongLong> {
    using type = unsigned long;
};

template<cleo::size_t Size>
struct unsigned_integer_by_size<Size, false, false, false, false, true> {
    using type = unsigned long long;
};

template<typename From, typename To>
struct sign_copy_cv {
    using type = To;
};

template<typename From, typename To>
struct sign_copy_cv<const From, To> {
    using type = const To;
};

template<typename From, typename To>
struct sign_copy_cv<volatile From, To> {
    using type = volatile To;
};

template<typename From, typename To>
struct sign_copy_cv<const volatile From, To> {
    using type = const volatile To;
};

template<typename T>
struct make_signed_raw {};

template<>
struct make_signed_raw<char> : signed_integer_by_size<sizeof(char)> {};

template<>
struct make_signed_raw<signed char> {
    using type = signed char;
};

template<>
struct make_signed_raw<unsigned char> {
    using type = signed char;
};

template<>
struct make_signed_raw<wchar_t> : signed_integer_by_size<sizeof(wchar_t)> {};

template<>
struct make_signed_raw<char8_t> : signed_integer_by_size<sizeof(char8_t)> {};

template<>
struct make_signed_raw<char16_t> : signed_integer_by_size<sizeof(char16_t)> {};

template<>
struct make_signed_raw<char32_t> : signed_integer_by_size<sizeof(char32_t)> {};

template<>
struct make_signed_raw<signed short> {
    using type = signed short;
};

template<>
struct make_signed_raw<unsigned short> {
    using type = signed short;
};

template<>
struct make_signed_raw<signed int> {
    using type = signed int;
};

template<>
struct make_signed_raw<unsigned int> {
    using type = signed int;
};

template<>
struct make_signed_raw<signed long> {
    using type = signed long;
};

template<>
struct make_signed_raw<unsigned long> {
    using type = signed long;
};

template<>
struct make_signed_raw<signed long long> {
    using type = signed long long;
};

template<>
struct make_signed_raw<unsigned long long> {
    using type = signed long long;
};

template<typename T>
struct make_unsigned_raw {};

template<>
struct make_unsigned_raw<char> : unsigned_integer_by_size<sizeof(char)> {};

template<>
struct make_unsigned_raw<signed char> {
    using type = unsigned char;
};

template<>
struct make_unsigned_raw<unsigned char> {
    using type = unsigned char;
};

template<>
struct make_unsigned_raw<wchar_t> : unsigned_integer_by_size<sizeof(wchar_t)> {};

template<>
struct make_unsigned_raw<char8_t> : unsigned_integer_by_size<sizeof(char8_t)> {};

template<>
struct make_unsigned_raw<char16_t> : unsigned_integer_by_size<sizeof(char16_t)> {};

template<>
struct make_unsigned_raw<char32_t> : unsigned_integer_by_size<sizeof(char32_t)> {};

template<>
struct make_unsigned_raw<signed short> {
    using type = unsigned short;
};

template<>
struct make_unsigned_raw<unsigned short> {
    using type = unsigned short;
};

template<>
struct make_unsigned_raw<signed int> {
    using type = unsigned int;
};

template<>
struct make_unsigned_raw<unsigned int> {
    using type = unsigned int;
};

template<>
struct make_unsigned_raw<signed long> {
    using type = unsigned long;
};

template<>
struct make_unsigned_raw<unsigned long> {
    using type = unsigned long;
};

template<>
struct make_unsigned_raw<signed long long> {
    using type = unsigned long long;
};

template<>
struct make_unsigned_raw<unsigned long long> {
    using type = unsigned long long;
};

template<typename T, bool = is_same_v<remove_cv_t<T>, bool>>
struct make_signed_integral {};

template<typename T>
struct make_signed_integral<T, false> {
    using type = typename sign_copy_cv<T, typename make_signed_raw<remove_cv_t<T>>::type>::type;
};

template<typename T, bool = is_same_v<remove_cv_t<T>, bool>>
struct make_unsigned_integral {};

template<typename T>
struct make_unsigned_integral<T, false> {
    using type = typename sign_copy_cv<T, typename make_unsigned_raw<remove_cv_t<T>>::type>::type;
};

template<typename T, bool = is_integral_v<remove_cv_t<T>>, bool = is_enum_v<remove_cv_t<T>>>
struct make_signed_selector {};

template<typename T>
struct make_signed_selector<T, true, false> : make_signed_integral<T> {};

template<typename T>
struct make_signed_selector<T, false, true> {
    using type =
        typename sign_copy_cv<T, typename signed_integer_by_size<sizeof(remove_cv_t<T>)>::type>::
            type;
};

template<typename T, bool = is_integral_v<remove_cv_t<T>>, bool = is_enum_v<remove_cv_t<T>>>
struct make_unsigned_selector {};

template<typename T>
struct make_unsigned_selector<T, true, false> : make_unsigned_integral<T> {};

template<typename T>
struct make_unsigned_selector<T, false, true> {
    using type =
        typename sign_copy_cv<T, typename unsigned_integer_by_size<sizeof(remove_cv_t<T>)>::type>::
            type;
};
} // namespace detail

template<typename T>
struct make_signed : detail::make_signed_selector<T> {};

template<typename T>
using make_signed_t = typename make_signed<T>::type;

template<typename T>
struct make_unsigned : detail::make_unsigned_selector<T> {};

template<typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

// [meta.trans.other], Other transformations

// enable_if
template<bool B, typename T = void>
struct enable_if {};

template<typename T>
struct enable_if<true, T> {
    using type = T;
};

template<bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

// underlying_type
template<typename T, bool = is_enum_v<T>>
struct underlying_type {};

template<typename T>
struct underlying_type<T, true> {
    using type = __underlying_type(T);
};

template<typename T>
using underlying_type_t = typename underlying_type<T>::type;

// type_identity
template<typename T>
struct type_identity {
    using type = T;
};

template<typename T>
using type_identity_t = typename type_identity<T>::type;

// conditional
template<bool B, typename T, typename F>
struct conditional {
    using type = T;
};

template<typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};

template<bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

// void_t
template<typename...>
using void_t = void;

// remove_cvref
template<typename T>
struct remove_cvref {
    using type = remove_cv_t<remove_reference_t<T>>;
};

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

// decay
template<typename T>
struct decay {
private:
    using U = remove_reference_t<T>;

public:
    using type = conditional_t<
        is_array_v<U>,
        add_pointer_t<remove_extent_t<U>>,
        conditional_t<is_function_v<U>, add_pointer_t<U>, remove_cv_t<U>>>;
};

template<typename T>
using decay_t = typename decay<T>::type;

// common_type
template<typename... T>
struct common_type {};

template<typename T>
struct common_type<T> : common_type<T, T> {};

namespace detail {
template<typename T1, typename T2>
using conditional_result_t = decltype(false ? cleo::declval<T1>() : cleo::declval<T2>());

template<typename, typename, typename = void>
struct decay_conditional_result {};

template<typename T1, typename T2>
struct decay_conditional_result<T1, T2, void_t<conditional_result_t<T1, T2>>>
    : decay<conditional_result_t<T1, T2>> {};

template<typename T1, typename T2, typename = void>
struct common_type_2_impl : decay_conditional_result<const T1&, const T2&> {};

template<typename T1, typename T2>
struct common_type_2_impl<T1, T2, void_t<conditional_result_t<T1, T2>>>
    : decay_conditional_result<T1, T2> {};
} // namespace detail

template<typename T1, typename T2>
struct common_type<T1, T2> : conditional_t<
                                 is_same_v<T1, decay_t<T1>> && is_same_v<T2, decay_t<T2>>,
                                 detail::common_type_2_impl<T1, T2>,
                                 common_type<decay_t<T1>, decay_t<T2>>> {};

namespace detail {
template<typename AlwaysVoid, typename T1, typename T2, typename... R>
struct common_type_multi_impl {};

template<typename T1, typename T2, typename... R>
struct common_type_multi_impl<void_t<typename common_type<T1, T2>::type>, T1, T2, R...>
    : common_type<typename common_type<T1, T2>::type, R...> {};
} // namespace detail

template<typename T1, typename T2, typename... R>
struct common_type<T1, T2, R...> : detail::common_type_multi_impl<void, T1, T2, R...> {};

template<typename... T>
using common_type_t = typename common_type<T...>::type;

template<
    typename T,
    typename U,
    template<typename> typename TQual,
    template<typename> typename UQual>
struct basic_common_reference {};

// common_reference
template<typename... T>
struct common_reference {};

template<typename... T>
using common_reference_t = typename common_reference<T...>::type;

template<>
struct common_reference<> {};

template<typename T>
struct common_reference<T> {
    using type = T;
};

namespace detail {
template<typename From, typename To>
struct copy_cv {
    using type = To;
};

template<typename From, typename To>
struct copy_cv<const From, To> {
    using type = const To;
};

template<typename From, typename To>
struct copy_cv<volatile From, To> {
    using type = volatile To;
};

template<typename From, typename To>
struct copy_cv<const volatile From, To> {
    using type = const volatile To;
};

template<typename From, typename To>
using copy_cv_t = typename copy_cv<From, To>::type;

template<typename From, typename To>
struct copy_cvref {
    using type = copy_cv_t<From, To>;
};

template<typename From, typename To>
struct copy_cvref<From&, To> {
    using type = add_lvalue_reference_t<copy_cv_t<From, To>>;
};

template<typename From, typename To>
struct copy_cvref<From&&, To> {
    using type = add_rvalue_reference_t<copy_cv_t<From, To>>;
};

template<typename From, typename To>
using copy_cvref_t = typename copy_cvref<From, To>::type;

template<typename From>
struct xref {
    template<typename To>
    using apply = copy_cvref_t<From, To>;
};

template<typename X, typename Y>
using common_ref_lvalue_t =
    decltype(false ? cleo::declval<copy_cv_t<X, Y>&>() : cleo::declval<copy_cv_t<Y, X>&>());

template<typename T, typename U, typename = void>
struct common_ref {};

template<typename X, typename Y, bool>
struct common_ref_lvalue_select {};

template<typename X, typename Y>
struct common_ref_lvalue_select<X, Y, true> {
    using type = common_ref_lvalue_t<X, Y>;
};

template<typename X, typename Y>
struct common_ref<X&, Y&, void_t<common_ref_lvalue_t<X, Y>>>
    : common_ref_lvalue_select<X, Y, is_reference_v<common_ref_lvalue_t<X, Y>>> {};

template<typename X, typename Y>
using common_ref_rvalue_t =
    add_rvalue_reference_t<remove_reference_t<typename common_ref<X&, Y&>::type>>;

template<typename X, typename Y, bool>
struct common_ref_rvalue_select {};

template<typename X, typename Y>
struct common_ref_rvalue_select<X, Y, true> {
    using type = common_ref_rvalue_t<X, Y>;
};

template<typename X, typename Y>
struct common_ref<X&&, Y&&, void_t<common_ref_rvalue_t<X, Y>>>
    : common_ref_rvalue_select<
          X,
          Y,
          is_convertible_v<X&&, common_ref_rvalue_t<X, Y>>
              && is_convertible_v<Y&&, common_ref_rvalue_t<X, Y>>> {};

template<typename X, typename Y>
using common_ref_rvalue_lvalue_t = typename common_ref<const X&, Y&>::type;

template<typename X, typename Y, bool>
struct common_ref_rvalue_lvalue_select {};

template<typename X, typename Y>
struct common_ref_rvalue_lvalue_select<X, Y, true> {
    using type = common_ref_rvalue_lvalue_t<X, Y>;
};

template<typename X, typename Y>
struct common_ref<X&&, Y&, void_t<common_ref_rvalue_lvalue_t<X, Y>>>
    : common_ref_rvalue_lvalue_select<
          X,
          Y,
          is_convertible_v<X&&, common_ref_rvalue_lvalue_t<X, Y>>> {};

template<typename X, typename Y>
struct common_ref<X&, Y&&> : common_ref<Y&&, X&> {};

template<typename T, typename U>
using cond_res_t = decltype(false ? cleo::declval<T>() : cleo::declval<U>());

template<typename T, typename U, typename = void>
struct common_reference_step4 {};

template<typename T, typename U>
struct common_reference_step4<T, U, void_t<common_type_t<T, U>>> {
    using type = common_type_t<T, U>;
};

template<typename T, typename U, typename = void>
struct common_reference_step3 : common_reference_step4<T, U> {};

template<typename T, typename U>
struct common_reference_step3<T, U, void_t<cond_res_t<T, U>>> {
    using type = cond_res_t<T, U>;
};

template<typename T, typename U, typename = void>
struct common_reference_step2 : common_reference_step3<T, U> {};

template<typename T, typename U>
struct common_reference_step2<
    T,
    U,
    void_t<typename basic_common_reference<
        remove_cvref_t<T>,
        remove_cvref_t<U>,
        xref<T>::template apply,
        xref<U>::template apply>::type>> {
    using type = typename basic_common_reference<
        remove_cvref_t<T>,
        remove_cvref_t<U>,
        xref<T>::template apply,
        xref<U>::template apply>::type;
};

template<typename T, typename U, typename = void>
struct common_reference_step1 : common_reference_step2<T, U> {};

template<typename T, typename U>
struct common_reference_step1<T, U, void_t<typename common_ref<T, U>::type>> {
    using type = typename common_ref<T, U>::type;
};
} // namespace detail

template<typename T, typename U>
struct common_reference<T, U> : detail::common_reference_step1<T, U> {};

template<typename T, typename U, typename... Rest>
struct common_reference<T, U, Rest...>
    : common_reference<typename common_reference<T, U>::type, Rest...> {};

// member_pointer_class + INVOKE machinery - realizes [func.require] INVOKE
namespace detail {
template<typename T>
struct member_pointer_class;

template<typename M, typename C>
struct member_pointer_class<M C::*> {
    using type = C;
};

template<typename T>
using member_pointer_class_t = typename member_pointer_class<remove_cvref_t<T>>::type;

// pmf, (derived) object
template<typename F, typename O, typename... A>
    requires is_member_function_pointer_v<remove_cvref_t<F>>
             && is_base_of_v<member_pointer_class_t<F>, remove_cvref_t<O>>
constexpr auto invoke_impl(F&& f, O&& o, A&&... a) noexcept(
    noexcept((static_cast<O&&>(o).*f)(static_cast<A&&>(a)...))
) -> decltype((static_cast<O&&>(o).*f)(static_cast<A&&>(a)...)) {
    return (static_cast<O&&>(o).*f)(static_cast<A&&>(a)...);
}

// pmf, pointer-like object
template<typename F, typename O, typename... A>
    requires is_member_function_pointer_v<remove_cvref_t<F>>
             && (!is_base_of_v<member_pointer_class_t<F>, remove_cvref_t<O>>)
constexpr auto invoke_impl(F&& f, O&& o, A&&... a) noexcept(
    noexcept(((*static_cast<O&&>(o)).*f)(static_cast<A&&>(a)...))
) -> decltype(((*static_cast<O&&>(o)).*f)(static_cast<A&&>(a)...)) {
    return ((*static_cast<O&&>(o)).*f)(static_cast<A&&>(a)...);
}

// pmd, (derived) object
template<typename F, typename O>
    requires is_member_object_pointer_v<remove_cvref_t<F>>
             && is_base_of_v<member_pointer_class_t<F>, remove_cvref_t<O>>
constexpr auto invoke_impl(F&& f, O&& o) noexcept(noexcept(static_cast<O&&>(o).*f))
    -> decltype(static_cast<O&&>(o).*f) {
    return static_cast<O&&>(o).*f;
}

// pmd, pointer-like object
template<typename F, typename O>
    requires is_member_object_pointer_v<remove_cvref_t<F>>
             && (!is_base_of_v<member_pointer_class_t<F>, remove_cvref_t<O>>)
constexpr auto invoke_impl(F&& f, O&& o) noexcept(noexcept((*static_cast<O&&>(o)).*f))
    -> decltype((*static_cast<O&&>(o)).*f) {
    return (*static_cast<O&&>(o)).*f;
}

// ordinary callable
template<typename F, typename... A>
    requires(!is_member_pointer_v<remove_cvref_t<F>>)
constexpr auto
invoke_impl(F&& f, A&&... a) noexcept(noexcept(static_cast<F&&>(f)(static_cast<A&&>(a)...)))
    -> decltype(static_cast<F&&>(f)(static_cast<A&&>(a)...)) {
    return static_cast<F&&>(f)(static_cast<A&&>(a)...);
}

template<typename Void, typename F, typename... A>
struct invoke_result_impl {};

template<typename F, typename... A>
struct invoke_result_impl<void_t<decltype(invoke_impl(declval<F>(), declval<A>()...))>, F, A...> {
    using type = decltype(invoke_impl(declval<F>(), declval<A>()...));
};
} // namespace detail

// invoke_result
template<typename F, typename... Args>
struct invoke_result : detail::invoke_result_impl<void, F, Args...> {};

template<typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

// unwrap_reference
template<typename T>
struct unwrap_reference {
    using type = T;
};

template<typename U>
struct unwrap_reference<cleo::reference_wrapper<U>> {
    using type = U&;
};

template<typename T>
using unwrap_reference_t = typename unwrap_reference<T>::type;

// unwrap_ref_decay
template<typename T>
struct unwrap_ref_decay : unwrap_reference<decay_t<T>> {};

template<typename T>
using unwrap_ref_decay_t = typename unwrap_ref_decay<T>::type;

// [meta.logical], logical operator traits

// conjunction
template<typename...>
struct conjunction : true_type {};

template<typename B1>
struct conjunction<B1> : B1 {};

template<typename B1, typename... Bn>
struct conjunction<B1, Bn...> : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

template<typename... B>
constexpr bool conjunction_v = conjunction<B...>::value;

// disjunction
template<typename...>
struct disjunction : false_type {};

template<typename B1>
struct disjunction<B1> : B1 {};

template<typename B1, typename... Bn>
struct disjunction<B1, Bn...> : conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

template<typename... B>
constexpr bool disjunction_v = disjunction<B...>::value;

// negation
template<typename B>
struct negation : bool_constant<!bool(B::value)> {};

template<typename B>
constexpr bool negation_v = negation<B>::value;

// [meta.rel], type relations that depend on INVOKE

// is_invocable
namespace detail {
template<typename Void, typename F, typename... A>
struct is_invocable_impl : false_type {};

template<typename F, typename... A>
struct is_invocable_impl<void_t<invoke_result_t<F, A...>>, F, A...> : true_type {};
} // namespace detail

template<typename F, typename... Args>
struct is_invocable : detail::is_invocable_impl<void, F, Args...> {};

template<typename F, typename... Args>
inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

// is_invocable_r
namespace detail {
template<typename R, bool Invocable, typename F, typename... A>
struct is_invocable_r_impl : false_type {};

template<typename R, typename F, typename... A>
struct is_invocable_r_impl<R, true, F, A...>
    : integral_constant<bool, is_void_v<R> || is_convertible_v<invoke_result_t<F, A...>, R>> {};
} // namespace detail

template<typename R, typename F, typename... Args>
struct is_invocable_r : detail::is_invocable_r_impl<R, is_invocable_v<F, Args...>, F, Args...> {};

template<typename R, typename F, typename... Args>
inline constexpr bool is_invocable_r_v = is_invocable_r<R, F, Args...>::value;

// is_nothrow_invocable / is_nothrow_invocable_r
namespace detail {
template<typename F, typename... A>
inline constexpr bool invoke_is_nothrow = noexcept(invoke_impl(declval<F>(), declval<A>()...));

template<bool Invocable, typename F, typename... A>
struct is_nothrow_invocable_impl : false_type {};

template<typename F, typename... A>
struct is_nothrow_invocable_impl<true, F, A...> : bool_constant<invoke_is_nothrow<F, A...>> {};

template<typename R, bool InvocableR, typename F, typename... A>
struct is_nothrow_invocable_r_impl : false_type {};

template<typename R, typename F, typename... A>
struct is_nothrow_invocable_r_impl<R, true, F, A...>
    : bool_constant<
          invoke_is_nothrow<F, A...>
          && (is_void_v<R> || is_nothrow_convertible_v<invoke_result_t<F, A...>, R>)> {};
} // namespace detail

template<typename F, typename... Args>
struct is_nothrow_invocable
    : detail::is_nothrow_invocable_impl<is_invocable_v<F, Args...>, F, Args...> {};

template<typename F, typename... Args>
inline constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<F, Args...>::value;

template<typename R, typename F, typename... Args>
struct is_nothrow_invocable_r
    : detail::is_nothrow_invocable_r_impl<R, is_invocable_r_v<R, F, Args...>, F, Args...> {};

template<typename R, typename F, typename... Args>
inline constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, F, Args...>::value;

// meta.const.eval
// is_constant_evaluated
constexpr bool is_constant_evaluated() noexcept {
    if consteval {
        return true;
    }
    else {
        return false;
    }
}

} // namespace cleo
