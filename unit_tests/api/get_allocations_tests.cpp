#include "api/fixtures.h"

using OakumGetAllocationsTest = OakumTest;

TEST_F(OakumGetAllocationsTest, givenOakumNotInitializedWhenCallingOakumGetAllocationsThenFail) {
    size_t returned{};
    size_t available{};
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    EXPECT_EQ(OAKUM_UNINITIALIZED, oakumGetAllocations(nullptr, 0, &returned, &available));
    EXPECT_OAKUM_SUCCESS(oakumInit(nullptr));
}

TEST_F(OakumGetAllocationsTest, givenNoAllocationsWhenCallingOakumGetAllocationsThenReturnNoAllocations) {
    size_t returned{};
    size_t available{};
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(nullptr, 0, &returned, &available));
    EXPECT_EQ(0u, returned);
    EXPECT_EQ(0u, available);
}

TEST_F(OakumGetAllocationsTest, givenNonZeroAllocationsCountAndNullAllocationsPointerPassedWhenCallingOakumGetAllocationsThenReturnInvalidValue) {
    size_t returned{};
    size_t available{};
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(nullptr, 1, &returned, &available));
}

TEST_F(OakumGetAllocationsTest, givenIllegalNullArgumentsWhenCallingOakumGetAllocationsThenReturnInvalidValue) {
    size_t returned{};
    size_t available{};
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(nullptr, 0, &returned, nullptr));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(nullptr, 0, nullptr, &available));
    EXPECT_EQ(OAKUM_INVALID_VALUE, oakumGetAllocations(nullptr, 0, nullptr, nullptr));
}

TEST_F(OakumGetAllocationsTest, givenSomeAllocationsWhenCallingOakumGetAllocationsThenReturnThem) {
    char *a = new char;
    int *b = new int;
    int *c = new int[12];

    size_t returned{};
    size_t available{};
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(nullptr, 0, &returned, &available));
    EXPECT_EQ(0u, returned);
    EXPECT_EQ(3u, available);

    OakumAllocation allocations[3] = {};
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(allocations, 3, &returned, &available));
    EXPECT_EQ(3u, returned);
    EXPECT_EQ(3u, available);

    sortAllocationsById(allocations, 3);
    EXPECT_EQ(0u, allocations[0].allocationId);
    EXPECT_EQ(sizeof(char), allocations[0].size);
    EXPECT_EQ(a, allocations[0].pointer);
    EXPECT_FALSE(allocations[0].noThrow);

    EXPECT_EQ(1u, allocations[1].allocationId);
    EXPECT_EQ(sizeof(int), allocations[1].size);
    EXPECT_EQ(b, allocations[1].pointer);
    EXPECT_FALSE(allocations[1].noThrow);

    EXPECT_EQ(2u, allocations[2].allocationId);
    EXPECT_EQ(12 * sizeof(int), allocations[2].size);
    EXPECT_EQ(c, allocations[2].pointer);
    EXPECT_FALSE(allocations[2].noThrow);

    delete a;
    delete b;
    delete[] c;
}

TEST_F(OakumGetAllocationsTest, givenSomeNoThrowAllocationsWhenCallingOakumGetAllocationsThenReturnThem) {
    char *a = new (std::nothrow) char;
    int *b = new (std::nothrow) int;
    int *c = new (std::nothrow) int[12];

    size_t returned{};
    size_t available{};
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(nullptr, 0, &returned, &available));
    EXPECT_EQ(0u, returned);
    EXPECT_EQ(3u, available);

    OakumAllocation allocations[3] = {};
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(allocations, 3, &returned, &available));
    EXPECT_EQ(3u, returned);
    EXPECT_EQ(3u, available);

    sortAllocationsById(allocations, 3);
    EXPECT_EQ(0u, allocations[0].allocationId);
    EXPECT_EQ(sizeof(char), allocations[0].size);
    EXPECT_EQ(a, allocations[0].pointer);
    EXPECT_TRUE(allocations[0].noThrow);

    EXPECT_EQ(1u, allocations[1].allocationId);
    EXPECT_EQ(sizeof(int), allocations[1].size);
    EXPECT_EQ(b, allocations[1].pointer);
    EXPECT_TRUE(allocations[1].noThrow);

    EXPECT_EQ(2u, allocations[2].allocationId);
    EXPECT_EQ(12 * sizeof(int), allocations[2].size);
    EXPECT_EQ(c, allocations[2].pointer);
    EXPECT_TRUE(allocations[2].noThrow);

    delete a;
    delete b;
    delete[] c;
}
