//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShader(py::module& mod)
{
    py::class_<mx::Shader, mx::ShaderPtr>(mod, "Shader")
        .def(py::init<const std::string&, mx::ShaderGraphPtr>())
        .def("getName", &mx::Shader::getName)
        .def("numStages", &mx::Shader::numStages)
        .def("getStage", static_cast<mx::ShaderStage& (mx::Shader::*)(size_t)>(&mx::Shader::getStage))
        .def("getStage", static_cast<mx::ShaderStage& (mx::Shader::*)(const std::string&)>(&mx::Shader::getStage))
        .def("getSourceCode", &mx::Shader::getSourceCode)
        .def("hasAttribute", &mx::Shader::hasAttribute)
        .def("getAttribute", &mx::Shader::getAttribute)
        .def("setAttribute", static_cast<void (mx::Shader::*)(const std::string&)>(&mx::Shader::setAttribute))
        .def("setAttribute", static_cast<void (mx::Shader::*)(const std::string&, mx::ValuePtr)>(&mx::Shader::setAttribute));
}
