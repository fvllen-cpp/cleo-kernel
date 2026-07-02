#pragma once

namespace cleo {

class tlsf {
public:
    tlsf() = default;

    tlsf(void* mem_pool, cleo::size_t pool_size) {
        initialize(mem_pool, pool_size, sli_);
    }

    tlsf(const tlsf&) = delete;
    tlsf& operator=(const tlsf&) = delete;

    void initialize(void* memory, cleo::size_t bytes, cleo::size_t sli) noexcept;

    [[nodiscard]] void* allocate(cleo::size_t bytes, cleo::size_t alignment) noexcept;

    void deallocate(void* ptr) noexcept;

private:
    struct block_header {
        cleo::size_t size;
        block_header* prev_phys_block;
        block_header* next_free;
        block_header* prev_free;
    };

    struct mapping {
        unsigned fl;
        unsigned sl;
    };

    template<typename T>
        requires cleo::is_integral<T> && cleo::is_unsigned<T>
    constexpr cleo::size_t fls(T size) {
        constexpr cleo::size_t bit_width = sizeof(T) * 8;
        return (bit_width - 1) - __builtin_clzg(size);
    }

    constexpr mapping mapping_insert(cleo::size_t size) noexcept {
        const unsigned fl = fls(size);
        const unsigned sl = (size >> (fl - sli_)) - sl_count_;
        return {fl, sl};
    }

    constexpr mapping mapping_search(cleo::size_t size) noexcept {
        size = size + (1u << (fls(size) - sli_)) - 1;
        return mapping_insert(size);
    }

    constexpr void insert(const block_header& block) noexcept {
    }

    constexpr block_header& search_free(const cleo::size_t size) noexcept {
    }

    constexpr block_header& search_adj(const block_header& block) noexcept {
    }

    constexpr void remove(const block_header& block) noexcept {
    }

    constexpr void coalesce(const block_header& block) noexcept {
    }

    static constexpr bool has_single_bit(cleo::size_t x) noexcept {
        return x != 0 && (x & (x - 1)) == 0;
    }

    static constexpr unsigned floor_log2_const(cleo::size_t x) noexcept {
        unsigned result = 0;

        while (x > 1) {
            x >>= 1;
            ++result;
        }

        return result;
    }

    cleo::size_t fli_;
    static constexpr cleo::size_t sli_ = 4;
    static constexpr cleo::size_t mbs_ = 16;
    static constexpr cleo::size_t sl_count_ = (1 << sli_);

    static_assert(has_single_bit(mbs_));
};

} // namespace cleo
