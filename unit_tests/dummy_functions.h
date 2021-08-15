#pragma once

#include <memory>

inline std::unique_ptr<char> dummyFunctionC() {
    return std::make_unique<char>();
}

inline std::unique_ptr<char> dummyFunctionB() {
    return dummyFunctionC();
}

inline std::unique_ptr<char> dummyFunctionA() {
    return dummyFunctionB();
}

inline const char *getDummyFunctionsFilename() {
    return __FILE__;
}
