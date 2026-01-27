# Copy adsk resoures to output

add_custom_target(Contrib_resources ALL)

add_custom_command(TARGET Contrib_resources
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

# copy Fusion, Revit and Proceduralwood examples to TestSuite for testing
add_custom_command(TARGET Contrib_resources
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Materials/Examples/Fusion ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/Materials/TestSuite/adsklib/Fusion)

add_custom_command(TARGET Contrib_resources
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Materials/Examples/Revit ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/Materials/TestSuite/adsklib/Revit)

add_custom_command(TARGET Contrib_resources
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Materials/Examples/Proceduralwood ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/Materials/TestSuite/adsklib/Proceduralwood)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "resources" MESSAGE_NEVER)
