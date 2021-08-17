cmake_minimum_required(VERSION 3.10.0)

project(Oakum)
include(third_party/CMakeDependencyManager.cmake)

set(OAKUM_BUILD_UNIT_TESTS OFF CACHE BOOL "If enabled, Oakum unit tests will be added to the build")
set(OAKUM_MAX_STACK_FRAMES_COUNT "" CACHE STRING "Maximum number of stack frames captured by the library")

add_subdirectory(source)
if (OAKUM_BUILD_UNIT_TESTS)
    add_subdirectory(unit_tests)
endif()
