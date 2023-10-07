#include "source/include/oakum/oakum_api.h"
#include "source/oakum_controller.h"

#define OAKUM_VERIFY(condition, errorCode) \
    if (condition) {                       \
        return errorCode;                  \
    }
#define OAKUM_VERIFY_INITIALIZATION(expectedValue, errorCode) OAKUM_VERIFY(Oakum::OakumController::isInitialized() != expectedValue, errorCode)
#define OAKUM_VERIFY_NON_NULL(ptr) OAKUM_VERIFY(ptr == nullptr, OAKUM_INVALID_VALUE)
#define OAKUM_VERIFY_POSITIVE(value) OAKUM_VERIFY(value <= 0, OAKUM_INVALID_VALUE)

OakumResult oakumInit(const OakumInitArgs *args) {
    OAKUM_VERIFY_INITIALIZATION(false, OAKUM_ALREADY_INITIALIZED);
    OAKUM_VERIFY_NON_NULL(args);

    Oakum::OakumController::initialize(*args);
    return OAKUM_SUCCESS;
}

OakumResult oakumDeinit(bool requireNoLeaks) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);

    if (requireNoLeaks && Oakum::OakumController::getInstance()->hasAllocations()) {
        return OAKUM_LEAKS_DETECTED;
    }

    Oakum::OakumController::deinitialize();
    return OAKUM_SUCCESS;
}

OakumResult oakumGetCapabilities(OakumCapabilities *outCapabilities) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY_NON_NULL(outCapabilities);

    *outCapabilities = Oakum::OakumController::getInstance()->getCapabilities();
    return OAKUM_SUCCESS;
}

OakumResult oakumDetectLeaks() {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);

    if (Oakum::OakumController::getInstance()->hasAllocations()) {
        return OAKUM_LEAKS_DETECTED;
    }

    return OAKUM_SUCCESS;
}

OakumResult oakumGetAllocations(OakumAllocation **outAllocations, size_t *outAllocationsCount) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY_NON_NULL(outAllocations);
    OAKUM_VERIFY_NON_NULL(outAllocationsCount);

    Oakum::OakumController::getInstance()->getAllocations(*outAllocations, *outAllocationsCount);
    return OAKUM_SUCCESS;
}

OakumResult oakumReleaseAllocations(OakumAllocation *allocations, size_t allocationsCount) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY((allocations == nullptr) != (allocationsCount == 0), OAKUM_INVALID_VALUE);

    Oakum::OakumController::getInstance()->releaseAllocations(allocations, allocationsCount);
    return OAKUM_SUCCESS;
}

OakumResult oakumResolveStackTraceSymbols(OakumAllocation *allocations, size_t allocationsCount) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY((allocations == nullptr) != (allocationsCount == 0), OAKUM_INVALID_VALUE);
    OAKUM_VERIFY(!Oakum::OakumController::getInstance()->getCapabilities().supportStackTraces, OAKUM_FEATURE_NOT_SUPPORTED);

    for (size_t allocationIndex = 0; allocationIndex < allocationsCount; allocationIndex++) {
        const bool success = Oakum::OakumController::getInstance()->resolveStackTraceSymbols(allocations[allocationIndex]);
        if (!success) {
            return OAKUM_RESOLVING_FAILED;
        }
    }

    return OAKUM_SUCCESS;
}

OakumResult oakumResolveStackTraceSourceLocations(OakumAllocation *allocations, size_t allocationsCount) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY((allocations == nullptr) != (allocationsCount == 0), OAKUM_INVALID_VALUE);
    OAKUM_VERIFY(!Oakum::OakumController::getInstance()->getCapabilities().supportStackTraces, OAKUM_FEATURE_NOT_SUPPORTED);

    for (size_t allocationIndex = 0; allocationIndex < allocationsCount; allocationIndex++) {
        const bool success = Oakum::OakumController::getInstance()->resolveStackTraceSourceLocations(allocations[allocationIndex]);
        if (!success) {
            return OAKUM_RESOLVING_FAILED;
        }
    }

    return OAKUM_SUCCESS;
}

OakumResult oakumStartIgnore() {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);

    Oakum::OakumController::getInstance()->incrementIgnoreRefcount();
    return OAKUM_SUCCESS;
}

OakumResult oakumStopIgnore() {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);

    if (!Oakum::OakumController::getInstance()->decrementIgnoreRefcount()) {
        return OAKUM_NOT_IGNORING;
    }
    return OAKUM_SUCCESS;
}
