//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGenContext(py::module& mod)
{
    py::class_<mx::GenUserData, mx::GenUserDataPtr>(mod, "GenUserData")
        .def("getSelf", static_cast<mx::GenUserDataPtr (mx::GenUserData::*)()>(&mx::GenUserData::getSelf))
        .def("asA", static_cast<mx::HwClosureContextPtr (mx::GenUserData::*)()>(&mx::GenUserData::asA))
        .def("asA", static_cast<mx::HwLightShadersPtr (mx::GenUserData::*)()>(&mx::GenUserData::asA));

    py::class_<mx::GenContext, mx::GenContextPtr>(mod, "GenContext")
        .def(py::init<mx::ShaderGeneratorPtr>())
        .def("getShaderGenerator", &mx::GenContext::getShaderGenerator)
        .def("getOptions", static_cast<mx::GenOptions& (mx::GenContext::*)()>(&mx::GenContext::getOptions))
        .def("registerSourceCodeSearchPath", static_cast<void (mx::GenContext::*)(const std::string&)>(&mx::GenContext::registerSourceCodeSearchPath))
        .def("registerSourceCodeSearchPath", static_cast<void (mx::GenContext::*)(const mx::FilePath&)>(&mx::GenContext::registerSourceCodeSearchPath))
        .def("registerSourceCodeSearchPath", static_cast<void (mx::GenContext::*)(const mx::FileSearchPath&)>(&mx::GenContext::registerSourceCodeSearchPath))
        .def("findSourceCode", &mx::GenContext::findSourceCode)
        .def("sourceCodeSearchPath", &mx::GenContext::sourceCodeSearchPath)
        .def("addNodeImplementation", &mx::GenContext::addNodeImplementation)
        .def("findNodeImplementation", &mx::GenContext::findNodeImplementation)
        .def("pushUserData", &mx::GenContext::pushUserData)
        .def("popUserData", &mx::GenContext::popUserData)
        .def("getUserData", static_cast<mx::HwClosureContextPtr (mx::GenContext::*)(const std::string&)>( &mx::GenContext::getUserData))
        .def("getUserData", static_cast<mx::HwLightShadersPtr (mx::GenContext::*)(const std::string&)>( &mx::GenContext::getUserData))
        .def("addInputSuffix", &mx::GenContext::addInputSuffix)
        .def("removeInputSuffix", &mx::GenContext::removeInputSuffix)
        .def("getInputSuffix", &mx::GenContext::getInputSuffix)
        .def("addOutputSuffix", &mx::GenContext::addOutputSuffix)
        .def("removeOutputSuffix", &mx::GenContext::removeOutputSuffix)
        .def("getOutputSuffix", &mx::GenContext::getOutputSuffix);
}
