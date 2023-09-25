#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

struct OakumOperatorTest : OakumTest {
    size_t getHugeMemorySize() {
        return std::numeric_limits<size_t>::max() / 2;
    }
};

// TODO test all possible operators

TEST_F(OakumOperatorTest, givenNoThrowNotSpecifiedWhenMemoryAllocationFailsThenThrowBadAlloc) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_THROW(allocateMemoryFunction(getHugeMemorySize()), std::bad_alloc);
    EXPECT_EQ(nullptr, allocateMemoryFunctionNoThrow(getHugeMemorySize()));
}
