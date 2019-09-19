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
* @file Units.cpp
* @brief Units support
*/


#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Units.h>

namespace MaterialX
{

LengthUnitConverter::LengthUnitConverter(UnitTypeDefPtr unitTypeDef)
{
    // Build a unit scale map for each UnitDef. 
    vector<UnitDefPtr> unitDefs = unitTypeDef->getUnitDefs();
    for (UnitDefPtr unitdef : unitDefs)
    {
        const string& name = unitdef->getName();
        if (!name.empty())
        {
            const string& scaleString = unitdef->getAttribute("scale");
            if (!scaleString.empty())
            {
                ValuePtr scaleValue = Value::createValueFromStrings(scaleString, getTypeString<float>());
                _unitScale[name] = scaleValue->asA<float>();
            }
        }
    }
}

UnitConverterPtr LengthUnitConverter::create(UnitTypeDefPtr unitTypeDef)
{
    std::shared_ptr<LengthUnitConverter> converter(new LengthUnitConverter(unitTypeDef));
    return converter;
}

float LengthUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
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
