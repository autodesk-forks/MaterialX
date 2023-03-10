add_custom_target(Contrib_libraries)

add_custom_command(TARGET Contrib_libraries
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/adsklib ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libraries/adsklib)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}" MESSAGE_NEVER
        PATTERN "CMakeLists.txt" EXCLUDE)
