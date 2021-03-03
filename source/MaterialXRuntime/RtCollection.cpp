//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtCollection, "bindelement:collection");

RtPrim RtCollection::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtToken DEFAULT_NAME("collection1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Tokens::INCLUDE_COLLECTION);

    return primH;
}

void RtCollection::setIncludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::INCLUDE_GEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getIncludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::INCLUDE_GEOM);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtCollection::setExcludeGeom(const string& geom)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::EXCLUDE_GEOM, RtType::STRING);
    attr->asString() = geom;
}

const string& RtCollection::getExcludeGeom() const
{
    const RtTypedValue* attr = prim()->getAttribute(Tokens::EXCLUDE_GEOM);
    return attr ? attr->asString() : EMPTY_STRING;
}

void RtCollection::addCollection(const RtObject& collection)
{
    getIncludeCollection().addTarget(collection);
}

void RtCollection::removeCollection(const RtObject& collection)
{
    getIncludeCollection().removeTarget(collection);
}

RtRelationship RtCollection::getIncludeCollection() const
{
    return prim()->getRelationship(Tokens::INCLUDE_COLLECTION)->hnd();
}

bool RtCollectionConnectableApi::acceptRelationship(const RtRelationship& rel, const RtObject& target) const
{
    if (rel.getName() == Tokens::INCLUDE_COLLECTION)
    {
        // 'includecollection' only accepts other collection prims as target.
        return target.isA<RtPrim>() && target.asA<RtPrim>().hasApi<RtCollection>();
    }
    return false;
}

}
