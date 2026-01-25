file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

set(MTLX_MODULES MaterialXGenHw MaterialXCore)

if(MATERIALX_BUILD_METASHADE)
    list(APPEND MTLX_MODULES MaterialXMetashade)
endif()

mx_add_library(MaterialXGenGlsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        ${MTLX_MODULES}
    EXPORT_DEFINE
        MATERIALX_GENGLSL_EXPORTS)
