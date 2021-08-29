#include "source/error.h"
#include "source/include/oakum/oakum_api.h"
#include "source/linux/child_process.h"
#include "source/stack_trace.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <link.h>
#include <sstream>
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
    FATAL_ERROR_IF(status == -3, "Demangling of symbol \"", source, "\" failed. status=", status);
    if (status != 0) {
        setupString(destination, source);
    } else {
        setupString(destination, demangled);
        free(demangled);
    }
}

std::pair<std::string, size_t> addr2line(const char *binaryName, size_t vma) {
    std::stringstream hexStream;
    hexStream << std::hex << vma;
    std::string vmaString = hexStream.str();

    const std::string output = ChildProcess::runForOutput("addr2line", {"-e", binaryName, vmaString});
    const size_t colonPos = output.find_first_of(':');
    const std::string fileNameString = output.substr(0, colonPos);
    const std::string fileLineString = output.substr(colonPos + 1);

    std::istringstream lineStream{fileLineString};
    size_t fileLine{};
    lineStream >> fileLine;
    if (fileLine > 0) {
        fileLine--;
    }

    return {fileNameString, fileLine};
}

bool StackTraceHelper::supportsSourceLocations() {
    std::string output = ChildProcess::runForOutput("which", {"addr2line"});
    return !output.empty();
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
    return result;
}

bool StackTraceHelper::resolveSourceLocations(OakumStackFrame *frames, size_t framesCount) {
    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        OakumStackFrame &frame = frames[frameIndex];

        Dl_info dlInfo = {};
        link_map *linkMap = {};
        if (dladdr1(frame.address, &dlInfo, reinterpret_cast<void **>(&linkMap), RTLD_DL_LINKMAP) != 0) {
            const char *binaryName = dlInfo.dli_fname;
            const size_t addressVma = reinterpret_cast<size_t>(frame.address) - linkMap->l_addr;
            const auto [fileName, fileLine] = addr2line(binaryName, addressVma);
            if (fileName != "??") {
                setupString(frame.fileName, fileName.c_str());
            }
            frame.fileLine = fileLine;
        } else {
            return result;
        }
    }

    return result;
}
