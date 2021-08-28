file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_executable(OakumExample ${SOURCE_FILES})
target_link_libraries(OakumExample PRIVATE Oakum)
