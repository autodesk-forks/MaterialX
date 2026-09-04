file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.c*")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

find_package(Vulkan REQUIRED)
find_package(glslang CONFIG REQUIRED)

mx_add_library(MaterialXRenderVk
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXRender
        MaterialXGenGlsl
    EXPORT_DEFINE
        MATERIALX_RENDERVK_EXPORTS)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        Vulkan::Vulkan
    PRIVATE
        glslang::glslang
        glslang::SPIRV
        glslang::glslang-default-resource-limits)

target_compile_definitions(${TARGET_NAME}
    PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    PRIVATE
        $<$<BOOL:${WIN32}>:NOMINMAX>)
