#include "unit_tests/fixtures.h"

using OakumDetectLeaksTest = OakumTest;

TEST_F(OakumDetectLeaksTest, givenOakumNotInitializedWhenCallingOakumDetectLeaksThenFail) {
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumReleaseAllocations(nullptr, 0u));
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
}

TEST_F(OakumDetectLeaksTest, givenLeaksWhenCallingOakumDetectLeaksThenReportLeaks) {
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());

    char *memory0 = new char;
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    char *memory1 = new char;
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    delete memory0;
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    delete memory1;
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
}
