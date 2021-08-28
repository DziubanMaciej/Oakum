#pragma once

#include "source/error.h"

#include <cstring>

#define FATAL_ERROR_ON_FAILED_SYSCALL(expression)                                 \
    if ((expression) < 0) {                                                       \
        FATAL_ERROR("Syscall error on \"", #expression, "\", ", strerror(errno)); \
    }
