#include "unit_tests/allocate_memory_function.h"
#include "unit_tests/fixtures.h"

struct OakumOperatorTest : OakumTest {
    size_t getHugeMemorySize() {
        return std::numeric_limits<size_t>::max() / 2;
    }
};

TEST_F(OakumOperatorTest, givenNoThrowNotSpecifiedWhenMemoryAllocationFailsThenThrowBadAlloc) {
    EXPECT_THROW(allocateMemoryFunction(getHugeMemorySize()), std::bad_alloc);
    EXPECT_EQ(nullptr, allocateMemoryFunctionNoThrow(getHugeMemorySize()));
}
