#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"
#include "tests/unit_tests/mock_syscalls.h"

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

template <bool expectedSupport>
struct OakumResolveStackTraceSourceLocationsSupportedTestBase : OakumTest {
    void SetUp() override {
        OakumInitArgs initArgsLocal{};
        initArgsLocal.trackStackTraces = true;
        EXPECT_OAKUM_SUCCESS(oakumInit(&initArgsLocal));
        bool sourceLocationsSupported = isSourceLocationResolvingSupported();
        EXPECT_OAKUM_SUCCESS(oakumDeinit(true));
        if (sourceLocationsSupported != expectedSupport) {
            GTEST_SKIP();
        }
    }
};

using OakumResolveStackTraceSourceLocationsTest = OakumTest;
using OakumResolveStackTraceSourceLocationsSupportedTest = OakumResolveStackTraceSourceLocationsSupportedTestBase<true>;
using OakumResolveStackTraceSourceLocationsUnsupportedTest = OakumResolveStackTraceSourceLocationsSupportedTestBase<false>;

TEST_F(OakumResolveStackTraceSourceLocationsTest, givenStackTracesEnabledAndFallbackProvidedWhenResolvingSourceLocationsThenNoLocationIsNullptr) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSourceFileName = "<fallback>";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    oakumResolveStackTraceSourceLocations(allocations, allocationCount);
    OakumAllocation &allocation = allocations[0];
    for (size_t i = 0; i < allocation.stackFramesCount; i++) {
        OakumStackFrame &frame = allocation.stackFrames[i];
        EXPECT_NE(nullptr, frame.fileName);
    }

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenOakumNotInitializedWhenCallingOakumResolveStackTraceSourceLocationsThenFail) {
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumResolveStackTraceSourceLocations(nullptr, 0u));
}

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenNullArgumentsWhenCallingOakumResolveStackTraceSourceLocationsThenReturnCorrectValues) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSourceLocations(reinterpret_cast<OakumAllocation *>(0x1234), 0u));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraceSourceLocations(nullptr, 1u));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSourceLocations(nullptr, 0u));
}

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenSymbolsResolvingSuccessWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenSymbolsResolvingFailWhenOakumResolveStackTraceSourceLocationsIsCalledThenError) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenSymbolsResolvingSuccessAndFallbackFileIsPassedWhenOakumResolveStackTraceSourceLocationsIsCalledThenUseResolvedFile) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSourceFileName = "fallbackFile";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenSymbolsResolvingFailAndFallbackFileIsPassedWhenOakumResolveStackTraceSourceLocationsIsCalledThenUseFallbackFile) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSourceFileName = "fallbackFile";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    RaiiSyscallsBackup backup = MockSyscalls::mockSourceLocationResolvingFail();

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));
    for (int i = 0; i < allocations[0].stackFramesCount; i++) {
        EXPECT_STREQ("fallbackFile", allocations[0].stackFrames[i].fileName);
        EXPECT_EQ(0, allocations[0].stackFrames[i].fileLine);
    }
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenSourceLocationsAlreadyResolvedWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSourceFileName = "fallbackFile";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenSymbolsResolvedWhenOakumResolveStackTraceSourceLocationsIsCalledThenReturnCorrectStackTrace) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSourceFileName = "fallbackFile";
    initArgs.fallbackSymbolName = "fallbackSymbol";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

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

TEST_F(OakumResolveStackTraceSourceLocationsSupportedTest, givenStackTracesDisabledWhenResolvingSourceLocationsThenReturnFeatureUnsupported) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_FEATURE_NOT_SUPPORTED, oakumResolveStackTraceSourceLocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsUnsupportedTest, givenStackTracesEnabledButSourceLocationsUnsupportedWhenResolvingSourceLocationsThenReturnResolvingFailed) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(OAKUM_RESOLVING_FAILED, oakumResolveStackTraceSourceLocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumResolveStackTraceSourceLocationsUnsupportedTest, givenStackTracesEnabledAndFallbackProvidedButSourceLocationsUnsupportedWhenResolvingSourceLocationsThenReturnSuccess) {
    initArgs.trackStackTraces = true;
    initArgs.fallbackSourceFileName = "<fallback>";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    auto memory = allocateMemoryFunction();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}
