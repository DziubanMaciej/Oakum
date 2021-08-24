#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "error.h"
#include "stack_trace.h"

#include "oakum/oakum_api.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>

static void setupString(char *&destination, const char *source) {
    const size_t sourceSize = strlen(source);
    destination = new char[sourceSize + 1];
    strcpy(destination, source);
}

void demangleAndSetupString(char *&destination, const char *source) {
    if (source == nullptr) {
        return;
    }

    int status{};
    char *demangled = abi::__cxa_demangle(source, 0, 0, &status);
    FATAL_ERROR_IF(status != 0, "Demangling of symbol \"", source, "\" failed. status=", status);
    setupString(destination, demangled);
    free(demangled);
}

bool StackTraceHelper::supportsSourceLocations() {
    return false;
}

void StackTraceHelper::initializeFrames(OakumStackFrame *frames, size_t &framesCount) {
    OakumStackFrame emptyFrame = {
        nullptr,
        nullptr,
        nullptr,
        0u,
    };
    std::fill_n(frames, framesCount, emptyFrame);
    framesCount = 0u;
}

void StackTraceHelper::captureFrames(OakumStackFrame *frames, size_t &framesCount) {
    static thread_local void *frameAddresses[OAKUM_MAX_STACK_FRAMES_COUNT + skippedFrames] = {};
    framesCount = backtrace(frameAddresses, OAKUM_MAX_STACK_FRAMES_COUNT + skippedFrames);
    framesCount -= skippedFrames;

    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        frames[frameIndex].address = frameAddresses[frameIndex + skippedFrames];
    }
}

bool StackTraceHelper::resolveSymbols(OakumStackFrame *frames, size_t framesCount) {
    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        OakumStackFrame &frame = frames[frameIndex];

        Dl_info dlInfo = {};
        if (dladdr(frame.address, &dlInfo) != 0) {
            demangleAndSetupString(frame.symbolName, dlInfo.dli_sname);
        } else {
            result = false;
        }
    }
    return true;
}

bool StackTraceHelper::resolveSourceLocations(OakumStackFrame *frames, size_t framesCount) {
    // const size_t addressVma = reinterpret_cast<size_t>(address) - linkMap->l_addr;
    FATAL_ERROR("Not implemented");
    return false;
}