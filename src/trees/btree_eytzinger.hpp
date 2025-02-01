#include <immintrin.h>
#include <sys/mman.h>

#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <span>

#include "common.hpp"
#include "trees/base.hpp"

class btree_eytzinger : public tree_base {
public:
    btree_eytzinger(std::span<const int> data) {
        _nblocks = std::ceil(static_cast<f64>(data.size()) / constants::block_len);

        u64 bytes_required = (constants::block_len * sizeof(int)) * _nblocks;
        u16 pages_required = std::ceil(static_cast<f64>(bytes_required) / constants::page_size);

        _tree = static_cast<int*>(
            std::aligned_alloc(constants::page_size, constants::page_size * pages_required));
        madvise(_tree, pages_required, MADV_HUGEPAGE);

        size_t pos = 0;
        build(data, pos);
    }

    ~btree_eytzinger() {
        std::free(_tree);
    }

    int lower_bound(int target) const noexcept {
        int found = 0;

        u32 block = 0;
        while (block < _nblocks) {
            int i = first_ge(target, &_tree[block * constants::block_len]);
            if (i < constants::block_len) {
                found = _tree[block * constants::block_len + i];
            }

            block = child(block, i);
        }

        return found;
    }

private:
    int* _tree;
    u32 _nblocks;

    void build(std::span<const int> data, size_t& pos, u32 block = 0) {
        if (block < _nblocks) {
            for (int i = 0; i < constants::block_len; i++) {
                build(data, pos, child(block, i));

                _tree[block * constants::block_len + i] =
                    (pos < data.size()) ? data[pos++] : INT_MAX;
            }

            build(data, pos, child(block, constants::block_len));
        }
    }

    u32 child(u32 block, u16 offset) const {
        u32 child_block = 1 + block * (constants::block_len + 1);
        return child_block + offset;
    }

    // NOTE: This is difficult to make dynamic due to the interaction with lane width.
    // constants::block_len is more for readability, once changed, this will need updating.
    static_assert(constants::block_len == 16);
    int first_ge(int target, int* block) const noexcept {
        __m512i target_vec = _mm512_set1_epi32(target);
        __m512i data = _mm512_load_si512(reinterpret_cast<__m512i*>(block));
        __mmask16 mask = _mm512_cmpge_epi32_mask(data, target_vec);
        return __tzcnt_u16(mask);
    }
};
