#include "mock_syscalls.h"

RaiiSyscallsBackup MockSyscalls::mockSymbolResolvingSuccess(const char *valueToReturn) {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.SymFromAddr = [valueToReturn](HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol) -> BOOL {
        strcpy(Symbol->Name, valueToReturn);
        return TRUE;
    };

    return backup;
}

RaiiSyscallsBackup MockSyscalls::mockSymbolResolvingFail() {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.SymFromAddr = +[](HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol) -> BOOL {
        return FALSE;
    };

    return backup;
}
