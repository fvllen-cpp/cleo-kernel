#pragma once

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
template<class T>
struct remove_cv {
    using type = T;
};

template<class T>
struct remove_cv<const T> {
    using type = T;
};

template<class T>
struct remove_cv<volatile T> {
    using type = T;
};

template<class T>
struct remove_cv<const volatile T> {
    using type = T;
};

template<class T>
using remove_cv_t = typename remove_cv<T>::type;

// remove_const
template<class T>
struct remove_const {
    using type = T;
};

template<class T>
struct remove_const<const T> {
    using type = T;
};

template<class T>
using remove_const_t = typename remove_const<T>::type;

// remove_volatile
template<class T>
struct remove_volatile {
    using type = T;
};

template<class T>
struct remove_volatile<volatile T> {
    using type = T;
};

template<class T>
using remove_volatile_t = typename remove_volatile<T>::type;

// [meta.trans.ref], reference modifications

// remove_reference
template<class T>
struct remove_reference {
    using type = T;
};

template<class T>
struct remove_reference<T&> {
    using type = T;
};

template<class T>
struct remove_reference<T&&> {
    using type = T;
};

template<class T>
using remove_reference_t = typename remove_reference<T>::type;

// [meta.rel], type relations

// is_same
template<class T, class U>
struct is_same : false_type {};

template<class T>
struct is_same<T, T> : true_type {};

template<class T, class U>
inline constexpr bool is_same_v = is_same<T, U>::value;

// [meta.unary.cat], Primary type categories

// is_lvalue_reference
template<class T>
struct is_lvalue_reference_helper : false_type {};

template<class T>
struct is_lvalue_reference_helper<T&> : true_type {};

template<class T>
struct is_lvalue_reference : is_lvalue_reference_helper<remove_cv_t<T>> {};

template<class T>
inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

// is_rvalue_reference
template<class T>
struct is_rvalue_reference_helper : false_type {};

template<class T>
struct is_rvalue_reference_helper<T&&> : true_type {};

template<class T>
struct is_rvalue_reference : is_rvalue_reference_helper<remove_cv_t<T>> {};

template<class T>
inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

} // namespace cleo
