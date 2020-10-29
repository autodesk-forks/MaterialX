//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtRelationship.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtConnectableApi.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtRelationship, RtObjType::RELATIONSHIP, "PvtRelationship")

PvtRelationship::PvtRelationship(const RtToken& name, PvtPrim* parent) :
    PvtObject(name, parent)
{
    setTypeBit<PvtRelationship>();
}

void PvtRelationship::addTarget(const PvtObject* target)
{
    // Check if this relationship exists already.
    // Linear search here not ideal for performance, but we need the relationship ordering
    // so must maintain a vector here. If performance ever gets noticable we could add an
    // extra set/map if we can affor the storage.
    for (auto it = _targets.begin(); it != _targets.end(); ++it)
    {
        if (it->get() == target)
        {
            // Relationship already exists
            return;
        }
    }

    // Validate the new relationship with the connectable APIs.
    const PvtPrim* targetPrim = target->isA<PvtPrim>() ? target->asA<PvtPrim>() : target->getParent();
    RtConnectableApi* dstApi = RtConnectableApi::get(targetPrim->prim());
    RtConnectableApi* srcApi = RtConnectableApi::get(getParent()->prim());

    // Check with the destination's connectable API if it accepts the relationship.
    if (!(dstApi && dstApi->acceptRelationship(hnd(), target->hnd())))
    {
        throw ExceptionRuntimeError("Target '" + target->getPath().asString() + "' rejected the relationship");
    }

    // If the source prim is of another prim type check that this connectable API also
    // accepts the connection.
    if (srcApi != dstApi)
    {
        if (!(srcApi && srcApi->acceptRelationship(hnd(), target->hnd())))
        {
            throw ExceptionRuntimeError("Source '" + getPath().asString() + "' rejected the relationship");
        }
    }

    // Create the relationship.
    _targets.push_back(target->hnd());
}

void PvtRelationship::removeTarget(const PvtObject* target)
{
    for (auto it = _targets.begin(); it != _targets.end(); ++it)
    {
        if (it->get() == target)
        {
            _targets.erase(it);
            break;
        }
    }
}

}
