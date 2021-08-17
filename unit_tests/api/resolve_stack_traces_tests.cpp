#include "dummy_functions.h"

#include "api/fixtures.h"

using OakumResolveStackTracesTest = OakumTest;

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

TEST_F(OakumResolveStackTracesTest, givenOakumNotInitializedWhenCallingOakumResolveStackTracesThenFail) {
    size_t returned{};
    size_t available{};
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumResolveStackTraces(nullptr, 0u));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumStopIgnore());
    EXPECT_OAKUM_SUCCESS(oakumInit(nullptr));
}

TEST_F(OakumResolveStackTracesTest, givenIllegalNullArgumentsWhenCallingOakumResolveStackTracesThenReturnCorrectValues) {
    OakumAllocation *allocations = reinterpret_cast<OakumAllocation *>(0x1234);
    size_t allocationCount = 1u;

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraces(allocations, 0u));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumResolveStackTraces(nullptr, allocationCount));
    EXPECT_EQ(OAKUM_SUCCESS, oakumResolveStackTraces(nullptr, 0u));
}

TEST_F(OakumResolveStackTracesTest, givenDummyFunctionsCalledWhenOakumResolveStackTracesIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraces(allocations, allocationCount));
    OakumAllocation &allocation = allocations[0];

    EXPECT_STR_CONTAINS("operator new", allocation.stackFrames[0].symbolName);

    EXPECT_STR_CONTAINS("make_unique", allocation.stackFrames[1].symbolName);

    EXPECT_STR_CONTAINS("dummyFunctionC", allocation.stackFrames[2].symbolName);
    EXPECT_STREQ(getDummyFunctionsFilename(), allocation.stackFrames[2].fileName);
    EXPECT_EQ(6, allocation.stackFrames[2].fileLine);

    EXPECT_STR_CONTAINS("dummyFunctionB", allocation.stackFrames[3].symbolName);
    EXPECT_STREQ(getDummyFunctionsFilename(), allocation.stackFrames[3].fileName);
    EXPECT_EQ(10, allocation.stackFrames[3].fileLine);

    EXPECT_STR_CONTAINS("dummyFunctionA", allocation.stackFrames[4].symbolName);
    EXPECT_STREQ(getDummyFunctionsFilename(), allocation.stackFrames[4].fileName);
    EXPECT_EQ(14, allocation.stackFrames[4].fileLine);

    EXPECT_STREQ(__FILE__, allocation.stackFrames[5].fileName);

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}