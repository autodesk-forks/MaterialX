//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvElement.h>
#include <MaterialXRuntime/Private/PrvNode.h>
#include <MaterialXRuntime/Private/PrvNodeDef.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

const string PrvElement::PATH_SEPARATOR = "/";

PrvElement::PrvElement(RtObjType objType, const RtToken& name) :
    PrvObject(objType),
    _name(name)
{
}

void PrvElement::initialize()
{
    clearAttributes();
    clearChildren();
}

void PrvElement::addChild(PrvObjectHandle elem)
{
    if (!elem->hasApi(RtApiType::ELEMENT))
    {
        throw ExceptionRuntimeError("Given object is not a valid element");
    }

    PrvElement* el = elem->asA<PrvElement>();
    if (_childrenByName.count(el->getName()))
    {
        throw ExceptionRuntimeError("An element named '" + el->getName().str() + "' already exists in '" + getName().str() + "'");
    }

    _children.push_back(elem);
    _childrenByName[el->getName()] = elem;
}

void PrvElement::removeChild(const RtToken& name)
{
    for (auto it = _children.begin(); it != _children.end(); ++it)
    {
        if ((*it)->asA<PrvElement>()->getName() == name)
        {
            _children.erase(it);
            break;
        }
    }
    _childrenByName.erase(name);
}

PrvObjectHandle PrvElement::findChildByName(const RtToken& name) const
{
    auto it = _childrenByName.find(name);
    return it != _childrenByName.end() ? it->second : nullptr;
}

PrvObjectHandle PrvElement::findChildByPath(const string& path) const
{
    const StringVec elementNames = splitString(path, PATH_SEPARATOR);
    if (elementNames.empty())
    {
        return nullptr;
    }

    size_t i = 0;
    RtToken name = RtToken(elementNames[i++]);
    PrvObjectHandle elem = findChildByName(name);

    while (elem != nullptr && i < elementNames.size())
    {
        name = RtToken(elementNames[i++]);
        if (elem->hasApi(RtApiType::NODE))
        {
            PrvNode* node = elem->asA<PrvNode>();
            PrvNodeDef* nodedef = node->getNodeDef()->asA<PrvNodeDef>();
            elem = nodedef->findChildByName(name);
        }
        else
        {
            elem = elem->asA<PrvElement>()->findChildByName(name);
        }
    }

    return elem;
}

void PrvElement::clearChildren()
{
    _children.clear();
    _childrenByName.clear();
}

void PrvElement::addAttribute(const RtToken& name, const RtToken& type, const RtValue& value)
{
    auto it = _attributesByName.find(name);
    if (it != _attributesByName.end())
    {
        throw ExceptionRuntimeError("An attribute named '" + name.str() + "' already exists for '" + getName().str() + "'");
    }

    AttrPtr attr(new RtAttribute(name, type, value));
    _attributes.push_back(attr);
    _attributesByName[name] = attr;
}

void PrvElement::removeAttribute(const RtToken& name)
{
    for (auto it = _attributes.begin(); it != _attributes.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            _attributes.erase(it);
            break;
        }
    }
    _attributesByName.erase(name);
}

void PrvElement::clearAttributes()
{
    _attributes.clear();
    _attributesByName.clear();
}

PrvUnknown::PrvUnknown(const RtToken& name, const RtToken& category) :
    PrvElement(RtObjType::UNKNOWN, name),
    _category(category)
{
}

PrvObjectHandle PrvUnknown::createNew(const RtToken& name, const RtToken& category)
{
    return std::make_shared<PrvUnknown>(name, category);
}

}
