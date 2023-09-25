#pragma once

#include "oakum/oakum_api.h"

#include <algorithm>
#include <gtest/gtest.h>

#define EXPECT_OAKUM_SUCCESS(expr) EXPECT_EQ(OAKUM_SUCCESS, (expr))

struct OakumTest : ::testing::Test {
    void TearDown() override {
        if (oakumDetectLeaks() == OAKUM_LEAKS_DETECTED) {
            GTEST_NONFATAL_FAILURE_("Leaks detected at the end of test");
#if 1
            // Enable this code to debug the leak
            OakumAllocation *allocations{};
            size_t allocationsCount = 0;
            EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationsCount));
            EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationsCount));
            EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationsCount));
            EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationsCount));
            EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
#endif
        }
        const OakumResult deinitResult = oakumDeinit(false);
        EXPECT_TRUE(deinitResult == OAKUM_SUCCESS || deinitResult == OAKUM_UNINITIALIZED);
    }

    bool isSourceLocationResolvingSupported() {
        OakumCapabilities capabilities = {};
        EXPECT_OAKUM_SUCCESS(oakumGetCapabilities(&capabilities));
        return capabilities.supportStackTracesSourceLocations;
    }

    OakumInitArgs initArgs = {};
};

struct SkippedTest : OakumTest {
    void SetUp() override {
        GTEST_SKIP();
    }
    void TearDown() override {}
};

struct RaiiOakumIgnore {
    RaiiOakumIgnore() {
        EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    }
    ~RaiiOakumIgnore() {
        EXPECT_OAKUM_SUCCESS(oakumStopIgnore());
    }
};
