file(GLOB OAKUM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
add_os_directory()

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${OAKUM_SOURCES})
add_library(Oakum STATIC ${OAKUM_SOURCES})
target_compile_features(Oakum PRIVATE cxx_std_17)
target_include_directories(Oakum PRIVATE ${OAKUM_SOURCE_DIR})
target_include_directories(Oakum INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(NOT OAKUM_MAX_STACK_FRAMES_COUNT STREQUAL "")
    target_compile_definitions(Oakum PUBLIC -DOAKUM_MAX_STACK_FRAMES_COUNT=${OAKUM_MAX_STACK_FRAMES_COUNT})
endif()
if(WIN32)
    target_link_libraries(Oakum PUBLIC Dbghelp.lib)
endif()
if(MSVC)
    target_compile_options(Oakum PUBLIC "/MP")
endif()
if (UNIX)
    target_link_libraries(Oakum PUBLIC -rdynamic -ldl)
endif()
