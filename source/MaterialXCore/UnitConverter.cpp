//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

/**
* Copyright 2019 Autodesk, Inc.
* All rights reserved.
*
* This computer source code and related instructions and comments are the unpublished confidential
* and proprietary information of Autodesk, Inc. and are protected under Federal copyright and state
* trade secret law. They may not be disclosed to, copied or used by any third party without the
* prior written consent of Autodesk, Inc.
*
* @file UnitConverter.cpp
* @brief Support for unit conversion
*/


#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/UnitConverter.h>

namespace MaterialX
{

LengthUnitConverter::LengthUnitConverter()
{
    static const string BASE_UNIT_NAME = "meter";
    _baseUnit = BASE_UNIT_NAME;

    setMetersPerUnit("nanometer", 1000000000.0f);
    setMetersPerUnit("micron", 1000000.0f);
    setMetersPerUnit("millimeter", 1000.0f);
    setMetersPerUnit("centimeter", 100.0f);
    setMetersPerUnit(BASE_UNIT_NAME, 1.0f);
    setMetersPerUnit("kilometer", 0.001f);
    setMetersPerUnit("foot", 3.281f);
    setMetersPerUnit("inch", 39.37f);
    setMetersPerUnit("yard", 1.093613f);
    setMetersPerUnit("mile", 0.000621f);    
}

LengthUnitConverterPtr LengthUnitConverter::create()
{
    std::shared_ptr<LengthUnitConverter> converter(new LengthUnitConverter());
    return converter;
}

void LengthUnitConverter::setMetersPerUnit(const string& unit, float metersPerUnit)
{
    _unitScale[unit] = metersPerUnit;
}

float LengthUnitConverter::getMetersPerUnit(const string& unit) const
{
    auto it = _unitScale.find(unit);
    if (it != _unitScale.end())
    {
        return it->second;
    }
    throw ExceptionTypeError("Unrecognized unit: " + unit);
}

float LengthUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return input;
    }

    auto it = _unitScale.find(inputUnit);
    if (it == _unitScale.end())
    {
        throw ExceptionTypeError("Unrecognized source unit: " + inputUnit);
    }
    float fromScale = it->second;

    it = _unitScale.find(outputUnit);
    if (it == _unitScale.end())
    {
        throw ExceptionTypeError("Unrecognized destination unit: " + outputUnit);
    }
    float toScale = it->second;

    return (input * fromScale / toScale);
}

}
