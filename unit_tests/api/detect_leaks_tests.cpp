#include "unit_tests/allocate_memory_function.h"
#include "unit_tests/fixtures.h"

using OakumDetectLeaksTest = OakumTest;

TEST_F(OakumDetectLeaksTest, givenOakumNotInitializedWhenCallingOakumDetectLeaksThenFail) {
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumReleaseAllocations(nullptr, 0u));
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
}

TEST_F(OakumDetectLeaksTest, givenLeaksWhenCallingOakumDetectLeaksThenReportLeaks) {
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());

    auto memory0 = allocateMemoryFunction();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    auto memory1 = allocateMemoryFunction();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    memory0.reset();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    memory1.reset();
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
}
