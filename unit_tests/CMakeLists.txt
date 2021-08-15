file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_executable(OakumUnitTests ${SOURCE_FILES})

set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
DependencyManager_create_dependency(
    googletest
    BUILD_METHOD "in_project"
    DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/googletest
    IS_SUBMODULE
)
DependencyManager_link_to_dependency_library(OakumUnitTests PRIVATE googletest gtest_main)
target_link_libraries(OakumUnitTests PRIVATE Oakum)
target_compile_features(OakumUnitTests PRIVATE cxx_std_17)
target_include_directories(OakumUnitTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
