#pragma once

#include <climits>
#include <random>
#include <vector>

namespace constants {

inline constexpr size_t page_size = 1 << 21;
inline constexpr int block_len = 16;

}  // namespace constants

inline std::vector<int> generate_random_data(size_t n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, INT_MAX);

    std::vector<int> data(n);
    for (size_t i = 0; i < n; i++) {
        data[i] = dis(gen);
    }
    return data;
}
