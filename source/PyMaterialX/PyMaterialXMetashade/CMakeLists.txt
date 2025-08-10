file(GLOB pymaterialxmetashade_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxmetashade_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Filter out the Python binding file for the C++ library
list(FILTER pymaterialxmetashade_source EXCLUDE REGEX "PyModule\\.cpp$")

# Create C++ static library for linking from other MaterialX libraries
add_library(MaterialXMetashade STATIC 
    MetashadeNode.cpp
    ${pymaterialxmetashade_headers}
)

target_link_libraries(MaterialXMetashade
    PUBLIC MaterialXGenShader MaterialXCore
    PRIVATE pybind11::embed
)

target_include_directories(MaterialXMetashade
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXMetashade>
)

set_target_properties(MaterialXMetashade PROPERTIES
    OUTPUT_NAME MaterialXMetashade
    DEBUG_POSTFIX "${MATERIALX_LIBRARY_DEBUG_POSTFIX}")

# Install the C++ library and add it to the MaterialX export set
install(TARGETS MaterialXMetashade
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH})

install(FILES ${pymaterialxmetashade_headers}
    DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXMetashade/)

# Create Python module
pybind11_add_module(PyMaterialXMetashade SHARED ${PYBIND11_MODULE_FLAGS} 
    PyModule.cpp
    ${pymaterialxmetashade_headers}
)

if(APPLE)
    set_target_properties(PyMaterialXMetashade PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXMetashade
    PROPERTIES
    OUTPUT_NAME PyMaterialXMetashade
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXMetashade
    PUBLIC MaterialXMetashade MaterialXGenShader MaterialXCore
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXMetashade
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
