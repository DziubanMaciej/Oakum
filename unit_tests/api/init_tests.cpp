#include "fixtures.h"

TEST(OakumInitTest, givenOakumInitAndDeinitCalledWhenEnvironmentIsCleanThenSuccessIsReturned) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));

    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}

TEST(OakumInitTest, givenOakumInitCalledMultipleTimesWhenEnvironmentIsCleanThenOnlyTheFirstOneSucceeds) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_EQ(OAKUM_ALREADY_INITIALIZED, oakumInit(nullptr));
    EXPECT_EQ(OAKUM_ALREADY_INITIALIZED, oakumInit(nullptr));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}

TEST(OakumInitTest, givenOakumDeinitCalledWhenOakumIsNotInitializedThenFail) {
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumDeinit(false));
}

TEST(OakumInitTest, givenLeaksDetectionEnabledWhenOakumDeinitIsCalledThenDetectLeaks) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(true));

    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    int *a = new int;
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDeinit(true));
    delete a;
    EXPECT_OAKUM_SUCCESS(oakumDeinit(true));
}
