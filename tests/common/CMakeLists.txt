append_sources(OAKUM_TEST_COMMON_SOURCES OFF)
add_subdirectories()

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${OAKUM_TEST_COMMON_SOURCES})
add_library(OakumTestCommon STATIC ${OAKUM_TEST_COMMON_SOURCES})
DependencyManager_link_to_dependency_library(OakumTestCommon PUBLIC googletest gtest)
target_link_libraries(OakumTestCommon PUBLIC Oakum)
target_compile_features(OakumTestCommon PUBLIC cxx_std_17)
target_include_directories(OakumTestCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OAKUM_SOURCE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(OakumTestCommon PUBLIC
        -DOAKUM_SYMBOLS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>
        -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(OakumTestCommon PUBLIC
        -DOAKUM_SYMBOLS_AVAILABLE=1
        -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_definitions(OakumTestCommon PUBLIC -DOAKUM_SYMBOLS_AVAILABLE=1)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.0.0)
        target_compile_definitions(OakumTestCommon PUBLIC -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=0)
    else()
        # Clang 14 on Ubuntu 22.04 creates a weird binary, which cannot be queried by addr2line for some reason.
        # It returns "DWARF error: invalid or unhandled FORM value: 0x23". Forcing dwarf5 doesn't help. I don't
        # understand. This is hopeless, I'm giving up and just disabling tests, which query source locations.
        target_compile_definitions(OakumTestCommon PUBLIC -DOAKUM_SOURCE_LOCATIONS_AVAILABLE=$<IF:$<CONFIG:Debug>,1,0>)
    endif()
endif()
