#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "trees/btree.hpp"

constexpr u32 num_queries = (1 << 20);

static void BM_StdLowerBound(benchmark::State& state) {
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

template <typename tree_type>
static void BM_Tree(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    static const auto queries = generate_random_data(num_queries);
    static size_t query_idx = 0;

    tree_type tree(data);

    for (auto _ : state) {
        int query = queries[query_idx++ & (num_queries - 1)];
        benchmark::DoNotOptimize(tree.lower_bound(query));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_StdLowerBound)->DenseRange(1, 30);
BENCHMARK_TEMPLATE(BM_Tree, btree_eytzinger)->DenseRange(1, 30);

BENCHMARK_MAIN();
