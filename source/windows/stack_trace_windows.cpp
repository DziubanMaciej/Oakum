#include "stack_trace.h"

#include "oakum/oakum_api.h"

#include <Windows.h>
#include <algorithm>
#include <dbghelp.h>

void StackTraceHelper::captureFrames(OakumStackFrame *&frames, size_t &framesCount) {
    constexpr static size_t maxFramesCount = 100;
    constexpr static size_t skippedFrames = 3;
    static thread_local void *frameAddresses[maxFramesCount] = {};
    framesCount = CaptureStackBackTrace(3, maxFramesCount, frameAddresses, nullptr);

    if (framesCount > 0) {
        frames = new OakumStackFrame[framesCount]();
        for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
            frames[frameIndex].address = frameAddresses[frameIndex];
        }
    } else {
        frames = nullptr;
    }
}

static void copyString(char *destination, size_t destinationSize, const char *source) {
    size_t sourceSize = strlen(source);
    memcpy(destination, source, destinationSize);
}

bool StackTraceHelper::resolveFrames(OakumStackFrame *frames, size_t framesCount) {
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);

    constexpr static size_t maxSymbolNameLength = OAKUM_MAX_SYMBOL_NAME_SIZE;
    constexpr static size_t symbolInfoStructSize = sizeof(SYMBOL_INFO) + maxSymbolNameLength + 1;
    static thread_local union {
        char asMemory[symbolInfoStructSize];
        SYMBOL_INFO asSymbolInfo;
    } symbolInfo;
    DWORD displacement = 0;

    symbolInfo.asSymbolInfo.MaxNameLen = 255;
    symbolInfo.asSymbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
    IMAGEHLP_LINE64 lineInfo{};
    lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        const DWORD64 address = reinterpret_cast<DWORD64>(frames[frameIndex].address);

        if (SymFromAddr(process, address, 0, &symbolInfo.asSymbolInfo)) {
            copyString(frames[frameIndex].symbolName, OAKUM_MAX_SYMBOL_NAME_SIZE, symbolInfo.asSymbolInfo.Name);
        } else {
            result = false;
            break;
        }

        if (SymGetLineFromAddr64(process, address, &displacement, &lineInfo)) {
            copyString(frames[frameIndex].fileName, OAKUM_MAX_FILE_NAME_SIZE, lineInfo.FileName);
            frames[frameIndex].fileLine = lineInfo.LineNumber;
        }
    }

    return result;
}
