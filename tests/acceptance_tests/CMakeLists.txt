append_sources(OAKUM_ACCEPTANCE_TESTS_SOURCES OFF)
add_subdirectories()

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${OAKUM_ACCEPTANCE_TESTS_SOURCES})
add_executable(OakumAcceptanceTests ${OAKUM_ACCEPTANCE_TESTS_SOURCES})
target_link_libraries(OakumAcceptanceTests PRIVATE OakumTestCommon)
add_test(NAME OakumAcceptanceTests COMMAND OakumAcceptanceTests)
