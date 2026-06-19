#pragma once

namespace cleo {

using nullptr_t = decltype(nullptr);
using size_t = decltype(sizeof(0));
using ptrdiff_t = decltype((char*)0 - (char*)0);

enum class byte : unsigned char {
};

} // namespace cleo
