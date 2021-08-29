#include <functional>

#ifdef __linux__
#include "source/linux/child_process.h"

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>
#elif _WIN32
#include <Windows.h>
#include <dbghelp.h>
#else
static_cast(false, "Unsupported OS");
#endif

namespace Oakum {
struct Syscalls {
#ifdef __linux__
    using DemangleSymbolT = std::function<char *(const char *mangled_name, char *output_buffer, size_t *length, int *status)>;
    using RunProcessForOutputT = std::function<std::string(std::string_view binaryName, std::initializer_list<std::string_view> args)>;
    using DladdrT = std::function<int(const void *addr, Dl_info *info)>;
    using Dladdr1T = std::function<int(const void *addr, Dl_info *info, void **extra_info, int flags)>;

    DemangleSymbolT demangleSymbol = ::abi::__cxa_demangle;
    RunProcessForOutputT runProcessForOutput = ChildProcess::runForOutput;
    DladdrT dladdr = ::dladdr;
    Dladdr1T dladdr1 = ::dladdr1;
#elif _WIN32
    using SymFromAddrT = std::function<BOOL(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol)>;
    using SymGetLineFromAddr64T = std::function<BOOL(HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64)>;

    SymFromAddrT SymFromAddr = ::SymFromAddr;
    SymGetLineFromAddr64T SymGetLineFromAddr64 = ::SymGetLineFromAddr64;
#else
    static_cast(false, "Unsupported OS");
#endif
};

extern Syscalls syscalls;
} // namespace Oakum
