#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

using OakumDetectLeaksTest = OakumTest;

TEST_F(OakumDetectLeaksTest, givenLeaksWhenCallingOakumDetectLeaksThenReportLeaks) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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
