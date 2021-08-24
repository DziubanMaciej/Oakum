#pragma once

#include <cstddef>

struct OakumStackFrame;

struct StackTraceHelper {
    StackTraceHelper() = delete;
    static bool supportsSourceLocations();
    static void initializeFrames(OakumStackFrame *frames, size_t &framesCount);
    static void captureFrames(OakumStackFrame *frames, size_t &framesCount);

    static bool resolveSymbols(OakumStackFrame *frames, size_t framesCount);
    static bool resolveSourceLocations(OakumStackFrame *frames, size_t framesCount);

    constexpr static inline unsigned int skippedFrames = 3;
};
