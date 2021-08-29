#include "oakum/oakum_api.h"

#include <iostream>

class LeakingObject {
public:
    LeakingObject() {
        field1 = new int();
        field2 = new int[34];
        field3 = new char();
    }

private:
    int *field1{};
    int *field2{};
    char *field3{};
};

#define EXPECT_OAKUM_SUCCESS(expr)                                                                 \
    {                                                                                              \
        const auto result = (expr);                                                                \
        if (result != OAKUM_SUCCESS) {                                                             \
            std::cout << "Error: Oakum API error in \"" << #expr << "\, error=" << result << '\n'; \
            exit(1);                                                                               \
        }                                                                                          \
    }

inline void function3() {
    LeakingObject object{};
}

inline void function2() {
    function3();
}

inline void function1() {
    function2();
}

int main() {
    OakumInitArgs initArgs{};
    initArgs.trackStackTraces = true;
    initArgs.threadSafe = false;

    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));
    function1();

    OakumAllocation *allocations{};
    size_t allocationsCount{};
    EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationsCount));
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, allocationsCount));
    EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, allocationsCount));

    std::cout << "Detected " << allocationsCount << " leaks\n";
    for (size_t i = 0; i < allocationsCount; i++) {
        OakumAllocation &allocation = allocations[i];

        std::cout << "  id=" << allocation.allocationId << ", size=" << allocation.size << ", capturedStackFrames=" << allocation.stackFramesCount << '\n';
        for (size_t stackFrameIndex = 0u; stackFrameIndex < allocation.stackFramesCount; stackFrameIndex++) {
            OakumStackFrame &frame = allocation.stackFrames[stackFrameIndex];
            std::cout << "    " << frame.symbolName;
            if (frame.fileName) {
                std::cout << " in file " << frame.fileName << ":" << frame.fileLine;
            }
            std::cout << "\n";
        }
        std::cout << '\n';
    }

    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationsCount));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}
