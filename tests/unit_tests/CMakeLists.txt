append_sources(OAKUM_UNIT_TESTS_SOURCES OFF)
add_subdirectories()

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${OAKUM_UNIT_TESTS_SOURCES})
add_executable(OakumUnitTests ${OAKUM_UNIT_TESTS_SOURCES})
target_link_libraries(OakumUnitTests PRIVATE OakumTestCommon)
add_test(NAME OakumUnitTests COMMAND OakumUnitTests)
