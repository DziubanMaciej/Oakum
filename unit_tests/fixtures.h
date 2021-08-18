#pragma once

#include "oakum/oakum_api.h"

#include <algorithm>
#include <gtest/gtest.h>

#define EXPECT_OAKUM_SUCCESS(expr) EXPECT_EQ(OAKUM_SUCCESS, (expr))

const inline OakumInitArgs defaultInitArgs{
    true, // trackStackTraces
    true, // threadSafe
};

struct OakumTest : ::testing::Test {
    void SetUp() {
        EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    }

    void TearDown() override {
        EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
        EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    }

    OakumInitArgs initArgs = defaultInitArgs;
};

struct OakumTestWithoutStackTraces : OakumTest {
    void SetUp() override {
        initArgs.trackStackTraces = false;
        OakumTest::SetUp();
    }
};
