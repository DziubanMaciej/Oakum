#include "source/error.h"
#include "source/include/oakum/oakum_api.h"
#include "source/stack_trace.h"

#include <Windows.h>
#include <algorithm>
#include <dbghelp.h>

bool StackTraceHelper::supportsSourceLocations() {
    return true;
}

void StackTraceHelper::captureFrames(OakumStackFrame *frames, size_t &framesCount) {
    static thread_local void *frameAddresses[OAKUM_MAX_STACK_FRAMES_COUNT] = {};
    framesCount = CaptureStackBackTrace(skippedFrames, OAKUM_MAX_STACK_FRAMES_COUNT, frameAddresses, nullptr);

    if (framesCount > 0) {
        for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
            frames[frameIndex].address = frameAddresses[frameIndex];
        }
    }
}

bool StackTraceHelper::resolveSymbols(OakumStackFrame *frames, size_t framesCount, const std::optional<std::string> &fallbackSymbolName) {
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    // Prepare parameters
    constexpr static size_t maxSymbolNameLength = 128;
    constexpr static size_t symbolInfoStructSize = sizeof(SYMBOL_INFO) + maxSymbolNameLength + 1;
    static thread_local union {
        char asMemory[symbolInfoStructSize];
        SYMBOL_INFO asSymbolInfo;
    } symbolInfo;
    symbolInfo.asSymbolInfo.MaxNameLen = maxSymbolNameLength;
    symbolInfo.asSymbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);

    // Resolve symbol for each frame
    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        const DWORD64 address = reinterpret_cast<DWORD64>(frames[frameIndex].address);

        if (SymFromAddr(process, address, 0, &symbolInfo.asSymbolInfo)) {
            setupString(frames[frameIndex].symbolName, symbolInfo.asSymbolInfo.Name);
        } else if (fallbackSymbolName.has_value()) {
            setupString(frames[frameIndex].symbolName, fallbackSymbolName.value().c_str());
        } else {
            result = false;
        }
    }
    return result;
}

bool StackTraceHelper::resolveSourceLocations(OakumStackFrame *frames, size_t framesCount, const std::optional<std::string> &fallbackSourceFileName) {
    // Initialize environment for querying source locations
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);

    // Prepare parameters
    IMAGEHLP_LINE64 lineInfo{};
    lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD displacement = 0;

    // Resolve source location for each frame
    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        const DWORD64 address = reinterpret_cast<DWORD64>(frames[frameIndex].address);

        if (SymGetLineFromAddr64(process, address, &displacement, &lineInfo)) {
            setupString(frames[frameIndex].fileName, lineInfo.FileName);
            frames[frameIndex].fileLine = lineInfo.LineNumber;
        } else if (fallbackSourceFileName.has_value()) {
            setupString(frames[frameIndex].fileName, fallbackSourceFileName.value().c_str());
        } else {
            result = false;
        }
    }
    return result;
}
