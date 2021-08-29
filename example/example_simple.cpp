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
    EXPECT_OAKUM_SUCCESS(oakumInit(&initArgs));

    std::cout << "Checking for memory leaks... ";
    if (oakumDetectLeaks() == OAKUM_SUCCESS) {
        std::cout << "No leaks\n";
    } else {
        std::cout << "Leaks detected\n";
    }

    std::cout << "Calling a function leaking memory\n";
    function1();

    std::cout << "Checking for memory leaks... ";
    if (oakumDetectLeaks() == OAKUM_SUCCESS) {
        std::cout << "No leaks\n";
    } else {
        std::cout << "Leaks detected\n";
    }

    std::cout << "Cleaning up library resources\n";
    EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
}
