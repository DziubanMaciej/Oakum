set(OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(DOXYGEN_LOG_FILE "${OUT_DIR}/doxygen_log.txt")
set(DOXYFILE_NAME "Doxyfile.txt")

add_custom_target (OakumGenerateDocs)

add_custom_command(TARGET OakumGenerateDocs POST_BUILD
    COMMAND echo Creating directory: ${OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUT_DIR}
    COMMAND echo Running Doxygen
    COMMAND ${OAKUM_DOXYGEN_COMMAND} ${DOXYFILE_NAME} >${DOXYGEN_LOG_FILE} 2>&1
    COMMAND echo Generated documentation to ${OUT_DIR}/html/index.html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
