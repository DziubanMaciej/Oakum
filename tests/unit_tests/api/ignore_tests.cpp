#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

using OakumIgnoreTest = OakumTest;

TEST_F(OakumIgnoreTest, givenOakumNotInitializedWhenCallingOakumIgnoreFunctionsThenFail) {
    size_t returned{};
    size_t available{};
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumStartIgnore());
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumStopIgnore());
}

TEST_F(OakumIgnoreTest, givenOakumIgnoreIsStartedWhenMemoryIsAllocatedThenItIsNotRecorded) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());

    auto memory0 = allocateMemoryFunction();
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_EQ(nullptr, allocations);
    EXPECT_EQ(0u, allocationCount);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());

    auto memory1 = allocateMemoryFunction();
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumIgnoreTest, givenOakumIgnoreIsCalledMultipleTimesWhenMemoryIsAllocatedThenItIgnoreStateIsRefcounted) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());

    auto memory0 = allocateMemoryFunction(); // Ignore count = 3
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_EQ(nullptr, allocations);
    EXPECT_EQ(0u, allocationCount);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());

    auto memory1 = allocateMemoryFunction(); // Ignore count = 1
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_EQ(nullptr, allocations);
    EXPECT_EQ(0u, allocationCount);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());

    auto memory2 = allocateMemoryFunction(); // Ignore count = 0
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumIgnoreTest, givenOakumStopIgnoreIsCalledWhenIgnoreCountIsZeroThenReturnError) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    EXPECT_EQ(OAKUM_NOT_IGNORING, oakumStopIgnore());

    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());
    EXPECT_OAKUM_SUCCESS(oakumStopIgnore());

    EXPECT_EQ(OAKUM_NOT_IGNORING, oakumStopIgnore());
}
