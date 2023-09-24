#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

using OakumReleaseAllocationsTest = OakumTest;

TEST_F(OakumReleaseAllocationsTest, givenOakumNotInitializedWhenCallingOakumReleaseAllocationsThenFail) {
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumReleaseAllocations(nullptr, 0u));
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
}

TEST_F(OakumReleaseAllocationsTest, givenNullArgumentsWhenCallingOakumReleaseAllocationsThenReturnCorrectValues) {
    OakumAllocation *allocations = reinterpret_cast<OakumAllocation *>(0x1234);
    size_t allocationCount = 1u;

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumReleaseAllocations(allocations, 0u));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumReleaseAllocations(nullptr, allocationCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumReleaseAllocations(nullptr, 0u));
}

TEST_F(OakumReleaseAllocationsTest, givenAllocationWhenCallingOakumReleaseAllocationsThenItFixesMemoryLeak) {
    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    memory.reset();

    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
}
