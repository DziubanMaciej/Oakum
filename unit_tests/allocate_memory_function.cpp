#include "unit_tests/allocate_memory_function.h"

#include <iostream>

std::unique_ptr<char[]> allocateMemoryFunction2(size_t size) {
    // intentional comment
    // do not remove
    return std::unique_ptr<char[]>(new char[size]());
}

std::unique_ptr<char[]> allocateMemoryFunction1(size_t size) {
    return allocateMemoryFunction2(size);
}

std::unique_ptr<char[]> allocateMemoryFunction(size_t size) {
    return allocateMemoryFunction1(size);
}

std::unique_ptr<char[]> allocateMemoryFunctionNoThrow2(size_t size) {
    // intentional comment
    // do not remove
    return std::unique_ptr<char[]>(new (std::nothrow) char[size]);
}

std::unique_ptr<char[]> allocateMemoryFunctionNoThrow1(size_t size) {
    return allocateMemoryFunctionNoThrow2(size);
}

std::unique_ptr<char[]> allocateMemoryFunctionNoThrow(size_t size) {
    return allocateMemoryFunctionNoThrow1(size);
}
