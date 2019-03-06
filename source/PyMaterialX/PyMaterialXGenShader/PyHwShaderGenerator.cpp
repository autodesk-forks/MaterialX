//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHwShaderGenerator(py::module& mod)
{
    py::class_<mx::HwClosureContext, mx::GenUserData, mx::HwClosureContextPtr>(mod, "HwClosureContext")
        .def_static("create", &mx::HwClosureContext::create)
        .def(py::init<int>())
        .def("getType", &mx::HwClosureContext::getType)
        .def("addArgument", &mx::HwClosureContext::addArgument)
        .def("getArguments", &mx::HwClosureContext::getArguments)
        .def("setSuffix", &mx::HwClosureContext::setSuffix)
        .def("getSuffix", &mx::HwClosureContext::getSuffix);

    py::enum_<mx::HwClosureContext::Type>(mod, "Type")
        .value("REFLECTION", mx::HwClosureContext::REFLECTION)
        .value("TRANSMISSION", mx::HwClosureContext::TRANSMISSION)
        .value("INDIRECT", mx::HwClosureContext::INDIRECT)
        .value("EMISSION", mx::HwClosureContext::EMISSION)
        .export_values();

    py::class_<mx::HwLightShaders, mx::GenUserData, mx::HwLightShadersPtr>(mod, "HwLightShaders")
        .def_static("create", &mx::HwLightShaders::create)
        .def("bind", &mx::HwLightShaders::bind)
        .def("get", static_cast<const mx::ShaderNode* (mx::HwLightShaders::*)(unsigned int) const>(&mx::HwLightShaders::get))
        .def("get", static_cast<const std::unordered_map<unsigned int, mx::ShaderNodePtr>& (mx::HwLightShaders::*)() const>(&mx::HwLightShaders::get));

    py::class_<mx::HwShaderGenerator, mx::ShaderGenerator, mx::HwShaderGeneratorPtr>(mod, "HwShaderGenerator")
        .def("getNodeClosureContexts", &mx::HwShaderGenerator::getNodeClosureContexts)
        .def("bindLightShader", &mx::HwShaderGenerator::bindLightShader);
}
