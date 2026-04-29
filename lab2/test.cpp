#include "hashtable.h"
#include "hashtable_impl.h"
#include "test_util.h"
#include "gtest/gtest.h"

/// @brief Test HashTable without lock in single-thread
TEST_P(HashTableSingleThreadTest, NoLock) {
    hashtable = new HashTable();
}

/// @brief Test HashTable with coarse-grained lock in single-thread
TEST_P(HashTableSingleThreadTest, Coarse) {
    hashtable = new CoarseHashTable();
}

/// @brief Test HashTable with fine-grained lock in single-thread
TEST_P(HashTableSingleThreadTest, Fine) {
    hashtable = new FineHashTable();
}

/// @brief HashTableSingleThreadTest Test Case and Parameters
INSTANTIATE_TEST_CASE_P(Default, HashTableSingleThreadTest,
    ::testing::Values(
        // Workload Type, Request Num, Num of Threads
        std::make_tuple(INSERT_ONLY,          1000000, 1),
        std::make_tuple(INSERT_LOOKUP,        1000000, 1),
        std::make_tuple(INSERT_LOOKUP_DELETE, 1000000, 1)
    )
);

/// @brief Test HashTable with coarse-grained lock in multi-thread
TEST_P(HashTableMultiThreadTest, Coarse) {
    hashtable = new CoarseHashTable();
}

/// @brief Test HashTable with fine-grained lock in multi-thread
TEST_P(HashTableMultiThreadTest, Fine) {
    hashtable = new FineHashTable();
}

/// @brief HashTableMultiThreadTest Test Case and Parameters
INSTANTIATE_TEST_CASE_P(Default, HashTableMultiThreadTest,
    ::testing::Values(
        // Workload Type, Request Num, Num of Threads
        std::make_tuple(INSERT_ONLY,          1000000, 2),
        std::make_tuple(INSERT_ONLY,          1000000, 4),
        std::make_tuple(INSERT_ONLY,          1000000, 8),
        std::make_tuple(INSERT_LOOKUP,        1000000, 2),
        std::make_tuple(INSERT_LOOKUP,        1000000, 4),
        std::make_tuple(INSERT_LOOKUP,        1000000, 8),
        std::make_tuple(INSERT_LOOKUP_DELETE, 1000000, 2),
        std::make_tuple(INSERT_LOOKUP_DELETE, 1000000, 4),
        std::make_tuple(INSERT_LOOKUP_DELETE, 1000000, 8)
    )
);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
