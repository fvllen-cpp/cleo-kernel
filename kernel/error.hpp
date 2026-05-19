#pragma once

#include <cstdint>

namespace cleo {
enum class error : uint32_t {
    ok = 0,

    // generic
    invalid_argument,
    invalid_state,
    not_found,
    unsupported,

    // memory
    out_of_memory,

    // IPC
    buffer_full,
    buffer_empty,
};
} // namespace cleo
