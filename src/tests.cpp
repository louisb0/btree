#include <gtest/gtest.h>

#include <algorithm>

#include "common.hpp"
#include "trees/batching_bplus.hpp"
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

TEST_F(tree_test, bplus_batch) {
    constexpr size_t batch_size = 16;
    batching_bplus<n, batch_size> tree(data);

    for (size_t batch = 0; batch < n; batch += batch_size) {
        int batch_queries[batch_size];
        int batch_results[batch_size];

        size_t current_batch_size = std::min(batch_size, n - batch);
        for (size_t i = 0; i < current_batch_size; i++) {
            batch_queries[i] = queries[batch + i];
        }

        tree.lower_bound_batch(batch_queries, batch_results);

        for (size_t i = 0; i < current_batch_size; i++) {
            auto result = std::lower_bound(data.begin(), data.end(), batch_queries[i]);
            if (result != data.end()) {
                EXPECT_EQ(batch_results[i], *result)
                    << "Mismatch in batch " << batch / batch_size << " at index " << i
                    << ", query value: " << batch_queries[i]
                    << ", batch tree returned: " << batch_results[i] << ", expected: " << *result;
            }
        }
    }
}
