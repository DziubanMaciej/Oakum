#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @file oakum_api.h
 * @author Maciej Dziuban
 * @date 2021/08/25
 * @brief Public API of the Oakum library
 *
 * This file definse a set of API entrypoints for the Oakum library. Although this
 * API header should generally be compiler independent, the user must ensure all the
 * preprocessor definitions, which may alter some structures are the same when
 * compiling the library and when compiling the applications using it.
 */

extern "C" {

/// @brief An integer type for uniquely identifying allocations.
using OakumAllocationIdType = uint64_t;

#ifndef OAKUM_MAX_STACK_FRAMES_COUNT
/// @brief Maximum number of stack frames captured by the library.
#define OAKUM_MAX_STACK_FRAMES_COUNT 10
#endif

/// @brief Input configuration of the library via #oakumInit function
struct OakumInitArgs {
    bool trackStackTraces;              ///< Enable stack trace tracking. See #OakumStackFrame for more information.
    bool threadSafe;                    ///< Enable thread safety inside the library.
    const char *fallbackSymbolName;     ///< Symbol name to be used, when #oakumResolveStackTraceSymbols fails to resolve the actual name. May be null.
    const char *fallbackSourceFileName; ///< Source file name to be used, when #oakumResolveStackTraceSourceLocations fails to resolve the actual name. May be null.
};

/// @brief Output configuration of the library reported by #oakumGetCapabilities function.
struct OakumCapabilities {
    bool supportStackTraces;                ///< @brief If set to `true`, the library is tracking stack traces
                                            ///< @details Stack trace tracking is enabled by the setting #OakumInitArgs.trackStackTraces to `true`.
    bool supportStackTracesSourceLocations; ///< @brief If set to `true`, the library is capable of retrieving source locations for stack traces.
                                            ///< @details This capability will never be active unless #supportStackTraces is also active.
                                            ///< @details On some system Retrieving source locations of captured stack traces might not be supported, even if #supportStackTraces is enabled.
    bool threadSafe;                        ///< @brief If set to `true`, the library is ensuring thread safety with locks.
                                            ///< @details Thread safety can be enabled/disabled by the user by setting #OakumInitArgs.threadSafe to a desired value.
};

/// @brief Captured stack frame
struct OakumStackFrame {
    void *address;         ///< @brief Virtual address of captured stack frame.
                           ///< @details If library is not tracking stack traces (see #OakumCapabilities), this field will be set to `NULL`.
    char *symbolName;      ///< @brief Human-readable name of the function for the current stack frame.
                           ///< @details This field will be initialized to `NULL`. It will be filled after a successfull call to #oakumResolveStackTraceSymbols.
    char *fileName;        ///< @brief Name of the source file containing related code.
                           ///< @details This field will be initialized to `NULL`. It will be filled after a successfull call to #oakumResolveStackTraceSourceLocations.
    unsigned int fileLine; ///< @brief Line in the source file containing related code.
                           ///< @details This field will be initialized to `NULL`. It will be filled after a successfull call to #oakumResolveStackTraceSourceLocations.
};

/// @brief Captured memory allocation
struct OakumAllocation {
    OakumAllocationIdType allocationId;                        ///< @brief Unique allocation identifier
    size_t size;                                               ///< @brief Size of the allocation
    void *pointer;                                             ///< @brief Address of the allocation
    bool noThrow;                                              ///< @brief If set to `true`, allocation was made with `std::nothrow` specifier
    OakumStackFrame stackFrames[OAKUM_MAX_STACK_FRAMES_COUNT]; ///< @brief Captured stack trace
    size_t stackFramesCount;                                   ///< @brief Number of captured stack frames
};

/// @brief Result code returned from all Oakum API calls
enum OakumResult {
    OAKUM_SUCCESS,               ///< @brief Successfull function invocation.
    OAKUM_ALREADY_INITIALIZED,   ///< @brief Attempt to initialize the library multiple times.
    OAKUM_UNINITIALIZED,         ///< @brief Usage of a function other than #oakumInit before calling #oakumInit.
    OAKUM_NOT_IGNORING,          ///< @brief Too many calls to #oakumStopIgnore.
    OAKUM_INVALID_VALUE,         ///< @brief One or more arguments passed are incorrect.
    OAKUM_LEAKS_DETECTED,        ///< @brief Non-zero count of tracked allocations. Possible memory leak.
    OAKUM_RESOLVING_FAILED,      ///< @brief Error querying information from the system.
    OAKUM_FEATURE_NOT_SUPPORTED, ///< @brief Attempt to use unsupported API call.
};

/// @brief Initialize the library. This must be the first API call used.
/// @details It is an error to call #oakumInit multiple time subsequently.
/// @details Library can be initialize again after calling #OakumDeinit.
/// @param[in] args input configuration.
/// @return #OAKUM_ALREADY_INITIALIZED, if #oakumInit had been previously called without calling #oakumDeinit.
/// @return #OAKUM_INVALID_VALUE, if #args is `NULL`.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumInit(const OakumInitArgs *args);

/// @brief Resets the library.
/// @details If @p requireNoLeaks is not enabled, the library will ignore leaked memory and reset its state.
/// @details If @p requireNoLeaks is enabled and there are some allocations, which has not been yet deallocated,
/// the library will fail this function and will **not** reset its state. If no leaks are detected,
/// the library will be safely deinitialized.
/// @details The check performed when @p requireNoLeaks is passed is equivalent to calling #oakumDetectLeaks.
/// @param[in] requireNoLeaks detect leaks and possibly fail the call.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_LEAKS_DETECTED, if @p requireNoLeaks is enabled and leaks are detected.
/// @return #OAKUM_SUCCESS otherwise. This means the library is properly deinitialized.
OakumResult oakumDeinit(bool requireNoLeaks);

/// @brief Queries capabilities of the library
/// @param[out] outCapabilities capabilities structure to fill by the library.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_INVALID_VALUE, if @p outCapabilities is `NULL`.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumGetCapabilities(OakumCapabilities *outCapabilities);

/// @brief Checks for leaked memory.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_LEAKS_DETECTED, if leaks are detected.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumDetectLeaks();

/// @brief Retrieves currently un-freed memory allocations.
/// @details The library allocates an array for all un-freed allocations, copies them into that array and
/// stores the array address and size at *@p outAllocations and *@p outAllocationsCount.
/// @details The user should not manually free the memory allocated by this function, but rather call
/// #oakumReleaseAllocations to release all resources.
/// @details If stack trace tracking is enabled (see #OakumCapabilities), the library
/// fills #OakumStackFrame.address in all stack frames. However, the rest of the stack trace data is set to
/// `NULL` and must be explicitly requested with #oakumResolveStackTraceSymbols and #oakumResolveStackTraceSourceLocations
/// calls.
/// @param[out] outAllocations address, to which the library will store allocated array address.
/// @param[out] outAllocationsCount address, to which the library will store allocated array size.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_INVALID_VALUE, if @p outAllocations is `NULL`.
/// @return #OAKUM_INVALID_VALUE, if @p outAllocationsCount is `NULL`.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumGetAllocations(OakumAllocation **outAllocations, size_t *outAllocationsCount);

/// @brief Frees allocation structs allocated by #oakumGetAllocations
/// @details User must call this function to ensure proper releasing of library resources. Manual call to `free`
/// or `delete[]` on the allocation structs is not supported and may result in an undefined behaviour.
/// @param[in] allocations array of allocations to release.
/// @param[in] allocationsCount size of the @p allocations array.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_INVALID_VALUE, if @p allocations is `NULL` and @p allocationsCount is not zero.
/// @return #OAKUM_INVALID_VALUE, if @p allocations is not `NULL` and @p allocationsCount is zero.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumReleaseAllocations(OakumAllocation *allocations, size_t allocationsCount);

/// @brief Fills human-readable symbol names in stack traces.
/// @details This call will fill #OakumStackFrame.symbolName field for all stack frames.
/// @details Sometimes it is not possible to resolve the symbol (e.g. when binary was compiled in Release configuration). For these cases the
/// user can specify #OakumInitArgs.fallbackSymbolName, which will be used instead. This will also cause this function to not fail.
/// @param[in] allocations array of allocations to resolve symbols.
/// @param[in] allocationsCount size of the @p allocations array.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_INVALID_VALUE, if @p allocations is `NULL` and @p allocationsCount is not zero.
/// @return #OAKUM_INVALID_VALUE, if @p allocations is not `NULL` and @p allocationsCount is zero.
/// @return #OAKUM_FEATURE_NOT_SUPPORTED, if the library is not tracking stack traces (see #OakumCapabilities).
/// @return #OAKUM_RESOLVING_FAILED, if at least one symbol name could not be resolved and fallback name was not specified (see #OakumInitArgs).
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumResolveStackTraceSymbols(OakumAllocation *allocations, size_t allocationsCount);

/// @brief Fills source code locations in stack traces.
/// @details This call will fill #OakumStackFrame.fileName and #OakumStackFrame.fileLine fields for all stack frames.
/// @details Sometimes it is not possible to resolve the source locations (e.g. when binary was compiled in Release configuration). For these cases the
/// user can specify #OakumInitArgs.fallbackSourceFileName, which will be used instead. This will also cause this function to not fail.
/// @param[in] allocations array of allocations to resolve source locations.
/// @param[in] allocationsCount size of the @p allocations array.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_INVALID_VALUE, if @p allocations is `NULL` and @p allocationsCount is not zero.
/// @return #OAKUM_INVALID_VALUE, if @p allocations is not `NULL` and @p allocationsCount is zero.
/// @return #OAKUM_FEATURE_NOT_SUPPORTED, if the library is not supporting source code locations querying (see #OakumCapabilities).
/// @return #OAKUM_RESOLVING_FAILED, if at least one source code location could not be resolved and fallback name was not specified (see #OakumInitArgs).
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumResolveStackTraceSourceLocations(OakumAllocation *allocations, size_t allocationsCount);

/// @brief Increments allocation ignore counter.
/// @details Some data structures in applications may be growing over time, while still not strictly leaking memory.
/// For this cases we can ignore allocations made by specific sections of the code. All memory allocations made during
/// ignore mode will be dropped, meaning the will not be returned by #oakumGetAllocations, nor will #oakumDetectLeaks
/// report problems with these allocations.
/// @details The library maintains an internal counter. The user may call #oakumStartIgnore multiple times to increment it
/// and #oakumStopIgnore to decrement it. Ignore mode is active whenever the counter is greater than zero. The counter is
/// reset by #oakumDeinit call.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumStartIgnore();

/// @brief Decrements allocation ignore counter
/// @details For details on how ignore counter works, see #oakumStartIgnore.
/// @return #OAKUM_UNINITIALIZED, if #oakumInit has not been called.
/// @return #OAKUM_NOT_IGNORING, if the ignore counter is already 0.
/// @return #OAKUM_SUCCESS otherwise.
OakumResult oakumStopIgnore();
}
