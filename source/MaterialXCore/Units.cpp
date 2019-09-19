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
    static const string SCALE_ATTRIBUTE = "scale";

    // Build a unit scale map for each UnitDef. 
    vector<UnitDefPtr> unitDefs = unitTypeDef->getUnitDefs();
    for (UnitDefPtr unitdef : unitDefs)
    {
        const string& name = unitdef->getName();
        if (!name.empty())
        {
            const string& scaleString = unitdef->getAttribute(SCALE_ATTRIBUTE);
            if (!scaleString.empty())
            {
                ValuePtr scaleValue = Value::createValueFromStrings(scaleString, getTypeString<float>());
                _unitScale[name] = scaleValue->asA<float>();
            }
        }
    }

    // In case the default unit was not specified in the unittypedef explicit
    // add this to be able to accept converstion with the default 
    // as the output unit
    const string& defaultUnit = unitTypeDef->getDefault();
    auto it = _unitScale.find(defaultUnit);
    if (it == _unitScale.end())
    {
        _unitScale[defaultUnit] = 1.0f;
    }
}

UnitConverterPtr LengthUnitConverter::create(UnitTypeDefPtr unitTypeDef)
{
    std::shared_ptr<LengthUnitConverter> converter(new LengthUnitConverter(unitTypeDef));
    return converter;
}

float LengthUnitConverter::convert(float input, const string& inputUnit, const string& outputUnit) const
{
    if (inputUnit == outputUnit)
    {
        return 1.0f;
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
