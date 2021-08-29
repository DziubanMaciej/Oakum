#include "source/error.h"
#include "source/include/oakum/oakum_api.h"
#include "source/stack_trace.h"

#include <cstring>

namespace Oakum {
void StackTraceHelper::setupString(char *&destination, const char *source) {
    const size_t bufferSize = strlen(source) + 1;
    destination = new char[bufferSize];
    FATAL_ERROR_IF(strcpy(destination, source) != 0, "strcpy failed");
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
} // namespace Oakum
