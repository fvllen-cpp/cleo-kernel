#include "type_traits.hpp"

#include <gtest/gtest.h>

#include "functional.hpp"
#include "utility.hpp"

namespace {

using namespace cleo;

struct Base {};

struct Derived : Base {};

enum LegacyEnum {
    LE_A
};
enum class ScopedEnum {
    A
};

struct Widget {
    int value;
    int compute(int) const noexcept; // declared only; never called
};

// [meta.unary.cat] / [meta.unary.comp]
static_assert(is_integral_v<int>);
static_assert(is_integral_v<const unsigned long>);
static_assert(!is_integral_v<float>);
static_assert(is_floating_point_v<double>);
static_assert(!is_floating_point_v<int>);
static_assert(is_signed_v<int> && !is_signed_v<unsigned>);
static_assert(is_unsigned_v<unsigned> && !is_unsigned_v<int>);
static_assert(is_arithmetic_v<char>);
static_assert(is_function_v<int(int)>);
static_assert(is_function_v<void() const>); // abominable function type
static_assert(!is_function_v<int>);
static_assert(is_member_function_pointer_v<decltype(&Widget::compute)>);
static_assert(is_member_object_pointer_v<decltype(&Widget::value)>);
static_assert(!is_member_object_pointer_v<decltype(&Widget::compute)>);
static_assert(is_class_v<Base> && !is_class_v<LegacyEnum>);
static_assert(is_enum_v<LegacyEnum>);
static_assert(is_scoped_enum_v<ScopedEnum> && !is_scoped_enum_v<LegacyEnum>);
static_assert(is_scalar_v<int*> && is_object_v<Base>);
static_assert(is_pointer_v<int*> && !is_pointer_v<int>);
static_assert(is_array_v<int[4]> && is_bounded_array_v<int[4]> && is_unbounded_array_v<int[]>);

// [meta.rel]
static_assert(is_same_v<int, int> && !is_same_v<int, long>);
static_assert(is_base_of_v<Base, Derived>);
static_assert(is_convertible_v<Derived*, Base*> && !is_convertible_v<Base*, Derived*>);

// [meta.unary.prop]
static_assert(is_trivially_copyable_v<Base>);
static_assert(is_default_constructible_v<Widget>);
static_assert(is_nothrow_destructible_v<Widget>);

// invoke / is_invocable family
static_assert(is_invocable_v<int (*)(int), int>);
static_assert(is_invocable_v<decltype(&Widget::compute), Widget, int>);
static_assert(is_invocable_v<decltype(&Widget::compute), Widget*, int>);
static_assert(is_invocable_v<decltype(&Widget::value), Widget>);
static_assert(!is_invocable_v<int, int>);
static_assert(is_invocable_r_v<long, int (*)(int), int>);
static_assert(is_nothrow_invocable_v<decltype(&Widget::compute), Widget*, int>);
static_assert(is_same_v<invoke_result_t<int (*)(int), int>, int>);

// swappable
static_assert(is_swappable_v<int> && is_nothrow_swappable_v<int>);

} // namespace

// The trait coverage is the static_assert block above; this TU passing compilation
// is the test. The runtime cases below exercise the actual invoke/swap code paths.
TEST(TypeTraits, CompileTimeChecks) {
    SUCCEED();
}

TEST(Utility, SwapRuntime) {
    int a = 1, b = 2;
    cleo::swap(a, b);
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 1);
}

TEST(Functional, Invoke) {
    Widget w{40};
    auto add = [](int x, int y) { return x + y; };
    EXPECT_EQ(cleo::invoke(add, 2, 3), 5);
    EXPECT_EQ(cleo::invoke(&Widget::value, w), 40);
}
