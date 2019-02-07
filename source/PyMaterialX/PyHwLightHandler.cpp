//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenOptions.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHwLightHandler(py::module& mod)
{
    py::class_<mx::HwLightHandler, mx::HwLightHandlerPtr>(mod, "HwLightHandler")
        .def_static("create", &mx::HwLightHandler::create)
        .def("addLightSource", &mx::HwLightHandler::addLightSource)
        .def("getLightSources", &mx::HwLightHandler::getLightSources)
        .def("setLightSources", &mx::HwLightHandler::setLightSources)
        .def("setLightEnvIrradiancePath", &mx::HwLightHandler::setLightEnvIrradiancePath)
        .def("getLightEnvIrradiancePath", &mx::HwLightHandler::getLightEnvIrradiancePath)
        .def("setLightEnvRadiancePath", &mx::HwLightHandler::setLightEnvRadiancePath)
        .def("getLightEnvRadiancePath", &mx::HwLightHandler::getLightEnvRadiancePath);
}
