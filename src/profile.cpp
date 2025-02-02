#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "common.hpp"
#include "trees/bplus.hpp"
#include "trees/btree.hpp"

constexpr size_t num_queries = (1 << 28);
constexpr size_t num_elements = (1 << 28) / sizeof(int);

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

int main() {
    auto queries = read_or_generate("data/queries.bin", num_queries);
    auto data = read_or_generate("data/data.bin", num_elements, true);

    bplus<num_elements> tree(data);

    printf("Data prepared, profile begins.\n");

    int sink = 0;
    size_t query = 0;
    for (size_t i = 0; i < num_queries; i++) {
        sink += tree.lower_bound(queries[query++ & (num_queries - 1)]);
    }

    printf("%d\n", sink);
}
