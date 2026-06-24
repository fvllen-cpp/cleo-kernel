#pragma once

#include "cstddef.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

namespace cleo {

// [addressof]
template<typename T>
[[nodiscard]] constexpr T* addressof(T& r) noexcept {
    return __builtin_addressof(r);
}

template<typename T>
const T* addressof(const T&&) = delete;

// exposition
namespace detail {
template<typename Ptr>
struct __element_type_of {};

template<typename Ptr>
    requires requires { typename Ptr::element_type; }
struct __element_type_of<Ptr> {
    using type = typename Ptr::element_type;
};

template<template<typename...> typename SomePointer, typename T, typename... Args>
    requires(!requires { typename SomePointer<T, Args...>::element_type; })
struct __element_type_of<SomePointer<T, Args...>> {
    using type = T;
};

template<typename Ptr>
concept __has_element_type = requires { typename __element_type_of<Ptr>::type; };

template<typename Ptr>
struct __pointer_difference_type {
    using type = cleo::ptrdiff_t;
};

template<typename Ptr>
    requires requires { typename Ptr::difference_type; }
struct __pointer_difference_type<Ptr> {
    using type = typename Ptr::difference_type;
};

template<typename Ptr, typename U>
struct __pointer_rebind {};

template<typename Ptr, typename U>
    requires requires { typename Ptr::template rebind<U>; }
struct __pointer_rebind<Ptr, U> {
    using type = typename Ptr::template rebind<U>;
};

template<template<typename...> typename SomePointer, typename T, typename... Args, typename U>
    requires(!requires { typename SomePointer<T, Args...>::template rebind<U>; })
struct __pointer_rebind<SomePointer<T, Args...>, U> {
    using type = SomePointer<U, Args...>;
};

template<typename Ptr, typename Element, bool = cleo::is_void_v<cleo::remove_cv_t<Element>>>
struct pointer_traits_pointer_to {};

template<typename Ptr, typename Element>
struct pointer_traits_pointer_to<Ptr, Element, false> {
    static constexpr Ptr pointer_to(Element& r)
        requires requires(Element& e) { Ptr::pointer_to(e); }
    {
        return Ptr::pointer_to(r);
    }
};

template<typename T>
struct pointer_traits_pointer_to<T*, T, false> {
    static constexpr T* pointer_to(T& r) noexcept {
        return cleo::addressof(r);
    }
};

} // namespace detail

// [pointer_traits]
template<typename Ptr>
struct pointer_traits {};

template<typename Ptr>
    requires detail::__has_element_type<Ptr>
struct pointer_traits<Ptr>
    : detail::pointer_traits_pointer_to<Ptr, typename detail::__element_type_of<Ptr>::type> {
    // Member types
    using pointer = Ptr;
    using element_type = typename detail::__element_type_of<Ptr>::type;
    using difference_type = typename detail::__pointer_difference_type<Ptr>::type;

    // Member alias templates
    template<typename U>
    using rebind = typename detail::__pointer_rebind<Ptr, U>::type;
};

template<typename T>
struct pointer_traits<T*> : detail::pointer_traits_pointer_to<T*, T> {
    // Member types
    using pointer = T*;
    using element_type = T;
    using difference_type = cleo::ptrdiff_t;

    // Member alias templates
    template<typename U>
    using rebind = U*;
};

// [to_address]
template<typename T>
constexpr T* to_address(T* p) noexcept
    requires(!cleo::is_function_v<T>)
{
    return p;
}

template<typename Ptr>
constexpr auto to_address(const Ptr& p) noexcept {
    if constexpr (requires { cleo::pointer_traits<Ptr>::to_address(p); })
        return cleo::pointer_traits<Ptr>::to_address(p);
    else
        return cleo::to_address(p.operator->());
}

// [construct_at]
template<typename T, typename... Args>
constexpr T* construct_at(T* location, Args&&... args) {
    if constexpr (cleo::is_array_v<T>)
        return ::new (voidify(*location)) T[1]();
    else
        return ::new (voidify(*location)) T(cleo::forward<Args>(args)...);
}

// [destroy_at]
template<typename T>
constexpr void destroy_at(T* p) {
    if constexpr (cleo::is_array_v<T>)
        for (auto& elem : *p)
            (destroy_at)(cleo::addressof(elem));
    else
        p->~T();
}

// [allocation_result]
template<typename Pointer, typename SizeType = cleo::size_t>
struct allocation_result {
    Pointer ptr;
    SizeType count;
};

// [uses_allocator]
namespace detail::memory {
template<typename T, typename Alloc>
struct __uses_allocator : cleo::false_type {};

template<typename T, typename Alloc>
    requires requires { typename T::allocator_type; }
struct __uses_allocator<T, Alloc>
    : cleo::bool_constant<cleo::is_convertible_v<Alloc, typename T::allocator_type>> {};

} // namespace detail::memory

template<typename T, typename Alloc>
struct uses_allocator : detail::memory::__uses_allocator<T, Alloc> {};

template<typename T, typename Alloc>
constexpr bool uses_allocator_v = uses_allocator<T, Alloc>::value;

// [allocator_traits]
namespace detail {
namespace memory {
namespace allocator_traits {

// pointer
template<typename Alloc>
struct __pointer {
    using type = typename Alloc::value_type*;
};

template<typename Alloc>
    requires requires { typename Alloc::pointer; }
struct __pointer<Alloc> {
    using type = typename Alloc::pointer;
};

template<typename Alloc>
using __pointer_t = typename __pointer<Alloc>::type;

// const pointer
template<typename Alloc>
struct __const_pointer {
    using type = typename pointer_traits<__pointer_t<Alloc>>::template rebind<
        const typename Alloc::value_type>;
};

template<typename Alloc>
    requires requires { typename Alloc::const_pointer; }
struct __const_pointer<Alloc> {
    using type = typename Alloc::const_pointer;
};

template<typename Alloc>
using __const_pointer_t = typename __const_pointer<Alloc>::type;

// void pointer
template<typename Alloc>
struct __void_pointer {
    using type = typename pointer_traits<__pointer_t<Alloc>>::template rebind<void>;
};

template<typename Alloc>
    requires requires { typename Alloc::void_pointer; }
struct __void_pointer<Alloc> {
    using type = typename Alloc::void_pointer;
};

template<typename Alloc>
using __void_pointer_t = typename __void_pointer<Alloc>::type;

// const void pointer
template<typename Alloc>
struct __const_void_pointer {
    using type = typename pointer_traits<__pointer_t<Alloc>>::template rebind<const void>;
};

template<typename Alloc>
    requires requires { typename Alloc::const_void_pointer; }
struct __const_void_pointer<Alloc> {
    using type = typename Alloc::const_void_pointer;
};

template<typename Alloc>
using __const_void_pointer_t = typename __const_void_pointer<Alloc>::type;

// difference type
template<typename Alloc>
struct __difference_type {
    using type = typename pointer_traits<__pointer_t<Alloc>>::difference_type;
};

template<typename Alloc>
    requires requires { typename Alloc::difference_type; }
struct __difference_type<Alloc> {
    using type = typename Alloc::difference_type;
};

template<typename Alloc>
using __difference_type_t = typename __difference_type<Alloc>::type;

// size type
template<typename Alloc>
struct __size_type {
    using type = cleo::make_unsigned_t<__difference_type_t<Alloc>>;
};

template<typename Alloc>
    requires requires { typename Alloc::size_type; }
struct __size_type<Alloc> {
    using type = typename Alloc::size_type;
};

template<typename Alloc>
using __size_type_t = typename __size_type<Alloc>::type;

// propagate on container copy assignment
template<typename Alloc>
struct __propagate_on_container_copy_assignment {
    using type = cleo::false_type;
};

template<typename Alloc>
    requires requires { typename Alloc::propagate_on_container_copy_assignment; }
struct __propagate_on_container_copy_assignment<Alloc> {
    using type = typename Alloc::propagate_on_container_copy_assignment;
};

template<typename Alloc>
using __propagate_on_container_copy_assignment_t =
    typename __propagate_on_container_copy_assignment<Alloc>::type;

// propagate on container move assignment
template<typename Alloc>
struct __propagate_on_container_move_assignment {
    using type = cleo::false_type;
};

template<typename Alloc>
    requires requires { typename Alloc::propagate_on_container_move_assignment; }
struct __propagate_on_container_move_assignment<Alloc> {
    using type = typename Alloc::propagate_on_container_move_assignment;
};

template<typename Alloc>
using __propagate_on_container_move_assignment_t =
    typename __propagate_on_container_move_assignment<Alloc>::type;

// propagate on container swap
template<typename Alloc>
struct __propagate_on_container_swap {
    using type = cleo::false_type;
};

template<typename Alloc>
    requires requires { typename Alloc::propagate_on_container_swap; }
struct __propagate_on_container_swap<Alloc> {
    using type = typename Alloc::propagate_on_container_swap;
};

template<typename Alloc>
using __propagate_on_container_swap_t = typename __propagate_on_container_swap<Alloc>::type;

// is always equal
template<typename Alloc>
struct __is_always_equal {
    using type = typename cleo::is_empty<Alloc>::type;
};

template<typename Alloc>
    requires requires { typename Alloc::is_always_equal; }
struct __is_always_equal<Alloc> {
    using type = typename Alloc::is_always_equal;
};

template<typename Alloc>
using __is_always_equal_t = typename __is_always_equal<Alloc>::type;

// rebind alloc
template<typename Alloc, typename T>
struct __rebind_alloc {};

template<typename Alloc, typename T>
    requires requires { typename Alloc::template rebind<T>::other; }
struct __rebind_alloc<Alloc, T> {
    using type = typename Alloc::template rebind<T>::other;
};

template<
    template<typename, typename...> typename SomeAllocator,
    typename U,
    typename... Args,
    typename T>
    requires(!requires { typename SomeAllocator<U, Args...>::template rebind<T>::other; })
struct __rebind_alloc<SomeAllocator<U, Args...>, T> {
    using type = SomeAllocator<T, Args...>;
};

template<typename Alloc, typename T>
using __rebind_alloc_t = typename __rebind_alloc<Alloc, T>::type;

} // namespace allocator_traits
} // namespace memory
} // namespace detail

template<typename Alloc>
struct allocator_traits {
    // Member types
    using allocator_type = Alloc;
    using value_type = typename Alloc::value_type;
    using pointer = detail::memory::allocator_traits::__pointer_t<Alloc>;
    using const_pointer = detail::memory::allocator_traits::__const_pointer_t<Alloc>;
    using void_pointer = detail::memory::allocator_traits::__void_pointer_t<Alloc>;
    using const_void_pointer = detail::memory::allocator_traits::__const_void_pointer_t<Alloc>;
    using difference_type = detail::memory::allocator_traits::__difference_type_t<Alloc>;
    using size_type = detail::memory::allocator_traits::__size_type_t<Alloc>;
    using propagate_on_container_copy_assignment =
        detail::memory::allocator_traits::__propagate_on_container_copy_assignment_t<Alloc>;
    using propagate_on_container_move_assignment =
        detail::memory::allocator_traits::__propagate_on_container_move_assignment_t<Alloc>;
    using propagate_on_container_swap =
        detail::memory::allocator_traits::__propagate_on_container_swap_t<Alloc>;
    using is_always_equal = detail::memory::allocator_traits::__is_always_equal_t<Alloc>;

    // Member alias templates
    template<typename T>
    using rebind_alloc = detail::memory::allocator_traits::__rebind_alloc_t<Alloc, T>;

    template<typename T>
    using rebind_traits = cleo::allocator_traits<rebind_alloc<T>>;

    // Member functions
    static constexpr pointer allocate(Alloc& a, size_type n) {
        return a.allocate(n);
    }

    static constexpr pointer allocate(Alloc& a, size_type n, const_void_pointer hint) {
        if constexpr (requires { a.allocate(n, hint); }) {
            return a.allocate(n, hint);
        }
        else {
            (void)hint;
            return a.allocate(n);
        }
    }

    static constexpr cleo::allocation_result<pointer, size_type>
    allocate_at_least(Alloc& a, size_type n) {
        if constexpr (requires { a.allocate_at_least(n); }) {
            return a.allocate_at_least(n);
        }
        else {
            return {a.allocate(n), n};
        }
    }

    static constexpr void deallocate(Alloc& a, pointer p, size_type n) {
        a.deallocate(p, n);
    }

    template<typename T, typename... Args>
    static constexpr void construct(Alloc& a, T* p, Args&&... args) {
        if constexpr (requires { a.construct(p, cleo::forward<Args>(args)...); }) {
            a.construct(p, cleo::forward<Args>(args)...);
        }
        else {
            cleo::construct_at(p, cleo::forward<Args>(args)...);
        }
    }

    template<typename T>
    static constexpr void destroy(Alloc& a, T* p) {
        if constexpr (requires { a.destroy(p); }) {
            a.destroy(p);
        }
        else {
            cleo::destroy_at(p);
        }
    }

    static constexpr size_type max_size(const Alloc& a) noexcept {
        if constexpr (requires { a.max_size(); }) {
            return a.max_size();
        }
        else {
            return cleo::numeric_limits<size_type>::max() / sizeof(value_type);
        }
    }

    static constexpr Alloc select_on_container_copy_construction(const Alloc& a) {
        if constexpr (requires { a.select_on_container_copy_construction(); }) {
            return a.select_on_container_copy_construction();
        }
        else {
            return a;
        }
    }
};

// [allocator]
template<typename T>
class allocator {
public:
    // Member types
    using value_type = T;
    using size_type = cleo::size_t;
    using difference_type = cleo::ptrdiff_t;
    using propagate_on_container_move_assignment = cleo::true_type;

    // Member functions
    constexpr allocator() noexcept {
    }

    constexpr allocator(const allocator& other) noexcept {
    }

    template<typename U>
    constexpr allocator(const allocator<U>& other) noexcept {
    }

    constexpr ~allocator() {
    }

    constexpr T* allocate(cleo::size_t n) {
    }

    constexpr cleo::allocation_result<T*, cleo::size_t> allocate_at_least(cleo::size_t n) {
    }

    constexpr void deallocate(T* p, cleo::size_t n) {
    }

private:
};

template<typename T1, typename T2>
constexpr bool operator==(const allocator<T1>& lhs, const allocator<T2>& rhs) noexcept {
}

} // namespace cleo
