#include "tests/common/allocate_memory_function.h"

/// This file defines a simple function allocating memory and a couple of functions calling it and creating a stacktrace.
/// This can be used for Oakum library testing purposes. On top of that, line numbers, function names and file name of
/// the functions are stored in const globals and can be used to build expectations on what should be returned by the
/// symbol resolving logic.
///
/// Unfortunately, exact querying of line numbers in runtime for a given instruction address cannot be done precisely,
/// since it hardly depends on compiler logic and optimizations - it can differ by a few lines. Hence, this file only
/// provides a line range for each function, not the exact line number.

constexpr static size_t _line_level2_begin = __LINE__ + 1;
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction2(size_t size) {
    // intentional comment
    // do not remove
    return std::unique_ptr<char[]>(new char[size]());
}
constexpr static size_t _line_level2_end = __LINE__ - 1;

constexpr static size_t _line_level1_begin = __LINE__ + 1;
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction1(size_t size) {
    return allocateMemoryFunction2(size);
}
constexpr static size_t _line_level1_end = __LINE__ - 1;

constexpr static size_t _line_level0_begin = __LINE__ + 1;
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction(size_t size) {
    return allocateMemoryFunction1(size);
}
constexpr static size_t _line_level0_end = __LINE__ - 1;

NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow2(size_t size) {
    // intentional comment
    // do not remove
    return std::unique_ptr<char[]>(new (std::nothrow) char[size]);
}

NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow1(size_t size) {
    return allocateMemoryFunctionNoThrow2(size);
}

NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow(size_t size) {
    return allocateMemoryFunctionNoThrow1(size);
}

const char *allocateMemoryFunctionFile = __FILE__;
const size_t allocateMemoryFunctionDepth = 3;
const char *allocateMemoryFunctionNames[] = {
    "allocateMemoryFunction2",
    "allocateMemoryFunction1",
    "allocateMemoryFunction",
};
const size_t allocateMemoryFunctionBeginLines[] = {
    _line_level2_begin,
    _line_level1_begin,
    _line_level0_begin,
};
const size_t allocateMemoryFunctionEndLines[] = {
    _line_level2_end,
    _line_level1_end,
    _line_level0_end,
};
