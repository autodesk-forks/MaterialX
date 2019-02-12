file(GLOB pymaterialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialX SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialx_source} ${pymaterialx_headers})

if(APPLE)
    set_target_properties(PyMaterialX PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialX
    PROPERTIES
    OUTPUT_NAME PyMaterialX
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    PyMaterialX
    PUBLIC MaterialXCore
           MaterialXFormat
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialX
        DESTINATION "${CMAKE_INSTALL_PREFIX}/python/MaterialX")
