if (OAKUM_BUILD_EXAMPLES OR OAKUM_BUILD_TESTS)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    if (UNIX)
        find_package(Threads)
    endif()

    DependencyManager_create_dependency(
        googletest
        BUILD_METHOD "in_project"
        DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/googletest
        IS_SUBMODULE
    )
endif()
