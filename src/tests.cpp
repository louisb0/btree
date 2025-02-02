#include <gtest/gtest.h>

#include <algorithm>

#include "common.hpp"
#include "trees/bplus.hpp"
#include "trees/btree.hpp"

class tree_test : public ::testing::Test {
protected:
    static constexpr size_t n = 10000;
    std::vector<int> data;
    std::vector<int> queries;

    void SetUp() override {
        queries = generate_random_data(n);
        data = generate_random_data(n);
        std::sort(data.begin(), data.end());
    }
};

TEST_F(tree_test, btree) {
    btree tree(data);

    for (size_t i = 0; i < n; i++) {
        int tree_result = tree.lower_bound(queries[i]);
        auto std_result = std::lower_bound(data.begin(), data.end(), queries[i]);

        if (std_result != data.end()) {
            EXPECT_EQ(tree_result, *std_result)
                << "Mismatch at index " << i << ", query value: " << queries[i]
                << ", tree returned: " << tree_result << ", expected: " << *std_result;
        }
    }
}

TEST_F(tree_test, bplus) {
    bplus<n> tree(data);

    for (size_t i = 0; i < n; i++) {
        int tree_result = tree.lower_bound(queries[i]);
        auto std_result = std::lower_bound(data.begin(), data.end(), queries[i]);

        if (std_result != data.end()) {
            EXPECT_EQ(tree_result, *std_result)
                << "Mismatch at index " << i << ", query value: " << queries[i]
                << ", tree returned: " << tree_result << ", expected: " << *std_result;
        }
    }
}
