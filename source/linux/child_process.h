#pragma once

#include "pipe.h"

#include <optional>
#include <string_view>
#include <sys/types.h>
#include <vector>

class ChildProcess {
public:
    enum class Result {
        Success,
        ForkFailed,
        ChildProcessKilled,
        NotRun,
        AlreadyRun,
        ReadError,
    };

    ChildProcess(std::string_view binaryName);
    ChildProcess(std::string_view binaryName, std::initializer_list<std::string_view> args);

    void addArgument(std::string_view arg);
    Result run();
    Result wait();
    Result getOutput(std::string *&output);

    static std::string runForOutput(std::string_view binaryName, std::initializer_list<std::string_view> args);

private:
    std::vector<char *> getArgumentsPointers();

    Pipe outputPipe{};
    std::string binaryName{};
    std::vector<std::string> arguments{};

    std::optional<pid_t> pid;
    std::optional<std::string> output;
};

#define FATAL_ERROR_ON_FAILED_CHILD_PROCESS(expression)                                                                                                 \
    {                                                                                                                                                   \
        const auto result = (expression);                                                                                                               \
        FATAL_ERROR_IF(result != ChildProcess::Result::Success, "Failure on \"", #expression, "\", ChildProcess::Result = ", static_cast<int>(result)); \
    }
