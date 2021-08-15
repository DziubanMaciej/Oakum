cmake_minimum_required(VERSION 3.18.0)

project(Oakum)
include(third_party/CMakeDependencyManager.cmake)

option(BUILD_UNIT_TESTS "If enabled, Oakum unit tests will be added to the build" OFF)

add_subdirectory(source)
if (BUILD_UNIT_TESTS)
    add_subdirectory(unit_tests)
endif()
