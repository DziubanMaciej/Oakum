append_sources(OAKUM_UNIT_TESTS_SOURCES OFF)
add_subdirectories()

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${OAKUM_UNIT_TESTS_SOURCES})
add_executable(OakumUnitTests ${OAKUM_UNIT_TESTS_SOURCES})
DependencyManager_link_to_dependency_library(OakumUnitTests PRIVATE googletest gtest)
target_link_libraries(OakumUnitTests PRIVATE Oakum)
target_compile_features(OakumUnitTests PRIVATE cxx_std_17)
target_include_directories(OakumUnitTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${OAKUM_SOURCE_DIR})
