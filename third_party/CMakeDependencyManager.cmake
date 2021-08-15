# MIT License
#
# Copyright (c) 2021 Maciej Dziuban
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# ------------------------------------------------------------------------------------- Private functions - simple getters

set(DEPENDENCY_MANAGER_FILE ${CMAKE_CURRENT_LIST_FILE})

function(_DependencyManager_get_targets_in_directory OUT_VAR DIRECTORY)
    get_directory_property(children DIRECTORY ${DIRECTORY} SUBDIRECTORIES)
    set(TARGETS)

    foreach(DIR ${DIRECTORY} ${children})
        if(NOT IS_DIRECTORY ${DIR})
            continue()
        endif()
        if(NOT EXISTS ${DIR}/CMakeLists.txt)
            continue()
        endif()

        get_property(TARGETS_IN_DIRECTORY DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
        list(APPEND TARGETS ${TARGETS_IN_DIRECTORY})
    endforeach()
    set(${OUT_VAR} ${TARGETS} PARENT_SCOPE)
endfunction()

function(_DependencyManager_get_core_count OUT_VAR)
    if(WIN32)
        set(${OUT_VAR} "$ENV{NUMBER_OF_PROCESSORS}")
    else()
        execute_process(COMMAND nproc
                        ERROR_QUIET
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        OUTPUT_VARIABLE ${OUT_VAR})
    endif()
endfunction()

function(_DependencyManager_get_full_dependency_target_name OUT_VAR DEPENDENCY_NAME TGT)
    set(${OUT_VAR} "DependencyManager_${DEPENDENCY_NAME}_${TGT}" PARENT_SCOPE)
endfunction()

function(_DependencyManager_is_in_list OUT_VAR ELEMENT ELEMENTS)
    list (FIND ELEMENTS "${ELEMENT}" INDEX)
    if (${INDEX} GREATER -1)
        set(${OUT_VAR} 1 PARENT_SCOPE)
    else()
        set(${OUT_VAR} 0 PARENT_SCOPE)
    endif()
endfunction()

# ------------------------------------------------------------------------------------- Private functions - simple checks

function(_DependencyManager_require_dependency_property_is_set DEPENDENCY_NAME PROPERTY)
    get_property(VALUE GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_${PROPERTY})
    if (VALUE STREQUAL "")
        message(FATAL_ERROR "${PROPERTY} property has not been set for ${DEPENDENCY_NAME} dependency")
    endif()
endfunction()

function(_DependencyManager_require_dependency_exists DEPENDENCY_NAME)
    get_property(DEPENDENCIES GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCIES)
    _DependencyManager_is_in_list(DEPENDENCY_EXISTS "${DEPENDENCY_NAME}" "${DEPENDENCIES}")
    if (DEPENDENCY_EXISTS STREQUAL "0")
        message(FATAL_ERROR "${DEPENDENCY_NAME} dependency does not exist")
    endif()
endfunction()

function(_DependencyManager_require_valid_value VALUE VALID_VALUES VALUE_NAME)
    _DependencyManager_is_in_list(VALUE_IS_VALID "${VALUE}" "${VALID_VALUES}")
    if (VALUE_IS_VALID STREQUAL "0")
        message(FATAL_ERROR "\"${VALUE}\" is not a valid value for ${VALUE_NAME}.")
    endif()
endfunction()

function(_DependencyManager_require_variable_defined VARIABLE_NAME VALUE_NAME ADDITIONAL_ERROR_MSG)
    if (NOT DEFINED ${VARIABLE_NAME})
        message(FATAL_ERROR "${VALUE_NAME} must be defined${ADDITIONAL_ERROR_MSG}.")
    endif()
endfunction()

function(_DependencyManager_require_dependency_target_exists DEPENDENCY_NAME TGT)
     _DependencyManager_get_full_dependency_target_name(FULL_DEPENDENCY_TARGET_NAME ${DEPENDENCY_NAME} ${TGT})
    if(NOT TARGET ${FULL_DEPENDENCY_TARGET_NAME})
        message(FATAL_ERROR "${FULL_DEPENDENCY_TARGET_NAME} dependency target does not exist")
    endif()
endfunction()

# ------------------------------------------------------------------------------------- Private functions - functionality

function(_DependencyManager_setup_dependency_imported_target DEPENDENCY_NAME)
    # Parse arguments
    set(SINGLE_VALUE_ARGS TARGET TYPE)
    set(MULTI_VALUE_ARGS INCLUDE_DIRECTORIES LINK_LIBRARIES IMPORTED_IMPLIB IMPORTED_LOCATION)
    cmake_parse_arguments(ARG "" "${SINGLE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # Get target name
    _DependencyManager_require_variable_defined(ARG_TARGET TARGET "")
    _DependencyManager_get_full_dependency_target_name(FULL_DEPENDENCY_TARGET_NAME ${DEPENDENCY_NAME} ${ARG_TARGET})

    # Create target
    _DependencyManager_require_valid_value(${ARG_TYPE} "EXECUTABLE;STATIC_LIBRARY;SHARED_LIBRARY" "TYPE")
    if(ARG_TYPE STREQUAL "EXECUTABLE")
        add_executable(${FULL_DEPENDENCY_TARGET_NAME} IMPORTED GLOBAL)
    elseif(ARG_TYPE STREQUAL "STATIC_LIBRARY")
        add_library(${FULL_DEPENDENCY_TARGET_NAME} IMPORTED STATIC GLOBAL)
    elseif(ARG_TYPE STREQUAL "SHARED_LIBRARY")
        add_library(${FULL_DEPENDENCY_TARGET_NAME} IMPORTED SHARED GLOBAL)
    else()
        message(FATAL_ERROR "Unreachable code")
    endif()

    # Set interface properties
    if (DEFINED ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${FULL_DEPENDENCY_TARGET_NAME} INTERFACE ${ARG_INCLUDE_DIRECTORIES})
    endif()
    if (DEFINED ARG_LINK_LIBRARIES)
        target_link_libraries(${FULL_DEPENDENCY_TARGET_NAME} INTERFACE ${ARG_LINK_LIBRARIES})
    endif()
    if (DEFINED ARG_IMPORTED_IMPLIB)
        set_target_properties(${FULL_DEPENDENCY_TARGET_NAME} PROPERTIES IMPORTED_IMPLIB ${ARG_IMPORTED_IMPLIB})
    endif()
    if (DEFINED ARG_IMPORTED_LOCATION)
        set_target_properties(${FULL_DEPENDENCY_TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${ARG_IMPORTED_LOCATION})
    endif()
endfunction()

function (_DependencyManager_generate_target_infos OUTPUT_FILE TARGETS)
    set(CONTENT)
    foreach(TGT ${TARGETS})
        get_target_property(TARGET_TYPE ${TGT} TYPE)
        set(TARGET_INFO "TARGET ${TGT}")
        string(APPEND TARGET_INFO " TYPE ${TARGET_TYPE}")
        string(APPEND TARGET_INFO " INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${TGT},INCLUDE_DIRECTORIES>")
        string(APPEND TARGET_INFO " INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${TGT},INCLUDE_DIRECTORIES>")
        string(APPEND TARGET_INFO " LINK_LIBRARIES $<TARGET_PROPERTY:${TGT},LINK_LIBRARIES>")
        if (NOT TARGET_TYPE STREQUAL "EXECUTABLE")
            string(APPEND TARGET_INFO " IMPORTED_IMPLIB $<TARGET_LINKER_FILE:${TGT}>")
        endif()
        string(APPEND TARGET_INFO " IMPORTED_LOCATION $<TARGET_FILE:${TGT}>")

        string(APPEND CONTENT "${TARGET_INFO}\n")
    endforeach()
    file(GENERATE OUTPUT ${OUTPUT_FILE} CONTENT "${CONTENT}")
endfunction()

function (_DependencyManager_setup_in_project_dependency DEPENDENCY_NAME)
    get_property(SOURCE_DIR GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_DIRECTORY)

    # Add to the project
    add_subdirectory(${SOURCE_DIR} ${CMAKE_BINARY_DIR}/${DEPENDENCY_NAME})

    # Create aliases for project's targets
    _DependencyManager_get_targets_in_directory(TARGETS ${SOURCE_DIR})

    foreach(TGT ${TARGETS})
        _DependencyManager_get_full_dependency_target_name(FULL_DEPENDENCY_TARGET_NAME ${DEPENDENCY_NAME} ${TGT})
        get_target_property(TARGET_TYPE ${TGT} TYPE)
        if (TARGET_TYPE STREQUAL "EXECUTABLE")
            add_executable(${FULL_DEPENDENCY_TARGET_NAME} ALIAS ${TGT})
        else()
            add_library(${FULL_DEPENDENCY_TARGET_NAME} ALIAS ${TGT})
        endif()
    endforeach()
endfunction()


function (_DependencyManager_setup_out_of_project_dependency DEPENDENCY_NAME DEPENDENCY_DIR)
    set(BUILD_DIR ${DEPENDENCY_DIR}/build)
    get_property(SOURCE_DIR GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_DIRECTORY)
    get_property(ALL_BUILD_TYPES GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_ALL_BUILD_TYPES)
    get_property(SELECTED_BUILD_TYPE GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_SELECTED_BUILD_TYPE)

    # Create CMakeLists.txt for the dependency, which will call the actual CMake file of the
    # dependency and intercept all targets created by it
    file(MAKE_DIRECTORY ${BUILD_DIR})
    file(WRITE ${DEPENDENCY_DIR}/CMakeLists.txt "\
        # This file was autogenerated by  the CMakeDependencyManager library.                                 \n\
        # Do not try to modify it                                                                             \n\
                                                                                                              \n\
        cmake_minimum_required(VERSION 3.0.0)                                                                 \n\
        project(Dependency_${DEPENDENCY_NAME})                                                                \n\
        include(${DEPENDENCY_MANAGER_FILE})                                                                   \n\
        add_subdirectory(${SOURCE_DIR} \${CMAKE_CURRENT_SOURCE_DIR}/build/${DEPENDENCY_NAME}/)                \n\
                                                                                                              \n\
        # Get all targets and output their info to files                                                      \n\
        _DependencyManager_get_targets_in_directory(TARGETS ${SOURCE_DIR})                                    \n\
        _DependencyManager_generate_target_infos(${DEPENDENCY_DIR}/targets\$<CONFIG>.txt \"\${TARGETS}\")     \n\
    ")

    # Run the created CMakeLists file
    set(LOG_FILE ${DEPENDENCY_DIR}/log_cmake.txt)
    execute_process(COMMAND cmake -DCMAKE_BUILD_TYPES=${ALL_BUILD_TYPES} ..
            WORKING_DIRECTORY ${BUILD_DIR}
            RESULT_VARIABLE RESULT
            OUTPUT_FILE ${LOG_FILE}
            ERROR_FILE ${LOG_FILE})
    if (NOT ${RESULT} STREQUAL 0)
        message("BUILD_DIR = ${BUILD_DIR}")
        message(FATAL_ERROR "CMake script for dependency ${DEPENDENCY_NAME} failed with error code ${RESULT}. Please inspect log in ${LOG_FILE}")
    endif()

    # Compile all targets in all required configurations
    _DependencyManager_get_core_count(CORE_COUNT)
    foreach(BUILD_TYPE ${ALL_BUILD_TYPES})
        file(READ ${DEPENDENCY_DIR}/targets${BUILD_TYPE}.txt TARGET_INFOS)
        string(REPLACE "\n" ";" TARGET_INFOS ${TARGET_INFOS})
        foreach(TARGET_INFO ${TARGET_INFOS})
            # Retrieve target name
            set(TGT ${TARGET_INFO})
            separate_arguments(TGT)
            list(GET TGT 1 TGT)

            # Compile
            set(LOG_FILE ${DEPENDENCY_DIR}/log_compile_${TGT}_${BUILD_TYPE}.txt)
            execute_process(COMMAND cmake --build . --target ${TGT} --parallel ${CORE_COUNT} --config ${BUILD_TYPE}
                WORKING_DIRECTORY ${BUILD_DIR}
                RESULT_VARIABLE RESULT
                OUTPUT_FILE ${LOG_FILE}
                ERROR_FILE ${LOG_FILE})
            if(NOT ${RESULT} STREQUAL 0)
                message(FATAL_ERROR "Compilation for dependency target ${TGT} failed with error code ${RESULT}. Please inspect log in ${LOG_FILE}")
            endif()

            # Create imported target if needed
            string(REPLACE " " ";" TARGET_INFO_UNPACKED ${TARGET_INFO}) # string->list conversion
            if(BUILD_TYPE STREQUAL SELECTED_BUILD_TYPE)
                _DependencyManager_setup_dependency_imported_target(${DEPENDENCY_NAME} ${TARGET_INFO_UNPACKED})
            endif()
        endforeach()
    endforeach()
endfunction()

function (_DependencyManager_ensure_submodule_initialized DEPENDENCY_NAME DIRECTORY)
    set(MAIN_FILE ${DIRECTORY}/CMakeLists.txt)
    if(NOT EXISTS ${MAIN_FILE})
        message(STATUS "Files for submodule dependency ${DEPENDENCY_NAME} were not found. Updating submodules...")
        execute_process(COMMAND git submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endfunction()

# ------------------------------------------------------------------------------------- Public API

function(DependencyManager_create_dependency DEPENDENCY_NAME)
    # Parse arguments
    set(OPTIONS_ARGS     IS_SUBMODULE)
    set(ONE_VALUE_ARGS   BUILD_METHOD DIRECTORY SELECTED_BUILD_TYPE)
    set(MULTI_VALUE_ARGS ALL_BUILD_TYPES)
    cmake_parse_arguments(ARG "${OPTIONS_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # Initialize default dependency data
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_IS_SUBMODULE "")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_BUILD_METHOD "")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_DIRECTORY "")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_ALL_BUILD_TYPES "")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_SELECTED_BUILD_TYPE "")
    set_property(GLOBAL APPEND PROPERTY DEPENDENCY_MANAGER_DEPENDENCIES ${DEPENDENCY_NAME})

    # Fill dependency data, which was passed by the user
    _DependencyManager_require_valid_value("${ARG_IS_SUBMODULE}" "TRUE;FALSE" "IS_SUBMODULE")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_IS_SUBMODULE "${ARG_IS_SUBMODULE}")

    _DependencyManager_require_valid_value("${ARG_BUILD_METHOD}" "in_project;out_of_project;binary" "BUILD_METHOD")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_BUILD_METHOD "${ARG_BUILD_METHOD}")

    _DependencyManager_require_variable_defined(ARG_DIRECTORY DIRECTORY "")
    set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_DIRECTORY "${ARG_DIRECTORY}")

    # Download submodule if needed
    if (${ARG_IS_SUBMODULE})
        _DependencyManager_ensure_submodule_initialized(${DEPENDENCY_NAME} ${ARG_DIRECTORY})
    endif()

    if (${ARG_BUILD_METHOD} STREQUAL "in_project")
        message(STATUS "Dependency ${DEPENDENCY_NAME} is an in-project dependency. It will be built as a regular target.")
        _DependencyManager_setup_in_project_dependency(${DEPENDENCY_NAME})
    elseif (${ARG_BUILD_METHOD} STREQUAL "out_of_project")
        _DependencyManager_require_variable_defined(ARG_ALL_BUILD_TYPES ALL_BUILD_TYPES " for out_of_project builds")
        set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_ALL_BUILD_TYPES "${ARG_ALL_BUILD_TYPES}")

        _DependencyManager_require_valid_value("${ARG_SELECTED_BUILD_TYPE}" "${ARG_ALL_BUILD_TYPES}" "SELECTED_BUILD_TYPE")
        set_property(GLOBAL PROPERTY DEPENDENCY_MANAGER_DEPENDENCY_${DEPENDENCY_NAME}_SELECTED_BUILD_TYPE "${ARG_SELECTED_BUILD_TYPE}")

        set(DEPENDENCY_DIR ${CMAKE_BINARY_DIR}/CMakeDependencyManager/${DEPENDENCY_NAME})
        message(STATUS "Dependency ${DEPENDENCY_NAME} is an out-of-project dependency. It will be built outside current project in ${DEPENDENCY_DIR}")
        _DependencyManager_setup_out_of_project_dependency(${DEPENDENCY_NAME} ${DEPENDENCY_DIR})
    elseif (${ARG_BUILD_METHOD} STREQUAL "binary")
        message(STATUS "Dependency ${DEPENDENCY_NAME} is a binary dependency.")
    else ()
        message(FATAL_ERROR "Unreachable code")
    endif()
endfunction()

function(DependencyManager_add_binary_dependency_target DEPENDENCY_NAME)
    _DependencyManager_require_dependency_exists(${DEPENDENCY_NAME})

    _DependencyManager_setup_dependency_imported_target(${DEPENDENCY_NAME} ${ARGN})
endfunction()

function(DependencyManager_link_to_dependency_library TARGET_NAME SCOPE DEPENDENCY_NAME DEPENDENCY_TARGET_NAME)
    _DependencyManager_require_dependency_exists(${DEPENDENCY_NAME})
    _DependencyManager_require_dependency_target_exists(${DEPENDENCY_NAME} ${DEPENDENCY_TARGET_NAME})

    _DependencyManager_get_full_dependency_target_name(FULL_DEPENDENCY_TARGET_NAME "${DEPENDENCY_NAME}" "${DEPENDENCY_TARGET_NAME}")
    target_link_libraries(${TARGET_NAME} ${SCOPE} ${FULL_DEPENDENCY_TARGET_NAME})
endfunction()

function(DependencyManager_get_files_requiring_copy_to_cwd OUT_VAR_ABSOLUTE_PATHS OUT_VAR_FILE_NAMES)
    set(ABSOLUTE_PATHS)
    set(FILE_NAMES)

    set(ARG_INDEX 2)
    while(ARG_INDEX LESS ARGC)
        # Get dependency and target name
        set(DEPENDENCY_NAME_INDEX ${ARG_INDEX})
        MATH(EXPR DEPENDENCY_TARGET_NAME_INDEX "${DEPENDENCY_NAME_INDEX}+1")
        set(DEPENDENCY_NAME ${ARGV${DEPENDENCY_NAME_INDEX}})
        set(DEPENDENCY_TARGET_NAME ${ARGV${DEPENDENCY_TARGET_NAME_INDEX}})
        _DependencyManager_get_full_dependency_target_name(FULL_DEPENDENCY_TARGET_NAME "${DEPENDENCY_NAME}" "${DEPENDENCY_TARGET_NAME}")

        # Validate input
        _DependencyManager_require_dependency_exists(${DEPENDENCY_NAME})
        _DependencyManager_require_dependency_target_exists(${DEPENDENCY_NAME} ${DEPENDENCY_TARGET_NAME})

        # Append paths if our target is .dll (or .so)
        get_target_property(TARGET_TYPE ${FULL_DEPENDENCY_TARGET_NAME} TYPE)
        if(TARGET_TYPE STREQUAL SHARED_LIBRARY)
            list(APPEND ABSOLUTE_PATHS "$<TARGET_FILE:${FULL_DEPENDENCY_TARGET_NAME}>")
            list(APPEND FILE_NAMES "$<TARGET_FILE_NAME:${FULL_DEPENDENCY_TARGET_NAME}>")
        endif()

        # Go to next entry
        MATH(EXPR ARG_INDEX "${ARG_INDEX}+2")
    endwhile()

    set(${OUT_VAR_ABSOLUTE_PATHS} ${ABSOLUTE_PATHS} PARENT_SCOPE)
    set(${OUT_VAR_FILE_NAMES} ${FILE_NAMES} PARENT_SCOPE)
endfunction()
