#include "tests/common/fixtures.h"

struct OakumGetAllocationsTest : OakumTest {
    void validateStackFrames(OakumAllocation &allocation) {
        EXPECT_GE(allocation.stackFramesCount, 0u);

        for (size_t stackFrameIndex = 0; stackFrameIndex < allocation.stackFramesCount; stackFrameIndex++) {
            const OakumStackFrame &frame = allocation.stackFrames[stackFrameIndex];
            EXPECT_NE(nullptr, frame.address);
            EXPECT_EQ(nullptr, frame.symbolName);
            EXPECT_EQ(nullptr, frame.fileName);
            EXPECT_EQ(0u, frame.fileLine);
        }
    }

    bool areAllocationsSorted(OakumAllocation *allocations, size_t count) {
        OakumAllocationIdType previousId = 0u;
        for (size_t allocationIndex = 0u; allocationIndex < count; allocationIndex++) {
            if (previousId >= allocations[allocationIndex].allocationId) {
                return false;
            }
            previousId = allocations[allocationIndex].allocationId;
        }
        return true;
    }
};

TEST_F(OakumGetAllocationsTest, givenOakumNotInitializedWhenCallingOakumGetAllocationsThenFail) {
    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;

    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumGetAllocations(&allocations, &allocationCount));

    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));

    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumGetAllocations(&allocations, &allocationCount));
}

TEST_F(OakumGetAllocationsTest, givenNoAllocationsWhenCallingOakumGetAllocationsThenReturnNoAllocations) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;

    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_EQ(nullptr, allocations);
    EXPECT_EQ(0u, allocationCount);
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumGetAllocationsTest, givenIllegalNullArgumentsWhenCallingOakumGetAllocationsThenReturnInvalidValue) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;

    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(&allocations, nullptr));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(nullptr, &allocationCount));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(nullptr, nullptr));
}

TEST_F(OakumGetAllocationsTest, givenSortAllocationsWhenCallingOakumGetAllocationsThenReturnSortedAllocations) {
    initArgs.sortAllocations = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    char *a = new char;
    int *b = new int;
    int *c = new int[12];

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(3u, allocationCount);
    EXPECT_TRUE(areAllocationsSorted(allocations, allocationCount));

    EXPECT_EQ(1u, allocations[0].allocationId);
    EXPECT_EQ(sizeof(char), allocations[0].size);
    EXPECT_EQ(a, allocations[0].pointer);
    EXPECT_FALSE(allocations[0].noThrow);
    validateStackFrames(allocations[0]);

    EXPECT_EQ(2u, allocations[1].allocationId);
    EXPECT_EQ(sizeof(int), allocations[1].size);
    EXPECT_EQ(b, allocations[1].pointer);
    EXPECT_FALSE(allocations[1].noThrow);
    validateStackFrames(allocations[1]);

    EXPECT_EQ(3u, allocations[2].allocationId);
    EXPECT_EQ(12 * sizeof(int), allocations[2].size);
    EXPECT_EQ(c, allocations[2].pointer);
    EXPECT_FALSE(allocations[2].noThrow);
    validateStackFrames(allocations[2]);

    delete a;
    delete b;
    delete[] c;
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumGetAllocationsTest, givenSortAllocationsAndNoThrowNewWhenCallingOakumGetAllocationsThenReturnSortedAllocations) {
    initArgs.sortAllocations = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    char *a = new (std::nothrow) char;
    int *b = new (std::nothrow) int;
    int *c = new (std::nothrow) int[12];

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(3u, allocationCount);
    EXPECT_TRUE(areAllocationsSorted(allocations, allocationCount));

    EXPECT_EQ(1u, allocations[0].allocationId);
    EXPECT_EQ(sizeof(char), allocations[0].size);
    EXPECT_EQ(a, allocations[0].pointer);
    EXPECT_TRUE(allocations[0].noThrow);
    validateStackFrames(allocations[0]);

    EXPECT_EQ(2u, allocations[1].allocationId);
    EXPECT_EQ(sizeof(int), allocations[1].size);
    EXPECT_EQ(b, allocations[1].pointer);
    EXPECT_TRUE(allocations[1].noThrow);
    validateStackFrames(allocations[1]);

    EXPECT_EQ(3u, allocations[2].allocationId);
    EXPECT_EQ(12 * sizeof(int), allocations[2].size);
    EXPECT_EQ(c, allocations[2].pointer);
    EXPECT_TRUE(allocations[2].noThrow);
    validateStackFrames(allocations[2]);

    delete a;
    delete b;
    delete[] c;
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumGetAllocationsTest, givenNoStackTracesWhenCallingOakumGetAllocationsThenReturnThemWithoutStackTraces) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    char *memory = new char;

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(1u, allocations[0].allocationId);
    EXPECT_EQ(sizeof(char), allocations[0].size);
    EXPECT_EQ(memory, allocations[0].pointer);
    EXPECT_FALSE(allocations[0].noThrow);
    EXPECT_EQ(0u, allocations[0].stackFramesCount);

    delete memory;

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumGetAllocationsTest, givenStackTracesWhenCallingOakumGetAllocationsThenReturnThemWithStackTraces) {
    initArgs.trackStackTraces = true;
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    char *memory = new char;

    OakumAllocation *allocations = nullptr;
    size_t allocationCount = 0u;
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationCount));
    EXPECT_NE(nullptr, allocations);
    EXPECT_EQ(1u, allocationCount);

    EXPECT_EQ(1u, allocations[0].allocationId);
    EXPECT_EQ(sizeof(char), allocations[0].size);
    EXPECT_EQ(memory, allocations[0].pointer);
    EXPECT_FALSE(allocations[0].noThrow);
    EXPECT_LT(0u, allocations[0].stackFramesCount);
    for (size_t i = 0u; i < allocations[0].stackFramesCount; i++) {
        EXPECT_NE(nullptr, allocations[0].stackFrames[i].address);
    }

    delete memory;

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationCount));
}

TEST_F(OakumGetAllocationsTest, givenOakumNotInitializedWhenCallingOakumDetectLeaksThenFail) {
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumReleaseAllocations(nullptr, 0u));
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
}
