#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "common.hpp"
#include "trees/bplus.hpp"
#include "trees/btree.hpp"

constexpr size_t num_queries = (1 << 28);
constexpr size_t num_elements = (1 << 28) / sizeof(int);

int main() {
    auto queries = generate_random_data(num_queries);
    auto data = generate_random_data(num_elements);
    std::sort(data.begin(), data.end());

    bplus<num_elements> tree(data);

    printf("Data prepared, profile begins.\n");

    int sink = 0;
    size_t query = 0;
    for (size_t i = 0; i < num_queries; i++) {
        sink += tree.lower_bound(queries[query++ & (num_queries - 1)]);
    }

    printf("%d\n", sink);
}
