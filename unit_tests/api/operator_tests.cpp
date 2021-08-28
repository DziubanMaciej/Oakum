#include "unit_tests/fixtures.h"

struct OakumOperatorTest : OakumTest {
    size_t getHugeMemorySize() {
        return std::numeric_limits<size_t>::max() / 2;
    }
};

TEST_F(OakumOperatorTest, givenNoThrowNotSpecifiedWhenMemoryAllocationFailsThenThrowBadAlloc) {
    EXPECT_THROW(new char[getHugeMemorySize()], std::bad_alloc);
    EXPECT_EQ(0u, new (std::nothrow) char[getHugeMemorySize()]);
}
