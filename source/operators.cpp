#include "source/oakum_controller.h"

#include <new>

void *operator new(std::size_t size) {
    return Oakum::OakumController::allocateMemory(size, false);
}

void *operator new(std::size_t size, [[maybe_unused]] const std::nothrow_t &tag) noexcept {
    return Oakum::OakumController::allocateMemory(size, true);
}

void *operator new[](std::size_t size) {
    return Oakum::OakumController::allocateMemory(size, false);
}

void *operator new[](std::size_t size, [[maybe_unused]] const std::nothrow_t &tag) noexcept {
    return Oakum::OakumController::allocateMemory(size, true);
}

void operator delete(void *ptr) noexcept {
    Oakum::OakumController::deallocateMemory(ptr);
}

void operator delete[](void *ptr) noexcept {
    Oakum::OakumController::deallocateMemory(ptr);
}

void operator delete(void *ptr, [[maybe_unused]] size_t size) noexcept {
    Oakum::OakumController::deallocateMemory(ptr);
}

void operator delete[](void *ptr, [[maybe_unused]] size_t size) noexcept {
    Oakum::OakumController::deallocateMemory(ptr);
}
