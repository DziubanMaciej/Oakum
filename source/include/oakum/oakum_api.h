#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
struct OakumInitArgs {
    bool trackStackTraces;
};

using OakumAllocationIdType = uint64_t;

struct OakumAllocation {
    OakumAllocationIdType allocationId;
    size_t size;
    void *pointer;
    bool noThrow;
};

enum OakumResult {
    OAKUM_SUCCESS,
    OAKUM_ALREADY_INITIALIZED,
    OAKUM_UNINITIALIZED,
    OAKUM_NOT_IGNORING,
    OAKUM_INVALID_VALUE,
    OAKUM_LEAKS_DETECTED,
};

OakumResult oakumInit(const OakumInitArgs *args);
OakumResult oakumDeinit(bool requireNoLeaks);

OakumResult oakumGetAllocations(OakumAllocation *outAllocations, size_t allocationsCount, size_t *outAllocationsReturned, size_t *outAllocationsAvailable);

OakumResult oakumStartIgnore();
OakumResult oakumStopIgnore();
}
