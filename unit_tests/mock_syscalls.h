#pragma once

#include "source/syscalls.h"

class RaiiSyscallsBackup {
public:
    RaiiSyscallsBackup() {
        savedSyscalls = Oakum::syscalls;
        saved = true;
    }

    RaiiSyscallsBackup(RaiiSyscallsBackup &&other) noexcept {
        savedSyscalls = other.savedSyscalls;
        saved = other.saved;
        other.saved = false;
    }

    ~RaiiSyscallsBackup() {
        if (saved) {
            Oakum::syscalls = savedSyscalls;
        }
    }

private:
    Oakum::Syscalls savedSyscalls;
    bool saved;
};

struct MockSyscalls {
    static RaiiSyscallsBackup mockSymbolResolvingSuccess(const char *valueToReturn);
    static RaiiSyscallsBackup mockSymbolResolvingFail();
};
