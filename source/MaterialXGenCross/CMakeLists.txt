file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

include(FetchContent)
# Override these values if needed
set(GLSLANG_GIT_REPO     https://github.com/KhronosGroup/glslang CACHE STRING "glslang Git repo")
set(SPIRVCROSS_GIT_REPO  https://github.com/KhronosGroup/SPIRV-Cross  CACHE STRING "spirv-cross Git repo")


#
# GLSLANG Configuration
#
FetchContent_Declare(
        glslang
        GIT_REPOSITORY ${GLSLANG_GIT_REPO}
        GIT_TAG master
)
set(glslang_BUILD_EXTERNAL OFF)
set(glslang_CMAKE_DEBUG_POSTFIX d)
set(glslang_ENABLE_HLSL OFF)
set(glslang_ENABLE_CTEST OFF)
FetchContent_MakeAvailable(glslang)

#
# SPIRV-CROSS Configuration
#
FetchContent_Declare(
        SPIRV-Cross
        GIT_REPOSITORY ${SPIRVCROSS_GIT_REPO}
        GIT_TAG 2021-01-15
)
set(SPIRV-Cross_CMAKE_DEBUG_POSTFIX d)
set(SPIRV-Cross_SPIRV_CROSS_ENABLE_TESTS OFF)
set(SPIRV-Cross_SPIRV_CROSS_ENABLE_GLSL OFF)
set(SPIRV-Cross_SPIRV_CROSS_ENABLE_CPP OFF)
set(SPIRV-Cross_SPIRV_CROSS_ENABLE_C_API OFF)
set(SPIRV-Cross_SPIRV_CROSS_ENABLE_UTIL OFF)
FetchContent_MakeAvailable(SPIRV-Cross)

add_library(MaterialXGenCross ${materialx_source} ${materialx_header})
add_definitions(-DMATERIALX_GENCROSS_EXPORTS)

#add_dependencies(MaterialXGenCross glslang)
#add_dependencies(MaterialXGenCross SPIRV-Cross)

set_target_properties(
    MaterialXGenCross PROPERTIES
    OUTPUT_NAME MaterialXGenCross
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXGenCross
    glslang OSDependent OGLCompiler HLSL SPIRV
    spirv-cross-core spirv-cross-hlsl
    ${CMAKE_DL_LIBS})

target_include_directories(
    MaterialXGenCross
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
)

# install
#glslang SPIRV HLSL OGLCompiler OSDependent
install(TARGETS glslang SPIRV OGLCompiler OSDependent HLSL
    EXPORT glslang
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)
#[[
install(TARGETS SPIRV
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(TARGETS OGLCompiler
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(TARGETS OSDependent
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(TARGETS HLSL
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)    
]]    
install(TARGETS MaterialXGenCross
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXGenCross/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXGenCross.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)

