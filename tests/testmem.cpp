/* testmem.cpp The test of Memory for IVM™.
Google Test (Gtest) as Dependency.
Copyright (C) 2026 Aleksandr Silaev.
C/C++/Zig.
IVM TEST ONLY.
*/

#include <gtest/gtest.h>

extern "C" {
    #include <ivm/ivm.h>
    #include <imm/IntMemoryManager.h>
}

#define TEST_ALINGMENT 16

class MemoryTest : public ::testing::Test
{
protected:
};

TEST_F (MemoryTest, BasicAlloc)
{
    size_t size = 128;
    void *ptr = MemoryAllocate (size);

    ASSERT_NE (ptr, nullptr);

    EXPECT_EQ ((uintptr_t)ptr % 16, 0);

    cleanbit (ptr);
}

TEST_F (MemoryTest, CoalesceBlocks)
{
    IntMemoryStatistics stats1, stats2;

    void *a = MemoryAllocate (32);
    void *b = MemoryAllocate (32);
    void *c = MemoryAllocate (32);

    MemoryPullStats (&stats1);
    int initial_blocks = stats1.active_blocks;

    cleanbit (a);
    cleanbit (b);
    cleanbit (c);

    MemoryPullStats (&stats2);

    EXPECT_EQ (stats2.active_blocks, 0);
}

TEST_F (MemoryTest, ReallocExpansion)
{
    char *data = (char *)MemoryAllocate (16);

    strcpy (data, "Hello");

    char *bigdata = (char *)MemoryReAllocate (data, 128);
    
    ASSERT_NE (bigdata, nullptr);
    EXPECT_STREQ (bigdata, "Hello");
    cleanbit (bigdata);
}