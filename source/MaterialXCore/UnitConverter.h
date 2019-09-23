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

class UnitConverter;

/// A shared pointer to an UnitConverter
using UnitConverterPtr = shared_ptr<UnitConverter>;
/// A shared pointer to a const UnitConverter
using ConstUnitConverterPtr = shared_ptr<const UnitConverter>;

/// @class UnitConverter
/// An unit conversion utility class.
///
/// An converter can be registered with a supporting document for a given UnitTypeDef.
class UnitConverter
{
  public:
    UnitConverter() { }
    virtual ~UnitConverter() { }

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    virtual float convert(float input, const string& inputUnit, const string& outputUnit) const = 0;
};


class LengthUnitConverter;

/// A shared pointer to an LengthUnitConverter
using LengthUnitConverterPtr = shared_ptr<LengthUnitConverter>;
/// A shared pointer to a const LengthUnitConverter
using ConstLengthUnitConverterPtr = shared_ptr<const LengthUnitConverter>;

/// @class LLengthUnitConverter
/// An unit conversion utility for handling length.
///
class LengthUnitConverter : public UnitConverter
{
  public:
    virtual ~LengthUnitConverter() { }

    /// Creator 
    static UnitConverterPtr create(UnitTypeDefPtr unitTypeDef);

    /// Return the mappings from unit names to the scale value
    /// defined by the "length" UnitTypeDef. Multiplying a
    /// value by the scal will convert it to a value in the default unit.
    /// Dividing will convert from the default unit to a given unit.
    const std::unordered_map<string, float>& getUnitScale() const
    {
        return _unitScale;
    }

    /// Return the name of the default unit for "length"
    const string& getGefaultUnit() const
    {
        return _defaultUnit;
    }

    /// Convert a given value in a given unit to a desired unit
    /// @param input Input value to convert
    /// @param inputUnit Unit of input value
    /// @param outputUnit Unit for output value
    float convert(float input, const string& inputUnit, const string& outputUnit) const override;

    static const string LENGTH_UNIT;

  private:
    LengthUnitConverter(UnitTypeDefPtr unitTypeDef);

    string _defaultUnit;
    std::unordered_map<string, float> _unitScale;
};

}  // namespace MaterialX

#endif  // MATERIALX_UNITCONVERTER_H_