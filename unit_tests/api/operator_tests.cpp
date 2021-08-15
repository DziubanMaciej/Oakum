#include "api/fixtures.h"

using OakumOperatorTest = OakumTest;

TEST_F(OakumOperatorTest, givenNoThrowNotSpecifiedWhenMemoryAllocationFailsThenThrowBadAlloc) {
    EXPECT_THROW(new char[getHugeMemorySize()], std::bad_alloc);
    EXPECT_EQ(0u, new (std::nothrow) char[getHugeMemorySize()]);
}
