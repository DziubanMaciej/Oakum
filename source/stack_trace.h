#pragma once

#include <cstddef>

struct OakumStackFrame;

struct StackTraceHelper {
    StackTraceHelper() = delete;
    static void initializeFrames(OakumStackFrame *frames, size_t &framesCount);
    static void captureFrames(OakumStackFrame *frames, size_t &framesCount);
    static bool resolveFrames(OakumStackFrame *frames, size_t framesCount);

    constexpr static inline unsigned int skippedFrames = 3;
};
