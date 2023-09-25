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

NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunction(size_t size = 1);
NO_INLINE_1 std::unique_ptr<char[]> allocateMemoryFunctionNoThrow(size_t size = 1);

extern const char *allocateMemoryFunctionFile;
extern const size_t allocateMemoryFunctionDepth;
extern const char *allocateMemoryFunctionNames[];
extern const size_t allocateMemoryFunctionBeginLines[];
extern const size_t allocateMemoryFunctionEndLines[];
