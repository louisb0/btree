#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "trees/btree.hpp"

constexpr size_t num_queries = (1 << 28);

static void BM_lower_bound(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    static const auto queries = generate_random_data(num_queries);
    static size_t query_idx = 0;

    for (auto _ : state) {
        int query = queries[query_idx++ & (num_queries - 1)];
        benchmark::DoNotOptimize(std::lower_bound(data.begin(), data.end(), query));
        benchmark::ClobberMemory();
    }
}

static void BM_btree(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    static const auto queries = generate_random_data(num_queries);
    static size_t query_idx = 0;

    btree tree(data);

    for (auto _ : state) {
        int query = queries[query_idx++ & (num_queries - 1)];
        benchmark::DoNotOptimize(tree.lower_bound(query));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_lower_bound)->DenseRange(1, 30);
BENCHMARK(BM_btree)->DenseRange(1, 30);

BENCHMARK_MAIN();
