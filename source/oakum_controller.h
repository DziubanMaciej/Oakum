#pragma once

#include "source/include/oakum/oakum_api.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace Oakum {

class OakumController {
public:
    static void initialize(const OakumInitArgs &initArgs);
    static void deinitialize();
    static bool isInitialized();
    static OakumController *getInstance();

    const OakumCapabilities &getCapabilities() { return capabilities; }

    static void *allocateMemory(std::size_t size, bool noThrow);
    static void deallocateMemory(void *pointer);

    void getAllocations(OakumAllocation *&outAllocations, size_t &outAllocationsCount);
    void releaseAllocation(OakumAllocation &allocation);
    bool hasAllocations();

    bool resolveStackTraceSymbols(OakumAllocation &allocation);
    bool resolveStackTraceSourceLocations(OakumAllocation &allocation);

    void incrementIgnoreRefcount();
    bool decrementIgnoreRefcount();

protected:
    static OakumCapabilities createCapabilities(const OakumInitArgs &initArgs);
    static std::optional<std::string> createOptionalString(const char *str);
    void registerAllocation(OakumAllocation info);
    void registerDeallocation(void *pointer);
    bool getIgnoreState();

    auto getAllocationsLock() {
        std::unique_lock lock{allocationsLock, std::defer_lock};
        if (capabilities.threadSafe) {
            lock.lock();
        }
        return lock;
    }

    OakumController(const OakumInitArgs &initArgs);

private:
    static inline std::unique_ptr<OakumController> instance = {};
    static inline thread_local size_t ignoreRefcount = false;

    const OakumCapabilities capabilities;
    const std::optional<std::string> fallbackSymbolName = {};
    const std::optional<std::string> fallbackSourceFileName = {};
    const bool sortAllocations = {};

    std::atomic<OakumAllocationIdType> allocationIdCounter = 1;
    std::recursive_mutex allocationsLock = {};
    std::unordered_map<void *, OakumAllocation> allocations = {};
};

} // namespace Oakum
