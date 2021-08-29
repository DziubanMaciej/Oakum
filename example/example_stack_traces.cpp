#include "oakum/oakum_api.h"

#include <iostream>

inline void function3() {
    auto field1 = new int();
    auto field2 = new int[34];
    auto field3 = new char();
}

inline void function2() {
    function3();
}

inline void function1() {
    function2();
}

#define EXPECT_OAKUM_SUCCESS(expr)                                                                 \
    {                                                                                              \
        const auto result = (expr);                                                                \
        if (result != OAKUM_SUCCESS) {                                                             \
            std::cout << "Error: Oakum API error in \"" << #expr << "\" error=" << result << '\n'; \
            exit(1);                                                                               \
        }                                                                                          \
    }

int main() {
    std::cout << "Initializing Oakum library\n";
    OakumInitArgs initArgs{};
    initArgs.trackStackTraces = true;
    initArgs.threadSafe = false;
    initArgs.fallbackSymbolName = "<unknown_symbol>";
    initArgs.fallbackSourceFileName = "<unknown_file>";
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    std::cout << "Calling a function leaking memory\n";
    function1();

    std::cout << "Getting detailed leaks information\n";
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
            std::cout << " in file " << frame.fileName << ":" << frame.fileLine;
            std::cout << "\n";
        }
        std::cout << '\n';
    }

    std::cout << "Cleaning up library resources\n";
    EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationsCount));
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}
