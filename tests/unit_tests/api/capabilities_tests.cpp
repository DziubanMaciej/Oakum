#include "tests/common/fixtures.h"

using OakumCapabilitiesTest = OakumTest;

TEST_F(OakumCapabilitiesTest, givenDefaultInitArgsWhenQueryingCapabilitiesThenReturnDefaultCapabilities) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    OakumCapabilities capabilities = {};
    EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));

    EXPECT_FALSE(capabilities.supportStackTraces);
    EXPECT_FALSE(capabilities.supportStackTracesSourceLocations);
    EXPECT_FALSE(capabilities.threadSafe);
}

TEST_F(OakumCapabilitiesTest, givenThreadSafetyRequestedWhenQueryingCapabilitiesThenReturnCapabilitiesWithThreadSafetyEnabled) {
    initArgs.threadSafe = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    OakumCapabilities capabilities = {};
    EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));

    EXPECT_FALSE(capabilities.supportStackTraces);
    EXPECT_FALSE(capabilities.supportStackTracesSourceLocations);
    EXPECT_TRUE(capabilities.threadSafe);
}

TEST_F(OakumCapabilitiesTest, givenStackTracesRequestedWhenQueryingCapabilitiesThenReturnCapabilitiesWithStackTraceTrackingEnabled) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    OakumCapabilities capabilities = {};
    EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));

    EXPECT_TRUE(capabilities.supportStackTraces);
#if OAKUM_SOURCE_LOCATIONS_AVAILABLE
    EXPECT_TRUE(capabilities.supportStackTracesSourceLocations);
#else
    EXPECT_FALSE(capabilities.supportStackTracesSourceLocations);
#endif
    EXPECT_FALSE(capabilities.threadSafe);
}
