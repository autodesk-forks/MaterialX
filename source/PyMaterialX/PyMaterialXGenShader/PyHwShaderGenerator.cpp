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
    mod.attr("VERTEX_STAGE") = mx::Stage::VERTEX;

    mod.attr("VERTEX_INPUTS") = mx::HW::VERTEX_INPUTS;
    mod.attr("VERTEX_DATA") = mx::HW::VERTEX_DATA;
    mod.attr("PRIVATE_UNIFORMS") = mx::HW::PRIVATE_UNIFORMS;
    mod.attr("PUBLIC_UNIFORMS") = mx::HW::PUBLIC_UNIFORMS;
    mod.attr("LIGHT_DATA") = mx::HW::LIGHT_DATA;
    mod.attr("PIXEL_OUTPUTS") = mx::HW::PIXEL_OUTPUTS;
    mod.attr("NORMAL_DIR") = mx::HW::NORMAL_DIR;
    mod.attr("LIGHT_DIR") = mx::HW::LIGHT_DIR;
    mod.attr("VIEW_DIR") = mx::HW::VIEW_DIR;
    mod.attr("ATTR_TRANSPARENT") =  mx::HW::ATTR_TRANSPARENT;

    py::class_<mx::HwShaderGenerator, mx::ShaderGenerator, mx::HwShaderGeneratorPtr>(mod, "HwShaderGenerator")
        .def("getNodeClosureContexts", &mx::HwShaderGenerator::getNodeClosureContexts)
        .def("bindLightShader", &mx::HwShaderGenerator::bindLightShader);
}
