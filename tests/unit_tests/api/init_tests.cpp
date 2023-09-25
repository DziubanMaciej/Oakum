#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

TEST(OakumInitTest, givenOakumInitAndDeinitCalledWhenEnvironmentIsCleanThenSuccessIsReturned) {
    OakumInitArgs defaultInitArgs{};
    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));

    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}

TEST(OakumInitTest, givenOakumInitCalledMultipleTimesWhenEnvironmentIsCleanThenOnlyTheFirstOneSucceeds) {
    OakumInitArgs defaultInitArgs{};
    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_EQ(OAKUM_ALREADY_INITIALIZED, oakumInit(nullptr));
    EXPECT_EQ(OAKUM_ALREADY_INITIALIZED, oakumInit(nullptr));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}

TEST(OakumInitTest, givenOakumDeinitCalledWhenOakumIsNotInitializedThenFail) {
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumDeinit(false));
}

TEST(OakumInitTest, givenLeaksDetectionEnabledWhenOakumDeinitIsCalledThenDetectLeaks) {
    OakumInitArgs defaultInitArgs{};
    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(true));

    EXPECT_OAKUM_SUCCESS(oakumInit(&defaultInitArgs));
    auto memory = allocateMemoryFunction();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDeinit(true));
    memory.reset();
    EXPECT_OAKUM_SUCCESS(oakumDeinit(true));
}
