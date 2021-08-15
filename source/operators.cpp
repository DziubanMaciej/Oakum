#include "oakum.h"

#include <new>

void *operator new(std::size_t size) {
    return Oakum::OakumController::allocateMemory(size, false);
}

void *operator new(std::size_t size, const std::nothrow_t &tag) noexcept {
    return Oakum::OakumController::allocateMemory(size, true);
}

void *operator new[](std::size_t size) {
    return Oakum::OakumController::allocateMemory(size, false);
}

void *operator new[](std::size_t size, const std::nothrow_t &tag) noexcept {
    return Oakum::OakumController::allocateMemory(size, true);
}

void operator delete(void *ptr) noexcept {
    Oakum::OakumController::deallocateMemory(ptr);
}

void operator delete[](void *ptr) noexcept {
    Oakum::OakumController::deallocateMemory(ptr);
}
