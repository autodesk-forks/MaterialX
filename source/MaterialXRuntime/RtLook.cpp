//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Tokens.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtLookGroup, "bindelement:lookgroup");

RtPrim RtLookGroup::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("lookgroup1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::LOOKS);

    return primH;
}

void RtLookGroup::setActiveLook(const string& look)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::ACTIVELOOK, RtType::STRING);
    attr->asString() = look;
}

const string& RtLookGroup::getActiveLook() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::ACTIVELOOK);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtLookGroup::addLook(const RtObject& look)
{
    getLooks().addTarget(look);
}

void RtLookGroup::removeLook(const RtObject& look)
{
    getLooks().removeTarget(look);
}

RtRelationship RtLookGroup::getLooks() const
{
    return prim()->getRelationship(Tokens::LOOKS)->hnd();
}

bool RtLookGroupConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::LOOKS)
    {
        // 'looks' relationship only accepts looks or lookgroups as target.
        return target.isA<RtPrim>() && 
            (target.asA<RtPrim>().hasApi<RtLook>() || target.asA<RtPrim>().hasApi<RtLookGroup>());
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtLook, "bindelement:look");

RtPrim RtLook::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("look1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::INHERIT);
    prim->createRelationship(Tokens::MATERIAL_ASSIGN);

    return primH;
}

RtRelationship RtLook::getInherit() const
{
    return prim()->getRelationship(Tokens::INHERIT)->hnd();
}

void RtLook::addMaterialAssign(const RtObject& assignment)
{
    getMaterialAssigns().addTarget(assignment);
}

void RtLook::removeMaterialAssign(const RtObject& assignment)
{
    getMaterialAssigns().removeTarget(assignment);
}

RtRelationship RtLook::getMaterialAssigns() const
{
    return prim()->getRelationship(Tokens::MATERIAL_ASSIGN)->hnd();
}

bool RtLookConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::INHERIT)
    {
        // 'inherit' relationship only accepts other looks as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtLook>();
    }
    else if (rel.getName() == Tokens::MATERIAL_ASSIGN)
    {
        // 'materialassign' relationship only accepts materialassigns as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtMaterialAssign>();
    }
    return false;
}


DEFINE_TYPED_SCHEMA(RtMaterialAssign, "bindelement:materialassign");

RtPrim RtMaterialAssign::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("materialassign1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createInput(Tokens::MATERIAL, RtType::MATERIAL);
    prim->createRelationship(Tokens::COLLECTION);

    return primH;
}

RtInput RtMaterialAssign::getMaterial() const
{
    return prim()->getInput(Tokens::MATERIAL)->hnd();
}

RtRelationship RtMaterialAssign::getCollection() const
{
    return prim()->getRelationship(Tokens::COLLECTION)->hnd();
}

void RtMaterialAssign::setGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::GEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtMaterialAssign::getGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::GEOM, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtMaterialAssign::setExclusive(bool exclusive)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::EXCLUSIVE, RtType::BOOLEAN);
    attr->asBool() = exclusive;
}

bool RtMaterialAssign::getExclusive() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::EXCLUSIVE, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

bool RtMaterialAssignConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::COLLECTION)
    {
        // 'collection' relationship only accepts other collections as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
