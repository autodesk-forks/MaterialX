if (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/NanoGUI/ext/glfw/src")
    message(FATAL_ERROR "Building the MaterialX viewer requires the NanoGUI submodule "
        "to be present. Update your repository by calling the following:\n"
        "git submodule update --init --recursive")
endif()

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL OFF CACHE BOOL " " FORCE)

# Turn off some additional warnings in NanoGUI dependents
set(PREV_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(PREV_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4389")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4389")
else()
    add_compile_options(-Wno-misleading-indentation)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-misleading-indentation")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-misleading-indentation")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/NanoGUI external/NanoGUI)
set_property(TARGET nanogui nanogui-obj glfw glfw_objects PROPERTY FOLDER "External")

set(CMAKE_C_FLAGS ${PREV_CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS ${PREV_CMAKE_CXX_FLAGS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/NanoGUI/include
    ${NANOGUI_EXTRA_INCS})

add_definitions(${NANOGUI_EXTRA_DEFS})

if(MSVC)
    add_compile_options(/D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS )
endif()

add_executable(MaterialXView ${materialx_source} ${materialx_headers})

target_link_libraries(
    MaterialXView
    MaterialXFormat
    MaterialXGenGlsl
    MaterialXRenderGlsl
    nanogui
    ${NANOGUI_EXTRA_LIBS})

add_custom_command(TARGET MaterialXView POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/libraries ${CMAKE_CURRENT_BINARY_DIR}/libraries)
add_custom_command(TARGET MaterialXView POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources)

install(TARGETS MaterialXView
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libraries/
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/libraries")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources/
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/resources")
