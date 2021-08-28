#include "source/error.h"
#include "source/oakum_controller.h"
#include "source/stack_trace.h"

struct RaiiOakumIgnore {
    RaiiOakumIgnore() {
        Oakum::OakumController::getInstance()->incrementIgnoreRefcount();
    }
    ~RaiiOakumIgnore() {
        FATAL_ERROR_IF(!Oakum::OakumController::getInstance()->decrementIgnoreRefcount(), "Cannot decrement ignore refcount");
    }
};

namespace Oakum {
OakumController::OakumController(const OakumInitArgs &initArgs) : capabilities(createCapabilities(initArgs)) {}

OakumCapabilities OakumController::createCapabilities(const OakumInitArgs &initArgs) {
    OakumCapabilities capabilities{};
    capabilities.supportStackTraces = initArgs.trackStackTraces;
    capabilities.supportStackTracesSourceLocations = initArgs.trackStackTraces && StackTraceHelper::supportsSourceLocations();
    capabilities.threadSafe = initArgs.threadSafe;
    return capabilities;
}

void OakumController::initialize(const OakumInitArgs &initArgs) {
    DEBUG_ERROR_IF(isInitialized(), "Multiple Oakum initialization");
    instance.reset(new OakumController(initArgs));
}

void OakumController::deinitialize() {
    DEBUG_ERROR_IF(!isInitialized(), "Oakum uninitialized");
    instance.reset();
}

bool OakumController::isInitialized() {
    return instance != nullptr;
}

OakumController *OakumController::getInstance() {
    DEBUG_ERROR_IF(!isInitialized(), "Oakum uninitialized");
    return instance.get();
}

void *OakumController::allocateMemory(std::size_t size, bool noThrow) {
    OakumAllocation info{};
    info.size = size;
    info.pointer = ::malloc(size);
    info.noThrow = noThrow;
    info.stackFramesCount = 0;

    const bool success = info.pointer != nullptr;
    if (success && isInitialized()) {
        OakumController &oakum = *getInstance();
        if (!getInstance()->getIgnoreState()) {
            const auto lock = oakum.getAllocationsLock();
            oakum.registerAllocation(info);
        }
    }

    if (!success) {
        if (noThrow) {
            return nullptr;
        } else {
            throw std::bad_alloc{};
        }
    }

    return info.pointer;
}

void OakumController::deallocateMemory(void *pointer) {
    if (pointer == nullptr) {
        return;
    }

    if (isInitialized()) {
        OakumController &oakum = *getInstance();
        if (!oakum.getIgnoreState()) {
            const auto lock = oakum.getAllocationsLock();
            oakum.registerDeallocation(pointer);
        }
    }
}

void OakumController::OakumController::registerAllocation(OakumAllocation info) {
    info.allocationId = this->allocationIdCounter++;
    StackTraceHelper::initializeFrames(info.stackFrames, info.stackFramesCount);
    if (capabilities.supportStackTraces) {
        StackTraceHelper::captureFrames(info.stackFrames, info.stackFramesCount);
    }

    FATAL_ERROR_IF(this->allocations.find(info.pointer) != this->allocations.end(), "Pointer already registered");

    RaiiOakumIgnore raiiIgnore{};
    this->allocations.insert({info.pointer, info});
}

void OakumController::OakumController::registerDeallocation(void *pointer) {
    FATAL_ERROR_IF(pointer == nullptr, "Null pointer registration");

    auto allocation = this->allocations.find(pointer);
    if (allocation != this->allocations.end()) {
        RaiiOakumIgnore raiiIgnore{};
        this->allocations.erase(allocation);
    }
}

void OakumController::getAllocations(OakumAllocation *&outAllocations, size_t &outAllocationsCount) {
    const auto lock = getAllocationsLock();

    outAllocationsCount = this->allocations.size();
    if (outAllocationsCount > 0) {
        outAllocations = new OakumAllocation[outAllocationsCount];

        size_t dstIndex = 0u;
        for (auto srcIterator = this->allocations.begin(); srcIterator != this->allocations.end(); srcIterator++) {
            if (srcIterator->first == outAllocations) {
                continue; // We allocated storage for OakumAllocations and we have to skip it here
            }

            outAllocations[dstIndex] = srcIterator->second;
            dstIndex++;
        }
        DEBUG_ERROR_IF(dstIndex != outAllocationsCount);
    } else {
        outAllocations = nullptr;
    }
}

void OakumController::releaseAllocation(OakumAllocation &allocation) {
    for (size_t stackFrameIndex = 0; stackFrameIndex < allocation.stackFramesCount; stackFrameIndex++) {
        delete[] allocation.stackFrames[stackFrameIndex].symbolName;
        delete[] allocation.stackFrames[stackFrameIndex].fileName;
    }
    delete &allocation;
}

bool OakumController::hasAllocations() {
    const auto lock = getAllocationsLock();
    return this->allocations.size();
}

bool OakumController::resolveStackTraceSymbols(OakumAllocation &allocation) {
    DEBUG_ERROR_IF(!this->capabilities.supportStackTraces);
    if (allocation.stackFramesCount != 0 && allocation.stackFrames[0].symbolName == nullptr) {
        return StackTraceHelper::resolveSymbols(allocation.stackFrames, allocation.stackFramesCount);
    }
    return true;
}

bool OakumController::resolveStackTraceSourceLocations(OakumAllocation &allocation) {
    DEBUG_ERROR_IF(!this->capabilities.supportStackTraces);
    DEBUG_ERROR_IF(!this->capabilities.supportStackTracesSourceLocations);
    if (allocation.stackFramesCount != 0 && allocation.stackFrames[0].fileName == nullptr) {
        return StackTraceHelper::resolveSourceLocations(allocation.stackFrames, allocation.stackFramesCount);
    }
    return true;
}

void OakumController::incrementIgnoreRefcount() {
    ignoreRefcount++;
}

bool OakumController::decrementIgnoreRefcount() {
    if (ignoreRefcount == 0) {
        return false;
    }
    ignoreRefcount--;
    return true;
}

bool OakumController::getIgnoreState() {
    return ignoreRefcount > 0;
}

} // namespace Oakum
