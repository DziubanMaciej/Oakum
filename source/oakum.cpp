#include "error.h"
#include "oakum.h"

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

    const bool success = info.pointer != nullptr;
    if (isInitialized() && success && !getInstance()->getIgnoreState()) {
        getInstance()->registerAllocation(info);
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

    if (isInitialized() && !getInstance()->getIgnoreState()) {
        getInstance()->registerDeallocation(pointer);
    }
}

void OakumController::OakumController::registerAllocation(OakumAllocation info) {
    RaiiOakumIgnore raiiIgnore{};

    FATAL_ERROR_IF(info.allocationId != 0, "allocationId should not be set");
    info.allocationId = this->allocationIdCounter++;

    std::lock_guard lock{this->allocationsLock};
    FATAL_ERROR_IF(this->allocations.find(info.pointer) != this->allocations.end(), "Pointer already registered");
    this->allocations.insert({info.pointer, info});
}

void OakumController::OakumController::registerDeallocation(void *pointer) {
    RaiiOakumIgnore raiiIgnore{};

    FATAL_ERROR_IF(pointer == nullptr, "Null pointer registration");

    std::lock_guard lock{this->allocationsLock};
    auto allocation = this->allocations.find(pointer);
    if (allocation != this->allocations.end()) {
        this->allocations.erase(allocation);
    }
}

void OakumController::getAllocations(OakumAllocation *outAllocations, size_t allocationsCount, size_t &outAllocationsReturned, size_t &outAllocationsAvailable) {
    std::lock_guard lock{this->allocationsLock};

    outAllocationsAvailable = this->allocations.size();
    outAllocationsReturned = std::min(outAllocationsAvailable, allocationsCount);

    size_t allocationIndex = 0;
    for (const auto &entry : this->allocations) {
        if (allocationIndex == outAllocationsReturned) {
            break;
        }

        outAllocations[allocationIndex++] = entry.second;
    }
}

bool OakumController::hasAllocations() {
    std::lock_guard lock{this->allocationsLock};
    return this->allocations.size();
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
