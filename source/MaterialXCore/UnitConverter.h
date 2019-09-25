//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

/**
* Copyright 2019 Autodesk, Inc.
* All rights reserved.
*
* This computer source code and related instructions and comments are the
* unpublished confidential and proprietary information of Autodesk, Inc. and
* are protected under Federal copyright and state trade secret law. They may
* not be disclosed to, copied or used by any third party without the prior
* written consent of Autodesk, Inc.
*
* @file UnitConverter.h
* @brief Unit converter classes.
*/

#ifndef MATERIALX_UNITCONVERTER_H_
#define MATERIALX_UNITCONVERTER_H_

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Types.h>

namespace MaterialX
{

class LengthUnitConverter;

/// A shared pointer to an LengthUnitConverter
using LengthUnitConverterPtr = shared_ptr<LengthUnitConverter>;
/// A shared pointer to a const LengthUnitConverter
using ConstLengthUnitConverterPtr = shared_ptr<const LengthUnitConverter>;

/// @class LLengthUnitConverter
/// An unit conversion utility for handling length.
/// - The base unit for conversion is "meter".
/// - The following units are supported by default:
///     - "nanometer"
///     - "micron", 
///     - "millimeter",
///     - "centimeter",
///     - "meter",
///     - "kilometer", 
///     - "foot", 
///     - "inch", 
///     - "yard", 
///     - "mile"
///
class LengthUnitConverter
{
  public:
    virtual ~LengthUnitConverter() { }

    /// Creator 
    static LengthUnitConverterPtr create();

    /// Set a new unit conversions to meters
    void setMetersPerUnit(const string& unit, float metersPerUnit);

    /// get the unit conversion to meters
    float getMetersPerUnit(const string& unit) const;

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    float convert(float input, const string& inputUnit, const string& outputUnit) const;

    /// Return base unit used for conversion
    const string& getBaseUnit() const
    {
        return _baseUnit;
    }

  private:
    LengthUnitConverter();

    string _baseUnit;
    std::unordered_map<string, float> _unitScale;
};

}  // namespace MaterialX

#endif  // MATERIALX_UNITCONVERTER_H_