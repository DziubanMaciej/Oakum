#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
struct OakumInitArgs {
    bool trackStackTraces;
};

using OakumAllocationIdType = uint64_t;

#define OAKUM_MAX_SYMBOL_NAME_SIZE 100
#define OAKUM_MAX_FILE_NAME_SIZE 200

struct OakumStackFrame {
    void *address;
    char symbolName[OAKUM_MAX_SYMBOL_NAME_SIZE];
    char fileName[OAKUM_MAX_FILE_NAME_SIZE];
    unsigned int fileLine;
};

struct OakumAllocation {
    OakumAllocationIdType allocationId;
    size_t size;
    void *pointer;
    bool noThrow;
    OakumStackFrame *stackFrames;
    size_t stackFramesCount;
};

enum OakumResult {
    OAKUM_SUCCESS,
    OAKUM_ALREADY_INITIALIZED,
    OAKUM_UNINITIALIZED,
    OAKUM_NOT_IGNORING,
    OAKUM_INVALID_VALUE,
    OAKUM_LEAKS_DETECTED,
    OAKUM_CANNOT_GET_STACK_TRACE,
};

OakumResult oakumInit(const OakumInitArgs *args);
OakumResult oakumDeinit(bool requireNoLeaks);

OakumResult oakumGetAllocations(OakumAllocation *outAllocations, size_t allocationsCount, size_t *outAllocationsReturned, size_t *outAllocationsAvailable);
OakumResult oakumGetStackTrace(OakumAllocation *allocation);

OakumResult oakumStartIgnore();
OakumResult oakumStopIgnore();
}
