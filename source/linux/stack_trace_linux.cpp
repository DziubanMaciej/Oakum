#include "source/error.h"
#include "source/include/oakum/oakum_api.h"
#include "source/stack_trace.h"
#include "source/syscalls.h"

#include <sstream>

namespace Oakum {
static void demangleAndSetupString(char *&destination, const char *source) {
    if (source == nullptr) {
        return;
    }

    int status{};
    char *demangled = syscalls.demangleSymbol(source, 0, 0, &status);
    FATAL_ERROR_IF(status == -3, "Demangling of symbol \"", source, "\" failed. status=", status);
    if (status != 0) {
        StackTraceHelper::setupString(destination, source);
    } else {
        StackTraceHelper::setupString(destination, demangled);
        free(demangled);
    }
}

static std::pair<std::string, size_t> addr2line(const char *binaryName, size_t vma) {
    std::stringstream hexStream;
    hexStream << std::hex << vma;
    std::string vmaString = hexStream.str();

    const std::string output = syscalls.runProcessForOutput("addr2line", {"-e", binaryName, vmaString});
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
    std::string output = syscalls.runProcessForOutput("which", {"addr2line"});
    return !output.empty();
}

void StackTraceHelper::captureFrames(OakumStackFrame *frames, size_t &framesCount) {
    static thread_local void *frameAddresses[OAKUM_MAX_STACK_FRAMES_COUNT + skippedFrames] = {};
    framesCount = backtrace(frameAddresses, OAKUM_MAX_STACK_FRAMES_COUNT + skippedFrames);
    framesCount -= skippedFrames;

    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        frames[frameIndex].address = frameAddresses[frameIndex + skippedFrames];
    }
}

bool StackTraceHelper::resolveSymbols(OakumStackFrame *frames, size_t framesCount, const std::optional<std::string> &fallbackSymbolName) {
    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        OakumStackFrame &frame = frames[frameIndex];

        Dl_info dlInfo = {};
        if (syscalls.dladdr(frame.address, &dlInfo) != 0) {
            demangleAndSetupString(frame.symbolName, dlInfo.dli_sname);
        } else if (fallbackSymbolName.has_value()) {
            setupString(frames[frameIndex].symbolName, fallbackSymbolName.value().c_str());
        } else {
            result = false;
        }
    }
    return result;
}

bool StackTraceHelper::resolveSourceLocations(OakumStackFrame *frames, size_t framesCount, const std::optional<std::string> &fallbackSourceFileName) {
    bool result = true;
    for (size_t frameIndex = 0; frameIndex < framesCount; frameIndex++) {
        OakumStackFrame &frame = frames[frameIndex];

        Dl_info dlInfo = {};
        link_map *linkMap = {};
        bool resolved = false;
        if (syscalls.dladdr1(frame.address, &dlInfo, reinterpret_cast<void **>(&linkMap), RTLD_DL_LINKMAP) != 0) {
            const char *binaryName = dlInfo.dli_fname;
            const size_t addressVma = reinterpret_cast<size_t>(frame.address) - linkMap->l_addr;
            const auto [fileName, fileLine] = addr2line(binaryName, addressVma);
            if (fileName != "??") {
                setupString(frame.fileName, fileName.c_str());
                resolved = true;
            }
            frame.fileLine = fileLine;
        }

        if (!resolved) {
            if (fallbackSourceFileName.has_value()) {
                setupString(frames[frameIndex].fileName, fallbackSourceFileName.value().c_str());
            } else {
                result = false;
            }
        }
    }

    return result;
}
} // namespace Oakum