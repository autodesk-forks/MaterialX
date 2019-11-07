//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvPortDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

PrvNodeDef::PrvNodeDef(const RtToken& name, const RtToken& category) :
    PrvValueStoringElement(RtObjType::NODEDEF, name),
    _category(category),
    _numOutputs(0)
{
}

PrvObjectHandle PrvNodeDef::createNew(const RtToken& name, const RtToken& category)
{
    return std::make_shared<PrvNodeDef>(name, category);
}

void PrvNodeDef::addPort(PrvObjectHandle portdef)
{
    if (!portdef->hasApi(RtApiType::PORTDEF))
    {
        throw ExceptionRuntimeError("Given object is not a valid portdef");
    }

    PrvPortDef* p = portdef->asA<PrvPortDef>();
    if (_childrenByName.count(p->getName()))
    {
        throw ExceptionRuntimeError("A port named '" + p->getName().str() + "' already exists for nodedef '" + getName().str() + "'");
    }

    // We want to preserve the ordering of having all outputs stored before any inputs.
    // So if inputs are already stored we need to handled inserting the new output in
    // the right place.
    if (p->isOutput() && _children.size() && !_children.back()->asA<PrvPortDef>()->isOutput())
    {
        // Insert the new output after the last output.
        for (auto it = _children.begin(); it != _children.end(); ++it)
        {
            if (!(*it)->asA<PrvPortDef>()->isOutput())
            {
                _children.insert(it, portdef);
                break;
            }
        }
    }
    else
    {
        _children.push_back(portdef);
    }

    _childrenByName[p->getName()] = portdef;
    _numOutputs += p->isOutput();
    rebuildPortIndex();
}

void PrvNodeDef::removePort(const RtToken& name)
{
    PrvPortDef* p = port(name);
    if (p)
    {
        _numOutputs -= p->isOutput();
        removeChild(name);
        rebuildPortIndex();
    }
}

void PrvNodeDef::rebuildPortIndex()
{
    for (size_t i = 0; i < numPorts(); ++i)
    {
        _portIndex[port(i)->getName()] = i;
    }
}

}
