#include "oakum/oakum_api.h"

#include <gtest/gtest.h>

#define EXPECT_OAKUM_SUCCESS(expr) EXPECT_EQ(OAKUM_SUCCESS, (expr))

class OakumGtestEventListener : public ::testing::EmptyTestEventListener {
public:
    OakumGtestEventListener(bool captureStackTraces)
        : captureStackTraces(captureStackTraces) {}

    void OnTestStart(const ::testing::TestInfo &test_info) override {
        OakumInitArgs args{};
        if (captureStackTraces) {
            args.trackStackTraces = true;
            args.fallbackSourceFileName = "<unknown_file>";
            args.fallbackSymbolName = "<unknown_symbol>";
        }
        EXPECT_OAKUM_SUCCESS(oakumInit(&args));
    }

    void OnTestEnd(const ::testing::TestInfo &test_info) override {
        if (oakumDetectLeaks() != OAKUM_SUCCESS) {
            GTEST_NONFATAL_FAILURE_("Memory leaks detected!");
            if (captureStackTraces) {
                OakumAllocation *allocations{};
                size_t allocationsCount{};
                EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationsCount));
                EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationsCount));
                EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationsCount));
                for (size_t allocationIndex = 0; allocationIndex < allocationsCount; allocationIndex++) {
                    std::cout << "  id=" << allocations[allocationIndex].allocationId << ", size=" << allocations[allocationIndex].size << '\n';
                    for (size_t stackFrameIndex = 0u; stackFrameIndex < allocations[allocationIndex].stackFramesCount; stackFrameIndex++) {
                        OakumStackFrame &frame = allocations[allocationIndex].stackFrames[stackFrameIndex];
                        std::cout << "    " << frame.symbolName << " in file " << frame.fileName << ":" << frame.fileLine << "\n";
                    }
                }
                EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationsCount));
            }
        }

        EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    }

private:
    const bool captureStackTraces;
};

TEST(Test, GoodTest1) {
    EXPECT_EQ(2, 1 + 1);
}

TEST(Test, GoodTest2) {
    EXPECT_EQ(3, 2 + 1);
}

TEST(Test, LeakingTest) {
    new int;
    EXPECT_EQ(3, 2 + 1);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new OakumGtestEventListener(true));

    return RUN_ALL_TESTS();
}
