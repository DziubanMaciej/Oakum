#include "unit_tests/dummy_functions.h"

std::unique_ptr<char> dummyFunctionC() {
    // intentional comment
    // do not remove
    return std::unique_ptr<char>(std::make_unique<char>());
}

std::unique_ptr<char> dummyFunctionB() {
    return dummyFunctionC();
}

std::unique_ptr<char> dummyFunctionA() {
    return dummyFunctionB();
}

const char *getDummyFunctionsFilename() {
    return __FILE__;
}
