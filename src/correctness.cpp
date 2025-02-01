#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "common.hpp"
#include "trees/btree.hpp"

template <typename tree_type>
void correctness() {
    auto queries = generate_random_data(1000);
    auto data = generate_random_data(1000);
    std::sort(data.begin(), data.end());

    tree_type tree(data);

    for (int i = 0; i < 1000; i++) {
        int lower1 = tree.lower_bound(queries[i]);
        auto lower2 = std::lower_bound(data.begin(), data.end(), queries[i]);

        if (lower2 != data.end() && lower1 != *lower2) {
            printf("Mismatch: %d %d\n", lower1, *lower2);
        }
    }

    printf("Completed\n");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("./%s <tree_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string filter = argv[1];
    if (filter.find("btree") != std::string::npos) {
        correctness<btree>();
    }

    return EXIT_SUCCESS;
}
