//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPortDef.h>

#include <MaterialXRuntime/Private/PrvPortDef.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>

namespace MaterialX
{

RtPortDef::RtPortDef(const RtObject& obj) : 
    RtElement(obj)
{
}

RtObject RtPortDef::create(const RtToken& name, const RtToken& type, const RtValue& value, uint32_t flags, RtObject parent)
{
    PrvObjectHandle portdef = PrvPortDef::create(name, type, value, flags);

    if (parent)
    {
        if (parent.hasApi(RtApiType::NODEDEF))
        {
            PrvObjectHandle parentData = RtApiBase::data(parent);
            parentData->asA<PrvNodeDef>()->addPort(portdef);
        }
        else
        {
            throw ExceptionRuntimeError("Parent object must be a nodedef");
        }
    }

    return RtObject(portdef);
}

RtApiType RtPortDef::getApiType() const
{
    return RtApiType::PORTDEF;
}

const RtToken& RtPortDef::getType() const
{
    return data()->asA<PrvPortDef>()->getType();
}

const RtValue& RtPortDef::getValue() const
{
    return data()->asA<PrvPortDef>()->getValue();
}

RtValue& RtPortDef::getValue()
{
    return data()->asA<PrvPortDef>()->getValue();
}

const RtToken& RtPortDef::getColorSpace() const
{
    return data()->asA<PrvPortDef>()->getColorSpace();
}

const RtToken& RtPortDef::getUnit() const
{
    return data()->asA<PrvPortDef>()->getUnit();
}

void RtPortDef::setValue(const RtValue& v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setValue(bool v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setValue(int v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setValue(float v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setValue(const Color3& v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setValue(const Vector4& v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setValue(void* v)
{
    return data()->asA<PrvPortDef>()->setValue(v);
}

void RtPortDef::setColorSpace(const RtToken& colorspace)
{
    return data()->asA<PrvPortDef>()->setColorSpace(colorspace);
}

void RtPortDef::setUnit(const RtToken& unit)
{
    return data()->asA<PrvPortDef>()->setUnit(unit);
}

int32_t RtPortDef::getFlags() const
{
    return data()->asA<PrvPortDef>()->getFlags();
}

bool RtPortDef::isInput() const
{
    return data()->asA<PrvPortDef>()->isInput();
}

bool RtPortDef::isOutput() const
{
    return data()->asA<PrvPortDef>()->isOutput();
}

bool RtPortDef::isConnectable() const
{
    return data()->asA<PrvPortDef>()->isConnectable();
}

bool RtPortDef::isUniform() const
{
    return data()->asA<PrvPortDef>()->isUniform();
}

}
