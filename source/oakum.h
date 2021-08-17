#pragma once

#include "oakum/oakum_api.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace Oakum {

class OakumController {
public:
    static void initialize();
    static void deinitialize();
    static bool isInitialized();
    static OakumController *getInstance();

    static void *allocateMemory(std::size_t size, bool noThrow);
    static void deallocateMemory(void *pointer);

    void getAllocations(OakumAllocation *&outAllocations, size_t &outAllocationsCount);
    void releaseAllocation(OakumAllocation &allocation);
    bool hasAllocations();

    bool getStackTrace(OakumAllocation &allocation);

    void incrementIgnoreRefcount();
    bool decrementIgnoreRefcount();

private:
    void registerAllocation(OakumAllocation info);
    void registerDeallocation(void *pointer);
    bool getIgnoreState();

    OakumController() = default;
    static inline std::unique_ptr<OakumController> instance = {};
    static inline thread_local size_t ignoreRefcount = false;

    std::atomic<OakumAllocationIdType> allocationIdCounter = {};
    std::recursive_mutex allocationsLock = {};
    std::unordered_map<void *, OakumAllocation> allocations = {};
};

} // namespace Oakum
