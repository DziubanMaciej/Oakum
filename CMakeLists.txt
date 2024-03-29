cmake_minimum_required(VERSION 3.10.0)

project(Oakum)
include(third_party/CMakeDependencyManager.cmake)
set(OAKUM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(OAKUM_BUILD_EXAMPLES OFF CACHE BOOL "If enabled, example Oakum applications will be added to the build")
set(OAKUM_BUILD_TESTS OFF CACHE BOOL "If enabled, Oakum tests will be added to the build")
set(OAKUM_MAX_STACK_FRAMES_COUNT "" CACHE STRING "Maximum number of stack frames captured by the library")
set(OAKUM_GENERATE_DOCS "" CACHE BOOL "Adds documentation generation target using Doxygen")
if (WIN32)
    set(EXECUTABLE_SUFFIX ".exe")
endif()
set(OAKUM_DOXYGEN_COMMAND "" CACHE STRING "doxygen${EXECUTABLE_SUFFIX}")

macro (add_subdirectories)
    set(FORBIDDEN_DIR_NAMES)
    if (UNIX)
        list(APPEND FORBIDDEN_DIR_NAMES "windows")
    elseif (WIN32)
        list(APPEND FORBIDDEN_DIR_NAMES "linux")
    endif()

    file(GLOB SUB_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(SUB_DIR ${SUB_DIRS})
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/CMakeLists.txt)
            continue()
        endif()

        list(FIND FORBIDDEN_DIR_NAMES ${SUB_DIR} INDEX)
        if (NOT INDEX EQUAL -1)
            continue()
        endif()

        add_subdirectory(${SUB_DIR})
    endforeach()
endmacro()

macro (append_sources LIST_NAME SET_IN_PARENT_SCOPE)
    file(GLOB SOURCES_IN_CURRENT_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )
    list(APPEND ${LIST_NAME} ${SOURCES_IN_CURRENT_DIR})
    if (${SET_IN_PARENT_SCOPE})
        set(${LIST_NAME} "${${LIST_NAME}}" PARENT_SCOPE)
    endif()
endmacro()

macro(target_env_specific_capabilities TARGET SCOPE)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(${TARGET} ${SCOPE}
            -DOAKUM_SYMBOLS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>
            -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>
        )
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_definitions(${TARGET} ${SCOPE}
            -DOAKUM_SYMBOLS_AVAILABLE=1
            -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_definitions(${TARGET} ${SCOPE} -DOAKUM_SYMBOLS_AVAILABLE=1)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.0.0)
            target_compile_definitions(${TARGET} ${SCOPE} -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=0)
        else()
            # Clang 14 on Ubuntu 22.04 creates a weird binary, which cannot be queried by addr2line for some reason.
            # It returns "DWARF error: invalid or unhandled FORM value: 0x23". Forcing dwarf5 doesn't help. I don't
            # understand. This is hopeless, I'm giving up and just disabling tests, which query source locations.
            target_compile_definitions(${TARGET} ${SCOPE} -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>)
        endif()
    endif()
endmacro()

add_subdirectory(third_party)
add_subdirectory(source)
if (OAKUM_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
if (OAKUM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if (OAKUM_GENERATE_DOCS)
    add_subdirectory(documentation)
endif()
