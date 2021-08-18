#include "fixtures.h"

using OakumInitTest = OakumTestBase;

TEST_F(OakumInitTest, givenOakumInitAndDeinitCalledWhenEnvironmentIsCleanThenSuccessIsReturned) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));

    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}

TEST_F(OakumInitTest, givenOakumInitCalledMultipleTimesWhenEnvironmentIsCleanThenOnlyTheFirstOneSucceeds) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_EQ(OAKUM_ALREADY_INITIALIZED, oakumInit(nullptr));
    EXPECT_EQ(OAKUM_ALREADY_INITIALIZED, oakumInit(nullptr));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}

TEST_F(OakumInitTest, givenOakumDeinitCalledWhenOakumIsNotInitializedThenFail) {
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumDeinit(false));
}

TEST_F(OakumInitTest, givenLeaksDetectionEnabledWhenOakumDeinitIsCalledThenDetectLeaks) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(true));

    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    int *a = new int;
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDeinit(true));
    delete a;
    EXPECT_OAKUM_SUCCESS(oakumDeinit(true));
}
