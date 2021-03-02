//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtPrim, RtObjType::PRIM, "PvtPrim")

PvtPrim::PvtPrim(const RtTypeInfo* typeInfo, const RtToken& name, PvtPrim* parent) :
    PvtObject(name, parent),
    _typeInfo(typeInfo)
{
    setTypeBit<PvtPrim>();
}

void PvtPrim::dispose(bool state)
{
    for (const PvtDataHandle& hnd : _rel.all())
    {
        hnd->setDisposed(state);
    }
    for (const PvtDataHandle& hnd : _inputs.all())
    {
        hnd->setDisposed(state);
    }
    for (const PvtDataHandle& hnd : _outputs.all())
    {
        hnd->setDisposed(state);
    }
    for (const PvtDataHandle& hnd : _prims.all())
    {
        hnd->asA<PvtPrim>()->dispose(state);
    }
    setDisposed(state);
}

void PvtPrim::destroy()
{
    // Disconnect and delete all relationships.
    for (PvtDataHandle& hnd : _rel.all())
    {
        hnd->asA<PvtRelationship>()->clearTargets();
    }
    _rel.clear();

    // Disconnect and delete all inputs.
    for (PvtDataHandle& hnd : _inputs.all())
    {
        hnd->asA<PvtInput>()->clearConnection();
    }
    _inputs.clear();

    // Disconnect and delete all outputs.
    for (PvtDataHandle& hnd : _outputs.all())
    {
        hnd->asA<PvtOutput>()->clearConnections();
    }
    _outputs.clear();

    // Destroy all child prims reqursively.
    for (const PvtDataHandle& hnd : _prims.all())
    {
        hnd->asA<PvtPrim>()->destroy();
    }
    _prims.clear();

    // Tag as disposed.
    dispose(true);
}

PvtRelationship* PvtPrim::createRelationship(const RtToken& name)
{
    if (getRelationship(name))
    {
        throw ExceptionRuntimeError("A relationship named '" + name.str() + "' already exists in prim '" + getName().str() + "'");
    }

    PvtDataHandle relH(new PvtRelationship(name, this));
    _rel.add(relH);

    return relH->asA<PvtRelationship>();
}

void PvtPrim::removeRelationship(const RtToken& name)
{
    PvtRelationship* rel = getRelationship(name);
    if (rel)
    {
        rel->setDisposed(true);
        _rel.remove(name);
    }
}

void PvtPrim::renameRelationship(const RtToken& name, const RtToken& newName)
{
    if (getRelationship(newName))
    {
        throw ExceptionRuntimeError("A relationship named '" + newName.str() + "' already exists in prim '" + getName().str() + "'");
    }
    PvtRelationship* rel = getRelationship(name);
    if (rel)
    {
        rel->setName(newName);
        _rel.add(rel->hnd());
        _rel.remove(name);
    }
}

PvtInput* PvtPrim::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    // Inputs with type filename, token or string must always be uniform.
    if (type == RtType::FILENAME || type == RtType::TOKEN || type == RtType::STRING)
    {
        flags |= RtPortFlag::UNIFORM;
    }

    RtToken uniqueName = makeUniqueChildName(name);
    PvtDataHandle portH(new PvtInput(uniqueName, type, flags, this));
    _inputs.add(portH);

    return portH->asA<PvtInput>();
}

void PvtPrim::removeInput(const RtToken& name)
{
    PvtPort* port = getInput(name);
    if (!port)
    {
        throw ExceptionRuntimeError("No input found with name '" + name.str() + "'");
    }
    port->setDisposed(true);
    _inputs.remove(name);
}

RtToken PvtPrim::renameInput(const RtToken& name, const RtToken& newName, bool makeUnique)
{
    PvtDataHandle hnd = _inputs.remove(name);
    if (!hnd)
    {
        throw ExceptionRuntimeError("Unable to rename input. Input named '" + name.str() + "' does not exist.");
    }

    RtToken result = newName;
    if (makeUnique)
    {
        result = makeUniqueChildName(newName);
    }

    hnd->setName(result);
    _inputs.add(hnd);

    return result;
}

PvtOutput* PvtPrim::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    RtToken uniqueName = makeUniqueChildName(name);
    PvtDataHandle portH(new PvtOutput(uniqueName, type, flags, this));
    _outputs.add(portH);

    return portH->asA<PvtOutput>();
}

void PvtPrim::removeOutput(const RtToken& name)
{
    PvtPort* port = getOutput(name);
    if (!port)
    {
        throw ExceptionRuntimeError("No output found with name '" + name.str() + "'");
    }
    port->setDisposed(true);
    _outputs.remove(name);
}

RtToken PvtPrim::renameOutput(const RtToken& name, const RtToken& newName, bool makeUnique)
{
    PvtDataHandle hnd = _outputs.remove(name);
    if (!hnd)
    {
        throw ExceptionRuntimeError("Unable to rename output. Output named '" + name.str() + "' does not exist.");
    }

    RtToken result = newName;
    if (makeUnique)
    {
        result = makeUniqueChildName(newName);
    }

    hnd->setName(result);
    _outputs.add(hnd);

    return result;
}

RtPrimIterator PvtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(hnd(), predicate);
}

RtToken PvtPrim::makeUniqueChildName(const RtToken& name) const
{
    // Collect all existing child names.
    RtTokenSet allNames;
    for (auto it : _inputs.all())
    {
        allNames.insert(it->getName());
    }
    for (auto it : _outputs.all())
    {
        allNames.insert(it->getName());
    }
    for (auto it : _prims.all())
    {
        allNames.insert(it->getName());
    }

    RtToken newName = name;

    // Check if there is another child with this name.
    if (allNames.count(newName))
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of("0123456789") + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other child with the resulting name.
        do {
            newName = RtToken(baseName + std::to_string(i++));
        } while (allNames.count(newName));
    }

    return newName;
}

void PvtPrim::addChildPrim(const PvtPrim* prim)
{
    _prims.add(prim->hnd());
}

void PvtPrim::removeChildPrim(const PvtPrim* prim)
{
    _prims.remove(prim->getName());
}

}
