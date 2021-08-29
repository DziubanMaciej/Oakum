#pragma once

#include <memory>

#ifdef __clang__
#define NO_INLINE_1 [[gnu::noinline]]
#elif __GNUC__
#define NO_INLINE_1 [[gnu::noinline]]
#elif _MSC_VER
#define NO_INLINE_1
#else
#define NO_INLINE_1
#endif

NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction2(size_t size = 1);
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction1(size_t size = 1);
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction(size_t size = 1);

NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow2(size_t size = 1);
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow1(size_t size = 1);
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow(size_t size = 1);
