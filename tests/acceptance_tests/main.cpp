#include "gtest/gtest.h"
#include "oakum/oakum_api.h"
#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

using AcceptanceTest = OakumTest;

#ifndef NDEBUG
using AcceptanceTestDebug = OakumTestWithFallbackStrings;
using AcceptanceTestRelease = SkippedTest;
#else
using AcceptanceTestDebug = SkippedTest;
using AcceptanceTestRelease = OakumTestWithFallbackStrings;
#endif

// TODO: this could be wrong, but I've got no better idea right now...
#ifdef NDEBUG
constexpr bool operatorInlined = true;
#else
constexpr bool operatorInlined = false;
#endif

TEST_F(AcceptanceTest, givenMemoryLeakWhenDetectingLeaksThenReturnTrue) {
    EXPECT_EQ(OAKUM_SUCCESS, oakumDetectLeaks());
    auto memory = allocateMemoryFunction();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());
    memory.reset();
    EXPECT_EQ(OAKUM_SUCCESS, oakumDetectLeaks());
}

TEST_F(AcceptanceTest, givenMemoryLeakWhenGettingAllocationThenReturnOneAllocationWithCorrectMetadata) {
    OakumAllocation *allocations{};
    size_t allocationsCount{};
    EXPECT_EQ(OAKUM_SUCCESS, oakumGetAllocations(&allocations, &allocationsCount));
    EXPECT_EQ(0u, allocationsCount);
    EXPECT_EQ(nullptr, allocations);

    auto memory = allocateMemoryFunction(13);

    EXPECT_EQ(OAKUM_SUCCESS, oakumGetAllocations(&allocations, &allocationsCount));
    ASSERT_EQ(1u, allocationsCount);
    ASSERT_NE(nullptr, allocations);
    OakumAllocation &allocation = allocations[0];
    EXPECT_EQ(1, allocation.allocationId);
    EXPECT_FALSE(allocation.noThrow);
    EXPECT_EQ(memory.get(), allocation.pointer);
    EXPECT_EQ(13, allocation.size);

    memory.reset();

    EXPECT_EQ(OAKUM_SUCCESS, oakumReleaseAllocations(allocations, allocationsCount));
}

TEST_F(AcceptanceTest, givenDebugConfigResolvingSymbolsThenReturnCorrectSymbols) {
    auto memory = allocateMemoryFunction(13);

    OakumAllocation *allocations{};
    size_t allocationsCount{};
    EXPECT_EQ(OAKUM_SUCCESS, oakumGetAllocations(&allocations, &allocationsCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSymbols(allocations, allocationsCount));
    OakumAllocation &allocation = allocations[0];
    constexpr size_t minimumStackDepth = 6; // 1 main, 1 this test, 3 levels of allocating, 1 operator 'new' - must be at least 6.
    EXPECT_LE(minimumStackDepth, allocation.stackFramesCount);

    // Verify if the latest stack frame before allocating is operator new
    if (!operatorInlined) {
        ASSERT_NE(nullptr, allocation.stackFrames[0].symbolName);
        EXPECT_NE(nullptr, strstr(allocation.stackFrames[0].symbolName, "operator new[]"));
    }

    // Verify next couple levels of allocation inside 'allocateMemoryFunction'
    for (size_t i = 0; i < allocateMemoryFunctionDepth; i++) {
        OakumStackFrame &frame = allocation.stackFrames[i + 1 - int(operatorInlined)]; // start from 1, because 0 is operator new (if not inlined)
        ASSERT_NE(nullptr, frame.symbolName);
        EXPECT_NE(nullptr, strstr(frame.symbolName, allocateMemoryFunctionNames[i]));
    }

    memory.reset();
    EXPECT_EQ(OAKUM_SUCCESS, oakumReleaseAllocations(allocations, allocationsCount));
}

TEST_F(AcceptanceTestDebug, givenDebugConfigWhenResolvingSourceLocationsThenReturnLocations) {
    auto memory = allocateMemoryFunction(13);

    OakumAllocation *allocations{};
    size_t allocationsCount{};
    EXPECT_EQ(OAKUM_SUCCESS, oakumGetAllocations(&allocations, &allocationsCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSymbols(allocations, allocationsCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSourceLocations(allocations, allocationsCount));
    OakumAllocation &allocation = allocations[0];
    constexpr size_t minimumStackDepth = 6; // 1 main, 1 this test, 3 levels of allocating, 1 operator 'new' - must be at least 6.
    EXPECT_LE(minimumStackDepth, allocation.stackFramesCount);

    // Verify couple levels of allocation inside 'allocateMemoryFunction'.
    for (size_t i = 0; i < allocateMemoryFunctionDepth; i++) {
        OakumStackFrame &frame = allocation.stackFrames[i + 1]; // start from 1, because 0 is operator new
        EXPECT_STREQ(allocateMemoryFunctionFile, frame.fileName);
        EXPECT_LE(allocateMemoryFunctionBeginLines[i], frame.fileLine);
        EXPECT_GE(allocateMemoryFunctionEndLines[i], frame.fileLine);
    }

    memory.reset();
    EXPECT_EQ(OAKUM_SUCCESS, oakumReleaseAllocations(allocations, allocationsCount));
}

TEST_F(AcceptanceTestRelease, givenReleaseConfigWhenResolvingSourceLocationsThenReturnFallbackLocations) {
    auto memory = allocateMemoryFunction(13);

    OakumAllocation *allocations{};
    size_t allocationsCount{};
    EXPECT_EQ(OAKUM_SUCCESS, oakumGetAllocations(&allocations, &allocationsCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSymbols(allocations, allocationsCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraceSourceLocations(allocations, allocationsCount));
    OakumAllocation &allocation = allocations[0];
    constexpr size_t minimumStackDepth = 6; // 1 main, 1 this test, 3 levels of allocating, 1 operator 'new' - must be at least 6.
    EXPECT_LE(minimumStackDepth, allocation.stackFramesCount);

    // Verify couple levels of allocation inside 'allocateMemoryFunction'.
    for (size_t i = 0; i < allocateMemoryFunctionDepth; i++) {
        OakumStackFrame &frame = allocation.stackFrames[i];
        EXPECT_STREQ(initArgs.fallbackSourceFileName, frame.fileName);
        EXPECT_EQ(0, frame.fileLine);
    }

    memory.reset();
    EXPECT_EQ(OAKUM_SUCCESS, oakumReleaseAllocations(allocations, allocationsCount));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
