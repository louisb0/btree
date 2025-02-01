#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "common.hpp"
#include "trees/btree.hpp"

constexpr size_t num_queries = (1 << 28);
constexpr size_t num_elements = (1 << 28) / sizeof(int);

template <typename tree_type>
void profiling() {
    auto queries = generate_random_data(num_queries);
    size_t query_idx = 0;

    auto data = generate_random_data(num_elements);
    std::sort(data.begin(), data.end());

    tree_type tree(data);
    printf("Done preparing\n");

    int sink = 0;
    for (size_t i = 0; i < num_queries; i++) {
        int query = queries[query_idx++ & (num_queries - 1)];
        sink += tree.lower_bound(query);
    }

    printf("%d\n", sink);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("./%s <tree_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string filter = argv[1];
    if (filter.find("btree") != std::string::npos) {
        profiling<btree>();
    }

    return EXIT_SUCCESS;
}
