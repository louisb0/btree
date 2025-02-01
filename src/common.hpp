#pragma once

#include <climits>
#include <cstdint>
#include <random>
#include <vector>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using f32 = float;
using f64 = double;

using b8 = bool;

namespace constants {
inline constexpr u32 page_size = 1 << 21;
inline constexpr u16 block_len = 16;
}  // namespace constants

inline std::vector<int> generate_random_data(std::size_t n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, INT_MAX);

    std::vector<int> data(n);
    for (size_t i = 0; i < n; i++) {
        data[i] = dis(gen);
    }
    return data;
}
