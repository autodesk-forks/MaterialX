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
* @file Units.h
* @brief Units declaration.
*/

#ifndef MATERIALX_UNITS_H_
#define MATERIALX_UNITS_H_

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Types.h>

namespace MaterialX
{

class UnitConverter;

/// A shared pointer to an UnitConverter
using UnitConverterPtr = shared_ptr<UnitConverter>;
/// A shared pointer to a const Observer
using ConstUnitConverterPtr = shared_ptr<const UnitConverter>;

/// @class UnitConverter
/// An unit conversoin utility class.
///
/// An convert may be registered with a supporting document for a given UnitTypeDef.
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


class LengthUnitConverter : public UnitConverter
{
  public:
    virtual ~LengthUnitConverter() { }

    float convert(float input, const string& inputUnit, const string& outputUnit) const override;

    static UnitConverterPtr create(UnitTypeDefPtr unitTypeDef);

  private:
    LengthUnitConverter(UnitTypeDefPtr unitTypeDef);

    std::unordered_map<string, float> _unitScale;
};

}  // namespace MaterialX

#endif  // MATERIALX_UNITS_H_