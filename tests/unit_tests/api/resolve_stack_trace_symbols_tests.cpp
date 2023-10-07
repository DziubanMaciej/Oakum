#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"
#include "tests/unit_tests/mock_syscalls.h"

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

template <bool expectedSupport>
struct OakumResolveStackTraceSymbolsSupportedTestBase : OakumTest {
    void SetUp() override {
        OakumInitArgs initArgsLocal{};
        initArgsLocal.trackStackTraces = true;
        EXPECT_OAKUM_SUCCESS(oakumInit(&initArgsLocal));
        bool sourceLocationsSupported = isSymbolLocationResolvingSupported();
        EXPECT_OAKUM_SUCCESS(oakumDeinit(true));
        if (sourceLocationsSupported != expectedSupport) {
            GTEST_SKIP();
        }
    }
};

using OakumResolveStackTraceSymbolsTest = OakumTest;
using OakumResolveStackTraceSymbolsSupportedTest = OakumResolveStackTraceSymbolsSupportedTestBase<true>;
using OakumResolveStackTraceSymbolsUnsupportedTest = OakumResolveStackTraceSymbolsSupportedTestBase<false>;

TEST_F(OakumResolveStackTraceSymbolsTest, givenStackTracesEnabledAndFallbackProvidedWhenResolvingStackTraceSymbolsThenNoSymbolIsNullptr) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSymbolName = "<fallback>";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    oakumResolveStackTraceSymbols(allocations, allocationCount);
    OakumAllocation &allocation = allocations[0];
    for (size_t i = 0; i < allocation.stackFramesCount; i++) {
        OakumStackFrame &frame = allocation.stackFrames[i];
        EXPECT_NE(nullptr, frame.symbolName);
    }

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenOakumNotInitializedWhenCallingOakumResolveStackTraceSymbolsThenFail) {
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumResolveStackTraceSymbols(nullptr, 0u));
}

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenNullArgumentsWhenCallingOakumResolveStackTraceSymbolsThenReturnCorrectValues) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSymbols(reinterpret_cast<OakumAllocation *>(0x1234), 0u));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSymbols(nullptr, 1u));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSymbols(nullptr, 0u));
}

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsResolvingSuccessWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsResolvingFailWhenOakumResolveStackTraceSymbolsIsCalledThenReturnError) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsResolvingToNullptrWhenOakumResolveStackTraceSymbolsIsCalledThenReturnError) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsResolvingSuccessAndFallbackSymbolIsPassWhenOakumResolveStackTraceSymbolsIsCalledThenUseResolvedSymbol) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSymbolName = "fallbackSymbol";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsResolvingFailAndFallbackSymbolIsPassWhenOakumResolveStackTraceSymbolsIsCalledThenUseFallbackSymbol) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSymbolName = "fallbackSymbol";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingFail();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("fallbackSymbol", allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsResolvingToNullptrAndFallbackSymbolIsPassWhenOakumResolveStackTraceSymbolsIsCalledThenUseFallbackSymbol) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSymbolName = "fallbackSymbol";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    RaiiSyscallsBackup backup = MockSyscalls::mockSymbolResolvingToNullptr();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("fallbackSymbol", allocations[0].stackFrames[i].symbolName);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSymbolsAlreadyResolvedWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenSourceLocationsResolvedWhenOakumResolveStackTraceSymbolsIsCalledThenReturnCorrectStackTrace) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSymbolsSupportedTest, givenStackTracesNotEnabledWhenCallingResolveStackTraceSymbolsThenReturnFeatureUnsupported) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_FEATURE_NOT_SUPPORTED, oakumResolveStackTraceSymbols(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsUnsupportedTest, givenStackTracesEnabledButSymbolsUnsupportedWhenResolvingSymbolsThenReturnResolvingFailed) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_RESOLVING_FAILED, oakumResolveStackTraceSymbols(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSymbolsUnsupportedTest, givenStackTracesEnabledAndFallbackProvidedButSymbolsUnsupportedWhenResolvingSymbolsThenReturnSuccess) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSymbolName = "<fallback>";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}
