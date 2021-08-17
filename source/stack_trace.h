#pragma once

struct OakumStackFrame;

struct StackTraceHelper {
    StackTraceHelper() = delete;
    static void initializeFrames(OakumStackFrame *frames, size_t &framesCount);
    static void captureFrames(OakumStackFrame *frames, size_t &framesCount);
    static bool resolveFrames(OakumStackFrame *frames, size_t framesCount);
};
