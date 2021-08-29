add_executable(OakumExampleSimple "example_simple.cpp")
target_link_libraries(OakumExampleSimple PRIVATE Oakum)

add_executable(OakumExampleStackTraces "example_stack_traces.cpp")
target_link_libraries(OakumExampleStackTraces PRIVATE Oakum)

add_executable(OakumExampleGtest "example_gtest.cpp")
target_link_libraries(OakumExampleGtest PRIVATE Oakum)
DependencyManager_link_to_dependency_library(OakumExampleGtest PRIVATE googletest gtest)
