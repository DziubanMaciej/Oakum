#include "stack_trace.h"

#include "oakum/oakum_api.h"

#include <Windows.h>
#include <algorithm>
#include <dbghelp.h>

void StackTraceHelper::captureFrames(OakumStackFrame *frames, size_t &framesCount) {
    constexpr static size_t skippedFrames = 3;
    static thread_local void *frameAddresses[OAKUM_MAX_STACK_FRAMES_COUNT] = {};
    framesCount = CaptureStackBackTrace(skippedFrames, OAKUM_MAX_STACK_FRAMES_COUNT, frameAddresses, nullptr);

    if (framesCount > 0) {
        for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
            frames[frameIndex].address = frameAddresses[frameIndex];
            frames[frameIndex].symbolName = nullptr;
            frames[frameIndex].fileName = nullptr;
            frames[frameIndex].fileLine = 0;
        }
    }
}

static void setupString(char *&destination, const char *source) {
    const size_t sourceSize = strlen(source);
    destination = new char[sourceSize + 1];
    strcpy(destination, source);
}

bool StackTraceHelper::resolveFrames(OakumStackFrame *frames, size_t framesCount) {
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);

    constexpr static size_t maxSymbolNameLength = 128;
    constexpr static size_t symbolInfoStructSize = sizeof(SYMBOL_INFO) + maxSymbolNameLength + 1;
    static thread_local union {
        char asMemory[symbolInfoStructSize];
        SYMBOL_INFO asSymbolInfo;
    } symbolInfo;
    DWORD displacement = 0;

    symbolInfo.asSymbolInfo.MaxNameLen = maxSymbolNameLength;
    symbolInfo.asSymbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
    IMAGEHLP_LINE64 lineInfo{};
    lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        const DWORD64 address = reinterpret_cast<DWORD64>(frames[frameIndex].address);

        if (SymFromAddr(process, address, 0, &symbolInfo.asSymbolInfo)) {
            setupString(frames[frameIndex].symbolName, symbolInfo.asSymbolInfo.Name);
        } else {
            result = false;
            break;
        }

        if (SymGetLineFromAddr64(process, address, &displacement, &lineInfo)) {
            setupString(frames[frameIndex].fileName, lineInfo.FileName);
            frames[frameIndex].fileLine = lineInfo.LineNumber;
        }
    }

    return result;
}
