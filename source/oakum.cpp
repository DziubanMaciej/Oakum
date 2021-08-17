#include "error.h"
#include "oakum.h"
#include "stack_trace.h"

struct RaiiOakumIgnore {
    RaiiOakumIgnore() {
        Oakum::OakumController::getInstance()->incrementIgnoreRefcount();
    }
    ~RaiiOakumIgnore() {
        FATAL_ERROR_IF(!Oakum::OakumController::getInstance()->decrementIgnoreRefcount(), "Cannot decrement ignore refcount");
    }
};

namespace Oakum {
void OakumController::initialize() {
    FATAL_ERROR_IF(isInitialized(), "Multiple Oakum initialization");
    instance.reset(new OakumController());
}

void OakumController::deinitialize() {
    FATAL_ERROR_IF(!isInitialized(), "Oakum uninitialized");
    instance.reset();
}

bool OakumController::isInitialized() {
    return instance != nullptr;
}

OakumController *OakumController::getInstance() {
    FATAL_ERROR_IF(!isInitialized(), "Oakum uninitialized");
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
            std::lock_guard lock{oakum.allocationsLock};
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
            std::lock_guard lock{oakum.allocationsLock};
            oakum.registerDeallocation(pointer);
        }
    }
}

void OakumController::OakumController::registerAllocation(OakumAllocation info) {
    info.allocationId = this->allocationIdCounter++;
    StackTraceHelper::captureFrames(info.stackFrames, info.stackFramesCount);

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
    std::lock_guard lock{this->allocationsLock};

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
    std::lock_guard lock{this->allocationsLock};
    return this->allocations.size();
}

bool OakumController::getStackTrace(OakumAllocation &allocation) {
    if (allocation.stackFramesCount != 0) {
        return StackTraceHelper::resolveFrames(allocation.stackFrames, allocation.stackFramesCount);
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
