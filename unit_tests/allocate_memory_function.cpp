#include "unit_tests/allocate_memory_function.h"

std::unique_ptr<char> allocateMemoryFunction2() {
    // intentional comment
    // do not remove
    return std::unique_ptr<char>(std::make_unique<char>());
}

std::unique_ptr<char> allocateMemoryFunction1() {
    return allocateMemoryFunction2();
}

std::unique_ptr<char> allocateMemoryFunction() {
    return allocateMemoryFunction1();
}

const char *getDummyFunctionsFilename() {
    return __FILE__;
}
