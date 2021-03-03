//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtStage.h>

#include <set>

/// @file
/// TODO: Docs

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtObject, RtObjType::OBJECT, "PvtObject")
RT_DEFINE_REF_PTR_FUNCTIONS(PvtObject)

PvtObject::PvtObject(const RtToken& name, PvtPrim* parent) :
    _typeBits(0),
    _name(name),
    _parent(parent)
{
    setTypeBit<PvtObject>();
}

PvtPath PvtObject::getPath() const
{
    return PvtPath(this);
}

PvtPrim* PvtObject::getRoot() const
{
    PvtPrim* root = isA<PvtPrim>() ? const_cast<PvtPrim*>(asA<PvtPrim>()) : _parent;
    while (root->_parent)
    {
        root = root->_parent;
    }
    return root;
}

RtStageWeakPtr PvtObject::getStage() const
{
    return getRoot()->asA<PvtStage::RootPrim>()->getStage();
}

RtTypedValue* PvtObject::createAttribute(const RtToken& name, const RtToken& type)
{
    RtTypedValue* value = getAttribute(name, type);
    if (value)
    {
        return value;
    }

    PvtPrim* prim = isA<PvtPrim>() ? asA<PvtPrim>() : _parent;
    _attr.push_back(RtTypedValue(type, RtValue::createNew(type, prim->prim())));
    _attrIndexByName[name] = _attr.size() - 1;

    return &_attr.back();
}

void PvtObject::removeAttribute(const RtToken& name)
{
    auto it = _attrIndexByName.find(name);
    if (it != _attrIndexByName.end())
    {
        _attr.erase(_attr.begin() + it->second);
        _attrIndexByName.erase(it);
    }
}

RtTypedValue* PvtObject::getAttribute(const RtToken& name, const RtToken& type)
{
    RtTypedValue* value = getAttribute(name);
    if (value && value->getType() != type)
    {
        throw ExceptionRuntimeError("Attribute '" + name.str() + "' found with an unmatching datatype on object '" + getName().str() + "'");
    }
    return value;
}

}
