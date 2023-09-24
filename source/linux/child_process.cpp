#include "source/linux/child_process.h"

#include <fcntl.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

ChildProcess::ChildProcess(std::string_view binaryName)
    : binaryName(binaryName) {}

ChildProcess::ChildProcess(std::string_view binaryName, std::initializer_list<std::string_view> args)
    : ChildProcess(binaryName) {
    for (std::string_view arg : args) {
        addArgument(arg);
    }
}

std::vector<char *> ChildProcess::getArgumentsPointers() {
    std::vector<char *> result{};
    result.push_back(binaryName.data());
    for (std::string &arg : this->arguments) {
        result.push_back(arg.data());
    }
    result.push_back(nullptr);
    return result;
}

void ChildProcess::addArgument(std::string_view arg) {
    arguments.emplace_back(arg);
}

ChildProcess::Result ChildProcess::run() {
    if (this->pid.has_value()) {
        return Result::AlreadyRun;
    }

    outputPipe.create();

    int forkResult = fork();
    if (forkResult == -1) {
        return Result::ForkFailed;
    }

    if (forkResult == 0) {
        // Child

        // Redirect stdout to the output pipe
        FATAL_ERROR_ON_FAILED_SYSCALL(dup2(outputPipe.getWrite(), STDOUT_FILENO));
        outputPipe.closeWrite();

        // Ignore stderr
        int devNull = open("/dev/null", O_WRONLY);
        FATAL_ERROR_ON_FAILED_SYSCALL(devNull);
        FATAL_ERROR_ON_FAILED_SYSCALL(dup2(devNull, STDERR_FILENO));
        FATAL_ERROR_ON_FAILED_SYSCALL(close(devNull));
        outputPipe.closeRead();

        // Execute binary
        std::vector<char *> argv = getArgumentsPointers();
        char **rawArgv = argv.data();
        FATAL_ERROR_ON_FAILED_SYSCALL(execvp(argv[0], rawArgv));
        FATAL_ERROR("Unreachable code");
    } else {
        // Parent
        this->pid = forkResult;
        outputPipe.closeWrite();
        return Result::Success;
    }
}

ChildProcess::Result ChildProcess::wait() {
    if (!this->pid.has_value()) {
        return Result::NotRun;
    }

    int status{};
    while (true) {
        int waitResult = waitpid(this->pid.value(), &status, 0);

        FATAL_ERROR_ON_FAILED_SYSCALL(waitResult);
        if (WIFSIGNALED(status)) {
            return Result::ChildProcessKilled;
        }
        if (WIFEXITED(status)) {
            return Result::Success;
        }
    }
}

ChildProcess::Result ChildProcess::getOutput(std::string *&output) {
    if (!this->pid.has_value()) {
        return Result::NotRun;
    }

    if (!this->output.has_value()) {
        char buffer[4096];
        std::ostringstream bufferStream{};
        ssize_t readResult{};
        while (true) {
            readResult = read(outputPipe.getRead(), buffer, sizeof(buffer) - 1);
            if (readResult > 0) {
                buffer[readResult] = '\0';
                bufferStream << buffer;
            } else if (readResult < 0) {
                return Result::ReadError;
            } else {
                this->output = bufferStream.str();
                break;
            }
        }
    }

    output = &this->output.value();
    return Result::Success;
}

std::string ChildProcess::runForOutput(std::string_view binaryName, std::initializer_list<std::string_view> args) {
    ChildProcess child{binaryName, args};

    FATAL_ERROR_ON_FAILED_CHILD_PROCESS(child.run());
    FATAL_ERROR_ON_FAILED_CHILD_PROCESS(child.wait());

    std::string *output{};
    FATAL_ERROR_ON_FAILED_CHILD_PROCESS(child.getOutput(output));
    return *output;
}
