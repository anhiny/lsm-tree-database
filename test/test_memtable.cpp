#include <gtest/gtest.h>
#include "../include/memtable/memtable.h"

TEST(MemtableTest, BasicOperation) {
    Memtable memtable;

    memtable.put("key1", "value1");
    EXPECT_EQ(memtable.get("key1"), "value1");

    memtable.put("key1", "value2");
    EXPECT_EQ(memtable.get("key1"), "value2");

    memtable.remove("key1");
    EXPECT_EQ(memtable.get("key1").value(), "");
}



int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}