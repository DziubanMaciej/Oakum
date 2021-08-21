#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "stack_trace.h"

#include "oakum/oakum_api.h"
#include "error.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string_view>

static void setupString(char *&destination, const char *source) {
    const size_t sourceSize = strlen(source);
    destination = new char[sourceSize + 1];
    strcpy(destination, source);
}

void demangleAndSetupString(char *&destination, const char *source) {
    if (source == nullptr) {
        return;
    }

    int status {};
    char *demangled = abi::__cxa_demangle(source, 0, 0, &status);
    FATAL_ERROR_IF(status != 0, "Demangling of symbol \"", source, "\" failed. status=", status);
    setupString(destination, demangled);
    free(demangled);
}

void setupStackFrameInfo(OakumStackFrame &frame, void *address) {
    Dl_info dlInfo = {};
    link_map *linkMap = nullptr;
    dladdr1(address, &dlInfo, reinterpret_cast<void**>(&linkMap), RTLD_DL_LINKMAP); // TODO: check error code

    demangleAndSetupString(frame.symbolName, dlInfo.dli_sname);

    const size_t addressVma =  reinterpret_cast<size_t>(address) - linkMap->l_addr;
    printf("{.address=%p   .symbolName=%s    vma=%zx     file=%s  }\n", address, frame.symbolName, addressVma, dlInfo.dli_fname);
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

bool StackTraceHelper::resolveFrames(OakumStackFrame *frames, size_t framesCount) {
    void *frameAddresses[OAKUM_MAX_STACK_FRAMES_COUNT] = {};
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        frameAddresses[frameIndex] = frames[frameIndex].address;
    }
    char **symbols = backtrace_symbols(frameAddresses, framesCount);

    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        setupStackFrameInfo(frames[frameIndex], frames[frameIndex].address);
    }

    free(symbols);
    return true;
}