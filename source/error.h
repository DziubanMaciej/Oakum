#pragma once

#include <iostream>

namespace Oakum {
void performAbort() {
    throw std::exception{};
}

inline void dumpLog(std::ostream &out) {}

template <typename Arg, typename... Args>
inline void dumpLog(std::ostream &out, Arg &&arg, Args &&...args) {
    out << arg;
    dumpLog(out, std::forward<Args>(args)...);
}
} // namespace Oakum

#define FATAL_ERROR(message, ...)                          \
    Oakum::dumpLog(std::cerr, message, __VA_ARGS__, '\n'); \
    Oakum::performAbort();

#define FATAL_ERROR_IF(condition, message, ...) \
    if (condition) {                            \
        FATAL_ERROR(message, __VA_ARGS__);      \
    }

#ifdef _DEBUG
#define DEBUG_ERROR(message, ...) FATAL_ERROR(message, ...)
#define DEBUG_ERROR_IF(condiation, message, ...) FATAL_ERROR_IF(condition, message, ...)
#else
#define DEBUG_ERROR(message, ...)
#define DEBUG_ERROR_IF(condiation, message, ...)
#endif
