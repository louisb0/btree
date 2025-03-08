#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdio>
#include <random>
#include <vector>

#include "common.hpp"
#include "trees/batching_bplus.hpp"
#include "trees/bplus.hpp"
#include "trees/btree.hpp"

static void BM_lower_bound(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    std::mt19937 rng(12345);
    int min_val = data.empty() ? 0 : data.front();
    int max_val = data.empty() ? 100 : data.back();
    std::uniform_int_distribution<int> dist(min_val, max_val);

    for (auto _ : state) {
        int query = dist(rng);
        benchmark::DoNotOptimize(std::lower_bound(data.begin(), data.end(), query));
        benchmark::ClobberMemory();
    }

    state.counters["time_per_query"] = benchmark::Counter(
        state.iterations(), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

static void BM_btree(benchmark::State& state) {
    const size_t power = state.range(0);
    const size_t elements = (1 << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    btree tree(data);

    std::mt19937 rng(12345);
    int min_val = data.empty() ? 0 : data.front();
    int max_val = data.empty() ? 100 : data.back();
    std::uniform_int_distribution<int> dist(min_val, max_val);

    for (auto _ : state) {
        int query = dist(rng);
        benchmark::DoNotOptimize(tree.lower_bound(query));
        benchmark::ClobberMemory();
    }

    state.counters["time_per_query"] = benchmark::Counter(
        state.iterations(), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

template <size_t power>
static void BM_bplus_helper(benchmark::State& state) {
    constexpr size_t elements = (1ULL << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());

    bplus<elements> tree(data);

    std::mt19937 rng(12345);
    int min_val = data.empty() ? 0 : data.front();
    int max_val = data.empty() ? 100 : data.back();
    std::uniform_int_distribution<int> dist(min_val, max_val);

    for (auto _ : state) {
        int query = dist(rng);
        benchmark::DoNotOptimize(tree.lower_bound(query));
        benchmark::ClobberMemory();
    }

    state.counters["time_per_query"] = benchmark::Counter(
        state.iterations(), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

template <size_t I = 1>
static void BM_bplus(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_bplus_helper<I>(state);
    } else if constexpr (I < 30) {
        BM_bplus<I + 1>(state);
    }
}

template <size_t power, size_t B>
static void BM_batching_bplus_helper(benchmark::State& state) {
    constexpr size_t elements = (1ULL << power) / sizeof(int);

    auto data = generate_random_data(elements);
    std::sort(data.begin(), data.end());
    batching_bplus<elements, B> tree(data);

    std::mt19937 rng(12345);
    int min_val = data.empty() ? 0 : data.front();
    int max_val = data.empty() ? 100 : data.back();
    std::uniform_int_distribution<int> dist(min_val, max_val);

    int batch_queries[B];
    int batch_results[B];

    for (auto _ : state) {
        for (size_t i = 0; i < B; i++) {
            batch_queries[i] = dist(rng);
        }

        tree.lower_bound_batch(batch_queries, batch_results);

        benchmark::DoNotOptimize(batch_results);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * B);
    state.counters["time_per_query"] = benchmark::Counter(
        state.iterations() * B, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

template <size_t I = 1>
static void BM_batching_bplus_4(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_batching_bplus_helper<I, 4>(state);
    } else if constexpr (I < 30) {
        BM_batching_bplus_4<I + 1>(state);
    }
}

template <size_t I = 1>
static void BM_batching_bplus_8(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_batching_bplus_helper<I, 8>(state);
    } else if constexpr (I < 30) {
        BM_batching_bplus_8<I + 1>(state);
    }
}

template <size_t I = 1>
static void BM_batching_bplus_16(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_batching_bplus_helper<I, 16>(state);
    } else if constexpr (I < 30) {
        BM_batching_bplus_16<I + 1>(state);
    }
}

template <size_t I = 1>
static void BM_batching_bplus_32(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_batching_bplus_helper<I, 32>(state);
    } else if constexpr (I < 30) {
        BM_batching_bplus_32<I + 1>(state);
    }
}

template <size_t I = 1>
static void BM_batching_bplus_64(benchmark::State& state) {
    if (state.range(0) == I) {
        BM_batching_bplus_helper<I, 64>(state);
    } else if constexpr (I < 30) {
        BM_batching_bplus_64<I + 1>(state);
    }
}

BENCHMARK(BM_lower_bound)->DenseRange(1, 30);
BENCHMARK(BM_btree)->DenseRange(1, 30);
BENCHMARK(BM_bplus)->DenseRange(1, 30);
BENCHMARK(BM_batching_bplus_4)->DenseRange(1, 30);
BENCHMARK(BM_batching_bplus_8)->DenseRange(1, 30);
BENCHMARK(BM_batching_bplus_16)->DenseRange(1, 30);
BENCHMARK(BM_batching_bplus_32)->DenseRange(1, 30);
BENCHMARK(BM_batching_bplus_64)->DenseRange(1, 30);

BENCHMARK_MAIN();
