#pragma once

namespace cleo {

[[nodiscard]] void* allocate(cleo::size_t bytes, cleo::size_t alignment) noexcept;

void deallocate(void* ptr, cleo::size_t bytes, cleo::size_t alignment) noexcept;

} // namespace cleo
