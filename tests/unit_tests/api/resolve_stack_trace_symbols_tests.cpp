#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"
#include "tests/unit_tests/mock_syscalls.h"

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

using OakumResolveStackTraceSymbolsTest = OakumTest;

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

TEST_F(OakumResolveStackTraceSymbolsTest, givenSymbolsResolvingSuccessWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingSuccess("mySymbol");

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("mySymbol", allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenSymbolsResolvingFailWhenOakumResolveStackTraceSymbolsIsCalledThenReturnError) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingFail();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_RESOLVING_FAILED, oakumResolveStackTraceSymbols(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenSymbolsResolvingToNullptrWhenOakumResolveStackTraceSymbolsIsCalledThenReturnError) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingToNullptr();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_RESOLVING_FAILED, oakumResolveStackTraceSymbols(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

using OakumResolveStackTraceSymbolsWithFallbackStringsTest = OakumTestWithFallbackStrings;

TEST_F(OakumResolveStackTraceSymbolsWithFallbackStringsTest, givenSymbolsResolvingSuccessAndFallbackSymbolIsPassWhenOakumResolveStackTraceSymbolsIsCalledThenUseResolvedSymbol) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingSuccess("mySymbol");

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("mySymbol", allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsWithFallbackStringsTest, givenSymbolsResolvingFailAndFallbackSymbolIsPassWhenOakumResolveStackTraceSymbolsIsCalledThenUseFallbackSymbol) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingFail();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ(this->fallbackSymbolName, allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsWithFallbackStringsTest, givenSymbolsResolvingToNullptrAndFallbackSymbolIsPassWhenOakumResolveStackTraceSymbolsIsCalledThenUseFallbackSymbol) {
    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingToNullptr();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ(this->fallbackSymbolName, allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenSymbolsAlreadyResolvedWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    RaiiSyscallsBackup backup1 = MockSyscalls::mockSymbolResolvingSuccess("mySymbol");

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    // First resolve symbols, they should be called "mySymbol"
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("mySymbol", allocations[0].stackFrames[i].symbolName);
    }

    // Check that resolve is not called again internally. Symbols should not change.
    RaiiSyscallsBackup backup2 = MockSyscalls::mockSymbolResolvingSuccess("mySymbol2");
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("mySymbol", allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsTest, givenSourceLocationsResolvedWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    RaiiSyscallsBackup backup1 = MockSyscalls::mockSourceLocationResolvingSuccess("myFile", 10);
    RaiiSyscallsBackup backup2 = MockSyscalls::mockSymbolResolvingSuccess("mySymbol");

    OakumCapabilities capabilities{};
    EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));
    if (!capabilities.supportStackTracesSourceLocations) {
        RaiiOakumIgnore ignore;
        GTEST_SKIP();
    }

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("mySymbol", allocations[0].stackFrames[i].symbolName);
        EXPECT_STREQ("myFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(10, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

using OakumResolveStackTraceSymbolsWithoutStackTracesTest = OakumTestWithoutStackTraces;

TEST_F(OakumResolveStackTraceSymbolsWithoutStackTracesTest, givenNoStackTracesWhenCallingResolveStackTraceSymbolsThenReturnFeatureUnsupported) {
    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_FEATURE_NOT_SUPPORTED, oakumResolveStackTraceSymbols(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}
