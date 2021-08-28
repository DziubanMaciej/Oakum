#pragma once

#include "source/linux/error.h"

#include <unistd.h>

class Pipe {
public:
    Pipe() = default;
    Pipe(const Pipe &) = delete;
    Pipe &operator=(const Pipe &) = delete;
    Pipe(Pipe &&other) { *this = std::move(other); }
    Pipe &operator=(Pipe &&other) {
        this->descriptors[0] = other.descriptors[0];
        this->descriptors[1] = other.descriptors[1];
        other.releaseRead();
        other.releaseWrite();
        return *this;
    }
    ~Pipe() {
        closeRead();
        closeWrite();
    }

    void create() {
        closeRead();
        closeWrite();
        FATAL_ERROR_ON_FAILED_SYSCALL(pipe(descriptors));
    }

    int getRead() const { return descriptors[0]; }
    int getWrite() const { return descriptors[1]; }

    void closeDescriptor(int &descriptor) {
        if (descriptor != -1) {
            FATAL_ERROR_ON_FAILED_SYSCALL(close(descriptor));
            releaseDescriptor(descriptor);
        }
    }

    void closeRead() {
        closeDescriptor(descriptors[0]);
    }

    void closeWrite() {
        closeDescriptor(descriptors[1]);
    }

    void releaseDescriptor(int &descriptor) {
        descriptor = -1;
    }

    void releaseRead() {
        releaseDescriptor(descriptors[0]);
    }

    void releaseWrite() {
        releaseDescriptor(descriptors[1]);
    }

private:
    int descriptors[2] = {-1, -1};
};