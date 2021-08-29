#pragma once

#include "oakum/oakum_api.h"

#include <algorithm>
#include <gtest/gtest.h>

#define EXPECT_OAKUM_SUCCESS(expr) EXPECT_EQ(OAKUM_SUCCESS, (expr))

const inline OakumInitArgs defaultInitArgs{
    true,    // trackStackTraces
    true,    // threadSafe
    nullptr, // fallbackSymbolName
    nullptr, //fallbackSourceFileName
};

struct OakumTest : ::testing::Test {
    void SetUp() override {
        EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    }

    void TearDown() override {
        if (oakumDetectLeaks() != OAKUM_SUCCESS) {
            OakumAllocation *allocations{};
            size_t allocationsCount = 0;
            EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationsCount));
            // EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationsCount));
            EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationsCount));
            EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
        }
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

struct OakumTestWithFallbackStrings : OakumTest {
    void SetUp() override {
        initArgs.fallbackSymbolName = fallbackSymbolName;
        initArgs.fallbackSourceFileName = fallbackSourceFileName;
        OakumTest::SetUp();
    }

    const char *fallbackSymbolName = "<symbol>";
    const char *fallbackSourceFileName = "<fileName>";
};

struct RaiiOakumIgnore {
    RaiiOakumIgnore() {
        EXPECT_OAKUM_SUCCESS(oakumStartIgnore());
    }
    ~RaiiOakumIgnore() {
        EXPECT_OAKUM_SUCCESS(oakumStopIgnore());
    }
};
