file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/khronos-tools)

# Override these values if needed
set(GLSLANG_GIT_REPO     https://github.com/KhronosGroup/glslang CACHE STRING "glslang Git repo")
set(SPRIVCROSS_GIT_REPO  https://github.com/KhronosGroup/SPIRV-Cross  CACHE STRING "spirv-cross Git repo")

#
# GLSLANG Configuration
#
ExternalProject_Add(glslang
    GIT_REPOSITORY   ${GLSLANG_GIT_REPO}
    GIT_TAG master-tot
    GIT_PROGRESS OFF
    CMAKE_ARGS
        -DBUILD_EXTERNAL=OFF
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_COMMAND "" # Skip updates
)

#
# SPIRV-CROSS Configuration
# 
ExternalProject_Add(spriv-cross
    GIT_REPOSITORY  ${SPRIVCROSS_GIT_REPO}
    GIT_TAG 2021-01-15
    GIT_PROGRESS OFF
    CMAKE_ARGS
        -DCMAKE_DEBUG_POSTFIX=d
        -DSPIRV_CROSS_ENABLE_TESTS=OFF
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_COMMAND "" # Skip updates
)

add_library(MaterialXGenCross ${materialx_source} ${materialx_header})
add_dependencies(MaterialXGenCross glslang)
add_dependencies(MaterialXGenCross spriv-cross)

set_target_properties(
    MaterialXGenCross PROPERTIES
    OUTPUT_NAME MaterialXGenCross
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

#target_link_libraries(
#    MaterialXGenCross 
#    ${CMAKE_DL_LIBS})

target_include_directories(MaterialXGenCross
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
        ${EXTERNAL_INSTALL_LOCATION}/include
        ${MATERIALX_SPIRV_CROSS_DIR}
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS}
)

# install
install(TARGETS MaterialXGenCross
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

# install Khronos tools to MaterialX install        
install(DIRECTORY ${EXTERNAL_INSTALL_LOCATION} DESTINATION ".")        
