//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include "MetashadeNode.h"
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMetashadeNode(py::module& mod)
{
    py::class_<mx::MetashadeNode>(mod, "MetashadeNode")
        .def_static("create", &mx::MetashadeNode::create)
        .def_static("registerImplementations", &mx::MetashadeNode::registerImplementations,
            py::arg("generator"));
}

PYBIND11_MODULE(PyMaterialXMetashade, mod)
{
    mod.doc() = "MaterialX Metashade Python bindings";

    // Import dependencies
    PYMATERIALX_IMPORT_MODULE(PyMaterialXCore);
    PYMATERIALX_IMPORT_MODULE(PyMaterialXGenShader);

    // Bind MetashadeNode
    bindPyMetashadeNode(mod);
}
