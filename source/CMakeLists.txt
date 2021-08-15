file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
add_library(Oakum STATIC ${SOURCE_FILES})
target_compile_features(Oakum PRIVATE cxx_std_17)
target_include_directories(Oakum PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(Oakum PUBLIC Dbghelp.lib)
endif()
