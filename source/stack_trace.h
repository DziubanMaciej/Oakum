#pragma once

#include <cstddef>
#include <optional>
#include <string>

struct OakumStackFrame;

namespace Oakum {
struct StackTraceHelper {
    StackTraceHelper() = delete;
    static bool supportsSourceLocations();
    static void initializeFrames(OakumStackFrame *frames, size_t &framesCount);
    static void captureFrames(OakumStackFrame *frames, size_t &framesCount);

    static bool resolveSymbols(OakumStackFrame *frames, size_t framesCount, const std::optional<std::string> &fallbackSymbolName);
    static bool resolveSourceLocations(OakumStackFrame *frames, size_t framesCount, const std::optional<std::string> &fallbackSourceFileName);

    static void setupString(char *&destination, const char *source);
private:
    constexpr static inline unsigned int skippedFrames = 3;
};
} // namespace Oakum
