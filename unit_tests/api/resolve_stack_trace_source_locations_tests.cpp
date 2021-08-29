#include "unit_tests/allocate_memory_function.h"
#include "unit_tests/fixtures.h"
#include "unit_tests/mock_syscalls.h"

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

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenSymbolsResolvingSuccessWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSourceLocationResolvingSuccess("myFile", 19);

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("myFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(19, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenSymbolsResolvingFailWhenOakumResolveStackTraceSourceLocationsIsCalledThenError) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSourceLocationResolvingFail();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_RESOLVING_FAILED, oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

using OakumResolveStackTraceSourceLocationsWithFallbackStringsTest = OakumTestWithFallbackStrings;

TEST_F(OakumResolveStackTraceSourceLocationsWithFallbackStringsTest, givenSymbolsResolvingSuccessAndFallbackFileIsPassedWhenOakumResolveStackTraceSourceLocationsIsCalledThenUseResolvedFile) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSourceLocationResolvingSuccess("myFile", 19);

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("myFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(19, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsWithFallbackStringsTest, givenSymbolsResolvingFailAndFallbackFileIsPassedWhenOakumResolveStackTraceSourceLocationsIsCalledThenUseFallbackFile) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSourceLocationResolvingFail();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ(fallbackSourceFileName, allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(0, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenSourceLocationsAlreadyResolvedWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    RaiiSyscallsBackup backup1 = MockSyscalls::mockSourceLocationResolvingSuccess("myFile", 19);

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    // First resolve source locations, they should be called "myFile"
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("myFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(19, allocations[0].stackFrames[i].fileLine);
    }

    // Check that resolve is not called again internally. Source locations should not change.
    RaiiSyscallsBackup backup2 = MockSyscalls::mockSourceLocationResolvingSuccess("myFile2", 23);
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("myFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(19, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenSymbolsResolvedWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    RaiiSyscallsBackup backup1 = MockSyscalls::mockSourceLocationResolvingSuccess("myFile", 19);
    RaiiSyscallsBackup backup2 = MockSyscalls::mockSymbolResolvingSuccess("mySymbol");

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("mySymbol", allocations[0].stackFrames[i].symbolName);
        EXPECT_STREQ("myFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(19, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

using OakumResolveStackTraceSourceLocationsWithoutStackTracesTest = OakumTestWithoutStackTraces;

TEST_F(OakumResolveStackTraceSourceLocationsWithoutStackTracesTest, givenNoStackTracesWhenCallingResolveStackTraceSymbolsThenReturnFeatureUnsupported) {
    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_FEATURE_NOT_SUPPORTED, oakumResolveStackTraceSourceLocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}
