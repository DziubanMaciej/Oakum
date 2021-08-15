file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_executable(OakumUnitTests ${SOURCE_FILES})

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
DependencyManager_create_dependency(
    googletest
    BUILD_METHOD "out_of_project"
    DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/googletest
    IS_SUBMODULE
    ALL_BUILD_TYPES Debug
    SELECTED_BUILD_TYPE Debug
    CMAKE_ARGS "-DBUILD_GMOCK=OFF -DINSTALL_GTEST=OFF -DBUILD_SHARED_LIBS=ON"
)
DependencyManager_link_to_dependency_library(OakumUnitTests PRIVATE googletest gtest)
target_link_libraries(OakumUnitTests PRIVATE Oakum)
target_compile_features(OakumUnitTests PRIVATE cxx_std_17)
target_include_directories(OakumUnitTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
