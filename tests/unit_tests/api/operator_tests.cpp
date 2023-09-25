#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

using OakumOperatorTest = OakumTest;

TEST_F(OakumOperatorTest, givenFailedMallocWhenAllocatingMemoryThenFailAccordingly) {
    const size_t hugeMemorySize = std::numeric_limits<size_t>::max() / 3; // this should not be possible to allocate (recheck in year 2050)

    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_THROW(new char[hugeMemorySize](), std::bad_alloc);
    EXPECT_EQ(nullptr, new (std::nothrow) char[hugeMemorySize]());
}

TEST_F(OakumOperatorTest, givenNullptrWhenCallingDeleteThenNoErrorsAreThrown) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    int *ptr = nullptr;
    delete ptr;
    delete[] ptr;
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
}

TEST_F(OakumOperatorTest, givenSucceededMallocWhenAllocatingMemoryThenSaveCorrectAllocationMetadata) {
    initArgs.sortAllocations = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    // Test array and non-array new
    // Test throw and non-throw new
    int *mem0 = new int();
    int *mem1 = new int[2]();
    int *mem2 = new (std::nothrow) int();
    int *mem3 = new (std::nothrow) int[2]();

    // Validate metadata
    // We don't have a field for isArray... Probably not useful?
    OakumAllocation *allocations{};
    size_t allocationsCount{};
    EXPECT_EQ(OAKUM_SUCCESS, oakumGetAllocations(&allocations, &allocationsCount));
    EXPECT_FALSE(allocations[0].noThrow);
    EXPECT_FALSE(allocations[1].noThrow);
    EXPECT_TRUE(allocations[2].noThrow);
    EXPECT_TRUE(allocations[3].noThrow);
    EXPECT_EQ(OAKUM_SUCCESS, oakumReleaseAllocations(allocations, allocationsCount));

    // Test array and non-array delete
    // All deletes should be sized
    delete mem0;
    delete[] mem1;
    delete mem2;
    delete[] mem3;

}
