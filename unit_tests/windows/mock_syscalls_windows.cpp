#include "unit_tests/mock_syscalls.h"

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

RaiiSyscallsBackup MockSyscalls::mockSourceLocationResolvingSuccess(const char *fileToReturn, size_t lineToReturn) {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.SymGetLineFromAddr64 = [fileToReturn, lineToReturn](HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64) -> BOOL {
        static char internalBuffer[256] = {};
        strcpy(internalBuffer, fileToReturn);
        Line64->FileName = internalBuffer;
        Line64->LineNumber = static_cast<DWORD>(lineToReturn);
        return TRUE;
    };

    return backup;
}

RaiiSyscallsBackup MockSyscalls::mockSourceLocationResolvingFail() {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.SymGetLineFromAddr64 = +[](HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64) -> BOOL {
        return FALSE;
    };

    return backup;
}
