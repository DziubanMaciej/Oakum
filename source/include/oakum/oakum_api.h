#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
struct OakumInitArgs {
    bool trackStackTraces;
    bool threadSafe;
};

using OakumAllocationIdType = uint64_t;

#ifndef OAKUM_MAX_STACK_FRAMES_COUNT
#define OAKUM_MAX_STACK_FRAMES_COUNT 10
#endif

struct OakumStackFrame {
    void *address;
    char *symbolName;
    char *fileName;
    unsigned int fileLine;
};

struct OakumAllocation {
    OakumAllocationIdType allocationId;
    size_t size;
    void *pointer;
    bool noThrow;
    OakumStackFrame stackFrames[OAKUM_MAX_STACK_FRAMES_COUNT];
    size_t stackFramesCount;
};

enum OakumResult {
    OAKUM_SUCCESS,
    OAKUM_ALREADY_INITIALIZED,
    OAKUM_UNINITIALIZED,
    OAKUM_NOT_IGNORING,
    OAKUM_INVALID_VALUE,
    OAKUM_LEAKS_DETECTED,
    OAKUM_RESOLVING_FAILED,
    OAKUM_FEATURE_NOT_SUPPORTED,
};

OakumResult oakumInit(const OakumInitArgs *args);
OakumResult oakumDeinit(bool requireNoLeaks);

OakumResult oakumDetectLeaks();

OakumResult oakumGetAllocations(OakumAllocation **outAllocations, size_t *outAllocationsCount);
OakumResult oakumReleaseAllocations(OakumAllocation *allocations, size_t allocationsCount);

OakumResult oakumResolveStackTraceSymbols(OakumAllocation *allocations, size_t allocationsCount);
OakumResult oakumResolveStackTraceSourceLocations(OakumAllocation *allocations, size_t allocationsCount);

OakumResult oakumStartIgnore();
OakumResult oakumStopIgnore();
}
