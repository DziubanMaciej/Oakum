#pragma once

#include "oakum/oakum_api.h"

#include <algorithm>
#include <gtest/gtest.h>

#define EXPECT_OAKUM_SUCCESS(expr) EXPECT_EQ(OAKUM_SUCCESS, (expr))

struct OakumTestBase : ::testing::Test {
    void SetUp(const OakumInitArgs *args) {
        EXPECT_OAKUM_SUCCESS(oakumInit(args));
    }

    void TearDown() override {
        if (OakumResult deinitResult = oakumDeinit(true); deinitResult != OAKUM_SUCCESS) {
            EXPECT_OAKUM_SUCCESS(deinitResult);
            EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
        }
    }

    void sortAllocationsById(OakumAllocation *allocations, size_t count) {
        auto comparator = [](const OakumAllocation &left, const OakumAllocation &right) {
            return left.allocationId < right.allocationId;
        };
        std::sort(allocations, allocations + count, comparator);
    }

    size_t getHugeMemorySize() {
        return std::numeric_limits<size_t>::max() / 2;
    }
};

struct OakumTest : OakumTestBase {
    void SetUp() override {
        OakumTestBase::SetUp(nullptr);
    }
};

struct OakumTestWithoutStackTraces : OakumTestBase {
    void SetUp() override {
        OakumInitArgs args{};
        args.trackStackTraces = false;
        OakumTestBase::SetUp(&args);
    }
};
