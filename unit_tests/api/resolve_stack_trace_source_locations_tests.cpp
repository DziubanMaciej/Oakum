#include "dummy_functions.h"
#include "fixtures.h"

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

struct OakumResolveStackTraceSourceLocationsTest : OakumTest {
    void SetUp() override {
        OakumTest::SetUp();

        OakumCapabilities capabilities{};
        EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));
        if (!capabilities.supportStackTracesSourceLocations) {
            RaiiOakumIgnore ignore;
            GTEST_SKIP();
        }
    }

    void validateSourceLocations(OakumAllocation &allocation) {
        EXPECT_STREQ(getDummyFunctionsFilename(), allocation.stackFrames[2].fileName);
        EXPECT_EQ(6, allocation.stackFrames[2].fileLine);

        EXPECT_STREQ(getDummyFunctionsFilename(), allocation.stackFrames[3].fileName);
        EXPECT_EQ(10, allocation.stackFrames[3].fileLine);

        EXPECT_STREQ(getDummyFunctionsFilename(), allocation.stackFrames[4].fileName);
        EXPECT_EQ(14, allocation.stackFrames[4].fileLine);

        EXPECT_STREQ(__FILE__, allocation.stackFrames[5].fileName);
    }
};

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenOakumNotInitializedWhenCallingOakumResolveStackTraceSourceLocationsThenFail) {
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumResolveStackTraceSourceLocations(nullptr, 0u));
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenNullArgumentsWhenCallingOakumResolveStackTraceSourceLocationsThenReturnCorrectValues) {
    OakumAllocation *allocations = reinterpret_cast<OakumAllocation *>(0x1234);
    size_t allocationCount = 1u;

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSourceLocations(allocations, 0u));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSourceLocations(nullptr, allocationCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSourceLocations(nullptr, 0u));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenDummyFunctionsCalledWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    validateSourceLocations(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenSourceLocationsAlreadyResolvedWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    validateSourceLocations(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    validateSourceLocations(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenSymbolsResolvedWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    validateSourceLocations(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

using OakumResolveStackTraceSourceLocationsWithoutStackTracesTest = OakumTestWithoutStackTraces;

TEST_F(OakumResolveStackTraceSourceLocationsWithoutStackTracesTest, givenNoStackTracesWhenCallingResolveStackTraceSymbolsThenReturnFeatureUnsupported) {
    char *memory = new char;

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_FEATURE_NOT_SUPPORTED, oakumResolveStackTraceSourceLocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
    delete memory;
}
