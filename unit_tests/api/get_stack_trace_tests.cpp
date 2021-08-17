#include "dummy_functions.h"

#include "api/fixtures.h"

using OakumGetStackTraceTest = OakumTest;

#define EXPECT_STR_CONTAINS(substring, string) EXPECT_NE(nullptr, strstr((string), (substring)))

TEST_F(OakumGetStackTraceTest, givenOakumNotInitializedWhenCallingOakumGetStackTraceThenFail) {
    size_t returned{};
    size_t available{};
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumGetStackTrace(nullptr));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumStopIgnore());
    EXPECT_OAKUM_SUCCESS(oakumInit(nullptr));
}

TEST_F(OakumGetStackTraceTest, givenNullAllocationPasseddWhenCallingOakumGetStackTraceThenFail) {
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetStackTrace(nullptr));
}

TEST_F(OakumGetStackTraceTest, givenDummyFunctionsCalledWhenOakumGetStackTraceIsCalledThenReturnCorrectStackTrace) {
    auto memory = dummyFunctionA();

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);
    OakumAllocation &allocation = allocations[0];

    EXPECT_OAKUM_SUCCESS(oakumGetStackTrace(&allocation));

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
