#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <expected>
#include <new>

#include "error.hpp"

namespace cleo {

template<typename T, std::size_t N>
class RingBuffer {
public:
    RingBuffer();
    ~RingBuffer() = default;

    std::expected<void, cleo::error> push(const T& val) noexcept {
        if (full()) {
            return std::unexpected(cleo::error::buffer_full);
        }

        const auto head = head_.load(std::memory_order_relaxed);

        rb_[head % N] = val;
        head_.store(head + 1, std::memory_order_release);
        return {};
    }

    std::expected<void, cleo::error> push(T&& val) noexcept {
        if (full()) {
            return std::unexpected(cleo::error::buffer_full);
        }

        const auto head = head_.load(std::memory_order_relaxed);

        rb_[head % N] = std::move(val);
        head_.store(head + 1, std::memory_order_release);
        return {};
    }

    std::expected<void, cleo::error> pop() noexcept {
        if (empty()) {
            return std::unexpected(cleo::error::buffer_empty);
        }

        const auto tail = tail_.load(std::memory_order_relaxed);
        tail_.store(tail + 1, std::memory_order_release);
        return {};
    }

    [[nodiscard]] std::expected<T&, cleo::error> front() noexcept {
        if (empty()) {
            return std::unexpected(cleo::error::buffer_empty);
        }

        const auto head = head_.load(std::memory_order_acquire);

        return rb_[head % N];
    }

    [[nodiscard]] std::expected<T&, cleo::error> back() noexcept {
        if (empty()) {
            return std::unexpected(cleo::error::buffer_empty);
        }

        const auto tail = tail_.load(std::memory_order_acquire);

        return rb_[tail % N];
    }

    [[nodiscard]] bool full() const noexcept {
        return size() == N;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto tail = tail_.load(std::memory_order_relaxed);
        return static_cast<std::size_t>(head - tail);
    }

private:
    std::array<T, N> rb_{};
    alignas(std::hardware_destructive_interference_size) std::atomic<std::uint64_t> head_;
    alignas(std::hardware_destructive_interference_size) std::atomic<std::uint64_t> tail_;
};

} // namespace cleo
