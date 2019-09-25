//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/UnitConverter.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUnitConverters(py::module& mod)
{
    py::class_<mx::LengthUnitConverter, mx::LengthUnitConverterPtr>(mod, "LengthUnitConverter")
        .def_static("create", &mx::LengthUnitConverter::create)
        .def("setMetersPerUnit", &mx::LengthUnitConverter::setMetersPerUnit)
        .def("getMetersPerUnit", &mx::LengthUnitConverter::getMetersPerUnit)
        .def("getBaseUnit", &mx::LengthUnitConverter::getBaseUnit)
        .def("convert", &mx::LengthUnitConverter::convert);
}
