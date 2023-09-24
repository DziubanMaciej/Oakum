#include "tests/unit_tests/mock_syscalls.h"

#include <gtest/gtest.h>

RaiiSyscallsBackup MockSyscalls::mockSymbolResolvingSuccess(const char *valueToReturn) {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.dladdr = [](const void *addr, Dl_info *info) -> int {
        info->dli_sname = "__MangledSymbolName__";
        return 1;
    };

    Oakum::syscalls.demangleSymbol = [valueToReturn](const char *mangled_name, char *output_buffer, size_t *length, int *status) -> char * {
        EXPECT_STREQ("__MangledSymbolName__", mangled_name);
        *status = 0;

        size_t demangledSize = strlen(valueToReturn);
        char *demangled = static_cast<char *>(malloc(demangledSize + 1));
        strcpy(demangled, valueToReturn);
        return demangled;
    };

    return backup;
}

RaiiSyscallsBackup MockSyscalls::mockSymbolResolvingFail() {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.dladdr = [](const void *addr, Dl_info *info) -> int {
        return 0;
    };

    return backup;
}

RaiiSyscallsBackup MockSyscalls::mockSourceLocationResolvingSuccess(const char *fileToReturn, size_t lineToReturn) {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.dladdr1 = [](const void *addr, Dl_info *info, void **extra_info, int flags) -> int {
        info->dli_fname = "NiceBinary";

        static link_map linkMap{};
        *reinterpret_cast<link_map **>(extra_info) = &linkMap;

        return 1;
    };

    Oakum::syscalls.runProcessForOutput = [fileToReturn, lineToReturn](std::string_view binaryName, std::initializer_list<std::string_view> args) -> std::string {
        if (binaryName == "which") {
            return "1";
        }
        if (binaryName == "addr2line") {
            // EXPECT_EQ("-e", args.data()[0]);
            //  EXPECT_EQ(binaryName, args.data()[1]);
            std::ostringstream result{};
            result << fileToReturn << ":" << (lineToReturn + 1);
            return result.str();
        }
        FATAL_ERROR("Unreachable code in syscall mock");
    };

    return backup;
}

RaiiSyscallsBackup MockSyscalls::mockSourceLocationResolvingFail() {
    RaiiSyscallsBackup backup{};

    Oakum::syscalls.dladdr1 = [](const void *addr, Dl_info *info, void **extra_info, int flags) -> int {
        return 0;
    };

    return backup;
}
