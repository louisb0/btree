#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
#include <vector>

#include "common.hpp"
#include "trees/batching_bplus.hpp"
#include "trees/bplus.hpp"
#include "trees/btree.hpp"

constexpr size_t num_queries = (1 << 28);
constexpr size_t num_elements = (1 << 28) / sizeof(int);

std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> dist;

std::vector<int> read_or_generate(const char* filename, size_t count, bool should_sort = false) {
    std::vector<int> data;
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        data.resize(count);
        file.read(reinterpret_cast<char*>(data.data()), count * sizeof(int));
        file.close();
    } else {
        printf("WARNING: Generating data at %s. This will interfere with the profile.\n", filename);
        data = generate_random_data(count);
        if (should_sort) {
            std::sort(data.begin(), data.end());
        }
        std::ofstream outfile(filename, std::ios::binary);
        outfile.write(reinterpret_cast<const char*>(data.data()), count * sizeof(int));
        outfile.close();
    }
    return data;
}

void setup_cpu_affinity() {
    if (nice(-20) == -1) {
        perror("nice()");
        exit(EXIT_FAILURE);
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np()");
        exit(EXIT_FAILURE);
    }
}

int profile_batching_bplus(const std::vector<int>& data) {
    constexpr int batch_size = 64;

    batching_bplus<num_elements, batch_size> tree(data);
    printf("Profile begins: batching_bplus\n");

    int sink = 0;

    for (size_t i = 0; i < num_queries; i += batch_size) {
        int batch_queries[batch_size];
        int batch_results[batch_size];

        for (size_t j = 0; j < batch_size; j++) {
            batch_queries[j] = dist(rng);
        }

        tree.lower_bound_batch(batch_queries, batch_results);

        for (size_t j = 0; j < batch_size; j++) {
            sink += batch_results[j];
        }
    }

    return sink;
}

int profile_bplus(const std::vector<int>& data) {
    bplus<num_elements> tree(data);
    printf("Profile begins: bplus\n");

    int sink = 0;

    for (size_t i = 0; i < num_queries; i++) {
        sink += tree.lower_bound(dist(rng));
    }

    return sink;
}

int profile_btree(const std::vector<int>& data) {
    btree tree(data);
    printf("Profile begins: btree\n");

    int sink = 0;

    for (size_t i = 0; i < num_queries; i++) {
        sink += tree.lower_bound(dist(rng));
    }

    return sink;
}

int profile_lower_bound(const std::vector<int>& data) {
    printf("Profile begins: std::lower_bound\n");

    int sink = 0;

    for (size_t i = 0; i < num_queries; i++) {
        sink += *std::lower_bound(data.begin(), data.end(), dist(rng));
    }

    return sink;
}

int main() {
    setup_cpu_affinity();

    auto data = read_or_generate("data/data.bin", num_elements, true);

    int min_val = *std::min_element(data.begin(), data.end());
    int max_val = *std::max_element(data.begin(), data.end());
    dist = std::uniform_int_distribution<int>(min_val, max_val);

    printf("Data prepared (%zu elements).\n", data.size());

    printf("Result: %d\n", profile_batching_bplus(data));
    return EXIT_SUCCESS;
}
