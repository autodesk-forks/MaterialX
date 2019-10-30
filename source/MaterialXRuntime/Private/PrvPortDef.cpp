//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvPortDef.h>

namespace MaterialX
{

const RtToken PrvPortDef::DEFAULT_OUTPUT_NAME("out");

PrvPortDef::PrvPortDef(const RtToken& name, const RtToken& type, const RtValue& value, uint32_t flags) :
PrvElement(RtObjType::PORTDEF, name),
_type(type),
_value(value),
_colorspace(EMPTY_TOKEN),
_unit(EMPTY_TOKEN),
_flags(flags)
{
}

PrvObjectHandle PrvPortDef::createNew(const RtToken& name, const RtToken& type, const RtValue& value, uint32_t flags)
{
    return std::make_shared<PrvPortDef>(name, type, value, flags);
}

bool PrvPortDef::canConnectTo(const PrvPortDef* other) const
{
    // TODO: Check if the data types are compatible.
    return isConnectable() && other->isConnectable() && // both must be connectable
           isOutput() != other->isOutput();             // they must be of different kinds
}

}
