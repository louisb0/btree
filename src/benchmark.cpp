#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "common.hpp"
#include "trees/bplus.hpp"
#include "trees/btree.hpp"

const std::vector<int> queries = generate_random_data(1 << 28);
size_t query_idx = 0;

static void BM_lower_bound(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);
    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    for (auto _ : state) {
        int query = queries[query_idx++ & (queries.size() - 1)];
        benchmark::DoNotOptimize(std::lower_bound(data.begin(), data.end(), query));
        benchmark::ClobberMemory();
    }
}

static void BM_btree(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);
    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());
    btree tree(data);

    for (auto _ : state) {
        int query = queries[query_idx++ & (queries.size() - 1)];
        benchmark::DoNotOptimize(tree.lower_bound(query));
        benchmark::ClobberMemory();
    }
}

template <size_t power>
static void BM_bplus_helper(benchmark::State& state) {
    constexpr size_t elements = (1ULL << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    bplus<elements> tree(data);

    for (auto _ : state) {
        int query = queries[query_idx++ & (queries.size() - 1)];
        benchmark::DoNotOptimize(tree.lower_bound(query));
        benchmark::ClobberMemory();
    }
}
template <size_t I = 1>
static void BM_bplus(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_bplus_helper<I>(state);
    } else if constexpr (I < 30) {
        BM_bplus<I + 1>(state);
    }
}

BENCHMARK(BM_lower_bound)->DenseRange(1, 30);
BENCHMARK(BM_btree)->DenseRange(1, 30);
BENCHMARK(BM_bplus)->DenseRange(1, 30);

BENCHMARK_MAIN();
