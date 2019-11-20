//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtElement.h>

#include <MaterialXRuntime/Private/PrvElement.h>

namespace MaterialX
{

RtAttribute::RtAttribute(const RtToken& name, const RtToken& type, RtObject parent) :
    _name(name),
    _type(type),
    _value(RtValue::createNew(type, parent))
{
}

string RtAttribute::getValueString() const
{
    string dest;
    RtValue::toString(getType(), _value, dest);
    return dest;
}

void RtAttribute::setValueString(const string& v)
{
    RtValue::fromString(getType(), v, _value);
}

RtElement::RtElement(const RtObject& obj)
{
    setObject(obj);
}

RtApiType RtElement::getApiType() const
{
    return RtApiType::ELEMENT;
}

const RtToken& RtElement::getName() const
{
    return data()->asA<PrvElement>()->getName();
}

RtAttribute* RtElement::addAttribute(const RtToken& name, const RtToken& type)
{
    PrvElement* elem = data()->asA<PrvElement>();
    return elem->addAttribute(name, type);
}

const RtAttribute* RtElement::getAttribute(const RtToken& name) const
{
    PrvElement* elem = data()->asA<PrvElement>();
    return elem->getAttribute(name);
}

RtAttribute* RtElement::getAttribute(const RtToken& name)
{
    PrvElement* elem = data()->asA<PrvElement>();
    return elem->getAttribute(name);
}

const RtAttribute* RtElement::getAttribute(size_t index) const
{
    PrvElement* elem = data()->asA<PrvElement>();
    return elem->getAttribute(index);
}

RtAttribute* RtElement::getAttribute(size_t index)
{
    PrvElement* elem = data()->asA<PrvElement>();
    return elem->getAttribute(index);
}

size_t RtElement::numAttributes() const
{
    return data()->asA<PrvElement>()->numAttributes();
}

RtTreeIterator RtElement::traverseTree(RtTraversalFilter filter)
{
    return RtTreeIterator(getObject(), filter);
}

}
