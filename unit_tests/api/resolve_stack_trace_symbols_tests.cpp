#include "dummy_functions.h"
#include "fixtures.h"

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

struct OakumResolveStackTraceSymbolsTest : OakumTest {
    void validateSymbols(OakumAllocation &allocation) {
        ASSERT_NE(nullptr, allocation.stackFrames[0].symbolName);

        EXPECT_STR_CONTAINS("operator new", allocation.stackFrames[0].symbolName);
        EXPECT_STR_CONTAINS("make_unique", allocation.stackFrames[1].symbolName);
        EXPECT_STR_CONTAINS("dummyFunctionC", allocation.stackFrames[2].symbolName);
        EXPECT_STR_CONTAINS("dummyFunctionB", allocation.stackFrames[3].symbolName);
        EXPECT_STR_CONTAINS("dummyFunctionA", allocation.stackFrames[4].symbolName);
    }
};

TEST_F(OakumResolveStackTraceSymbolsTest, givenOakumNotInitializedWhenCallingOakumResolveStackTraceSymbolsThenFail) {
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumResolveStackTraceSymbols(nullptr, 0u));
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenNullArgumentsWhenCallingOakumResolveStackTraceSymbolsThenReturnCorrectValues) {
    OakumAllocation *allocations = reinterpret_cast<OakumAllocation *>(0x1234);
    size_t allocationCount = 1u;

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSymbols(allocations, 0u));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSymbols(nullptr, allocationCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSymbols(nullptr, 0u));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenDummyFunctionsCalledWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    validateSymbols(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenSymbolsAlreadyResolvedWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    validateSymbols(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    validateSymbols(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenSourceLocationsResolvedWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    OakumCapabilities capabilities{};
    EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));
    if (!capabilities.supportStackTracesSourceLocations) {
        RaiiOakumIgnore ignore;
        GTEST_SKIP();
    }

    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    validateSymbols(allocations[0]);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

using OakumResolveStackTraceSymbolsWithoutStackTracesTest = OakumTestWithoutStackTraces;

TEST_F(OakumResolveStackTraceSymbolsWithoutStackTracesTest, givenNoStackTracesWhenCallingResolveStackTraceSymbolsThenReturnFeatureUnsupported) {
    char *memory = new char;

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_FEATURE_NOT_SUPPORTED, oakumResolveStackTraceSymbols(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
    delete memory;
}