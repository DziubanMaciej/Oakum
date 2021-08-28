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

NO_INLINE_1 std::unique_ptr<char> dummyFunctionC();
NO_INLINE_1 std::unique_ptr<char> dummyFunctionB();
NO_INLINE_1 std::unique_ptr<char> dummyFunctionA();
const char *getDummyFunctionsFilename();
