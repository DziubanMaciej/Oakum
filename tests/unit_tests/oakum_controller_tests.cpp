#include "source/oakum_controller.h"
#include "tests/common/fixtures.h"

#include <gtest/gtest.h>

struct OakumControllerWhitebox : Oakum::OakumController {
    using OakumController::getAllocationsLock;

    OakumControllerWhitebox(const OakumInitArgs &args) : OakumController(args) {}
};

using OakumControllerTest = OakumTest;

TEST_F(OakumControllerTest, givenThreadUnsafeOakumWhenAcquiringLockThenItIsNotLocked) {
    OakumInitArgs args{};
    args.threadSafe = false;
    OakumControllerWhitebox oakum{args};
    EXPECT_FALSE(oakum.getAllocationsLock().owns_lock());
}

TEST_F(OakumControllerTest, givenThreadSafeOakumWhenAcquiringLockThenItIsLocked) {
    OakumInitArgs args{};
    args.threadSafe = true;
    OakumControllerWhitebox oakum{args};
    EXPECT_TRUE(oakum.getAllocationsLock().owns_lock());
}
