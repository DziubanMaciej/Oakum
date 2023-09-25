#include "tests/common/allocate_memory_function.h"
#include "tests/common/fixtures.h"

#include <atomic>
#include <thread>

using OakumDetectLeaksTest = OakumTest;

TEST_F(OakumDetectLeaksTest, givenLeaksWhenCallingOakumDetectLeaksThenReportLeaks) {
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());

    auto memory0 = allocateMemoryFunction();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    auto memory1 = allocateMemoryFunction();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    memory0.reset();
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    memory1.reset();
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
}

TEST_F(OakumDetectLeaksTest, givenLeaksInDifferentThreadWhenCallingOakumDetectLeaksThenReportLeaks) {
    std::atomic_uint32_t sharedFlag = 0;
    auto waitFlag = [&sharedFlag](uint32_t value) { while(sharedFlag != value){} };
    auto notifyFlag = [&sharedFlag](uint32_t value) { sharedFlag = value; };

    std::thread thread{[&waitFlag, &notifyFlag]() {
        // Leak memory
        waitFlag(1);
        auto memory = allocateMemoryFunction();
        notifyFlag(2);

        // Free memory
        waitFlag(3);
        memory.reset();
        notifyFlag(4);

        // Finish
        waitFlag(5);
        notifyFlag(6);
    }};

    // Initialize Oakum
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());

    // Notify thread, so it leaks memory
    notifyFlag(1);
    waitFlag(2);
    EXPECT_EQ(OAKUM_LEAKS_DETECTED, oakumDetectLeaks());

    // Notify thread, so it frees memory
    notifyFlag(3);
    waitFlag(4);
    EXPECT_OAKUM_SUCCESS(oakumDetectLeaks());
    notifyFlag(5);

    // Notify thread, so it finishes
    notifyFlag(5);
    waitFlag(6);
    thread.join();
}
