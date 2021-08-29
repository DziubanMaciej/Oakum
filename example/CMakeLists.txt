add_executable(OakumExampleSimple "example_simple.cpp")
target_link_libraries(OakumExampleSimple PRIVATE Oakum)

add_executable(OakumExampleStackTraces "example_stack_traces.cpp")
target_link_libraries(OakumExampleStackTraces PRIVATE Oakum)
