#include <immintrin.h>
#include <sys/mman.h>

#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>

#include "common.hpp"
#include "trees/base.hpp"

template <size_t N>
class bplus : public tree_base {
private:
    static constexpr int block_count(int num_keys) {
        // block_count = ceil(num_keys / block_length)
        return (num_keys + (constants::block_len - 1)) / constants::block_len;
    }

    static constexpr int parent_layer_keys(int num_keys) {
        // parent_size = ceil(blocks_on_layer / children_per_block)
        return (block_count(num_keys) + constants::block_len) / (constants::block_len + 1) *
               constants::block_len;
    }

    static constexpr int height(int num_keys) {
        if (num_keys <= constants::block_len) {
            return 1;
        }

        return 1 + height(parent_layer_keys(num_keys));
    }

    static constexpr size_t offset(int layer) {
        size_t i = 0, num_keys = N;
        while (layer--) {
            i += block_count(num_keys) * constants::block_len;
            num_keys = parent_layer_keys(num_keys);
        }
        return i;
    }

    static constexpr int num_layers = height(N);
    static constexpr size_t size = offset(num_layers);  // offset of non-existent last layer

public:
    bplus(std::span<const int> data) {
        size_t bytes = size * sizeof(int);
        size_t padded_bytes =
            (bytes + constants::page_size - 1) / constants::page_size * constants::page_size;

        _tree = static_cast<int*>(std::aligned_alloc(constants::page_size, padded_bytes));
        madvise(_tree, padded_bytes, MADV_HUGEPAGE);

        build(data);
    }

    ~bplus() {
        std::free(_tree);
    }

    int lower_bound(int target) const noexcept {
        int k = 0;

        __m512i target_vec = _mm512_set1_epi32(target);
        for (int h = num_layers - 1; h > 0; h--) {
            int i = first_ge(target_vec, _tree + offset(h) + k);

            k = k * (constants::block_len + 1) + i * constants::block_len;
        }

        int i = first_ge(target_vec, _tree + k);
        return _tree[k + i];
    }

private:
    int* _tree;

    void build(std::span<const int> data) {
        memcpy(_tree, data.data(), N * sizeof(int));
        for (size_t i = N; i < size; i++) {
            _tree[i] = INT_MAX;
        }

        for (size_t h = 1; h < num_layers; h++) {
            size_t layer_start = offset(h);
            size_t layer_end = offset(h + 1);

            for (size_t i = 0; i < (layer_end - layer_start); i++) {
                int block = i / constants::block_len;
                int block_key_offset = i - block * constants::block_len;

                int child_layer_start = block * (constants::block_len + 1);
                int right_child_block = (child_layer_start + block_key_offset) + 1;

                int leftmost_block = right_child_block;
                for (size_t l = 0; l < h - 1; l++) {
                    leftmost_block *= (constants::block_len + 1);
                }

                size_t leftmost_index = leftmost_block * constants::block_len;
                _tree[layer_start + i] = (leftmost_index < N ? _tree[leftmost_index] : INT_MAX);
            }
        }
    }

    static_assert(constants::block_len == 16);
    int first_ge(__m512i target_vec, int* block) const noexcept {
        __m512i data = _mm512_load_si512(reinterpret_cast<__m512i*>(block));
        __mmask16 mask = _mm512_cmpge_epi32_mask(data, target_vec);
        return __tzcnt_u16(mask);
    }
};
