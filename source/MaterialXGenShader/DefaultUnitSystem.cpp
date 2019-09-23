//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/DefaultUnitSystem.h>

#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const string DefaultUnitSystem::UNITSYTEM_NAME = "default_unit_system";

//
// DefaultUnitSystem methods
//

string DefaultUnitSystem::getImplementationName(const UnitTransform& transform) const
{
    return "IM_" + LengthUnitConverter::LENGTH_UNIT + "_unit_" + transform.type->getName() + "_" + _language;
}

DefaultUnitSystemPtr DefaultUnitSystem::create(const string& language)
{
    DefaultUnitSystemPtr result(new DefaultUnitSystem(language));
    return result;
}

DefaultUnitSystem::DefaultUnitSystem(const string& language)
    : UnitSystem()
{
    _language = createValidName(language);
}

} // namespace MaterialX
