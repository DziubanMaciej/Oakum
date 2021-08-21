cmake_minimum_required(VERSION 3.10.0)

project(Oakum)
include(third_party/CMakeDependencyManager.cmake)

set(OAKUM_BUILD_UNIT_TESTS OFF CACHE BOOL "If enabled, Oakum unit tests will be added to the build")
set(OAKUM_MAX_STACK_FRAMES_COUNT "" CACHE STRING "Maximum number of stack frames captured by the library")

macro (add_os_directory)
    if (UNIX)
        set(OS_DIRECTORY_NAME linux)
    elseif (WIN32)
        set(OS_DIRECTORY_NAME windows)
    else()
        message(FATAL_ERROR "Unknown OS")
    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${OS_DIRECTORY_NAME}/CMakeLists.txt)
        add_subdirectory(${OS_DIRECTORY_NAME})
    endif()
endmacro()

add_subdirectory(source)
if (OAKUM_BUILD_UNIT_TESTS)
    add_subdirectory(unit_tests)
endif()
