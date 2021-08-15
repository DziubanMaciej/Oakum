#include "oakum.h"

#include "oakum/oakum_api.h"

#define OAKUM_VERIFY(condition, errorCode) \
    if (condition) {                       \
        return errorCode;                  \
    }
#define OAKUM_VERIFY_INITIALIZATION(expectedValue, errorCode) OAKUM_VERIFY(Oakum::OakumController::isInitialized() != expectedValue, errorCode)
#define OAKUM_VERIFY_NON_NULL(ptr) OAKUM_VERIFY(ptr == nullptr, OAKUM_INVALID_VALUE)

OakumResult oakumInit(const OakumInitArgs *args) {
    OAKUM_VERIFY_INITIALIZATION(false, OAKUM_ALREADY_INITIALIZED);

    Oakum::OakumController::initialize();
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

OakumResult oakumGetAllocations(OakumAllocation *outAllocations, size_t allocationsCount, size_t *outAllocationsReturned, size_t *outAllocationsAvailable) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY_NON_NULL(outAllocationsReturned);
    OAKUM_VERIFY_NON_NULL(outAllocationsAvailable);
    if (allocationsCount > 0) {
        OAKUM_VERIFY_NON_NULL(outAllocations);
    }

    Oakum::OakumController::getInstance()->getAllocations(outAllocations, allocationsCount, *outAllocationsReturned, *outAllocationsAvailable);
    return OAKUM_SUCCESS;
}

OakumResult oakumGetStackTrace(OakumAllocation *allocation) {
    OAKUM_VERIFY_INITIALIZATION(true, OAKUM_UNINITIALIZED);
    OAKUM_VERIFY_NON_NULL(allocation);

    if (!Oakum::OakumController::getInstance()->getStackTrace(*allocation)) {
        return OAKUM_CANNOT_GET_STACK_TRACE;
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
