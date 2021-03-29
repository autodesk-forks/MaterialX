//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodeDefPrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodeDefPrimSpec()
        {
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::NODE, RtType::TOKEN);
            addPrimAttribute(Identifiers::INHERIT, RtType::TOKEN);
            addPrimAttribute(Identifiers::NODEGROUP, RtType::TOKEN);
            addPrimAttribute(Identifiers::VERSION, RtType::TOKEN);
            addPrimAttribute(Identifiers::ISDEFAULTVERSION, RtType::BOOLEAN);
            addPrimAttribute(Identifiers::TARGET, RtType::TOKEN);
            addPrimAttribute(Identifiers::UINAME, RtType::STRING);
            addPrimAttribute(Identifiers::INTERNALGEOMPROPS, RtType::TOKEN);
            addPrimAttribute(Identifiers::NAMESPACE, RtType::TOKEN);

            addInputAttribute(Identifiers::DOC, RtType::STRING);
            addInputAttribute(Identifiers::UNIFORM, RtType::BOOLEAN);
            addInputAttribute(Identifiers::DEFAULTGEOMPROP, RtType::TOKEN);
            addInputAttribute(Identifiers::ENUM, RtType::STRING);
            addInputAttribute(Identifiers::ENUMVALUES, RtType::STRING);
            addInputAttribute(Identifiers::UINAME, RtType::STRING);
            addInputAttribute(Identifiers::UIFOLDER, RtType::STRING);

            addInputAttributeByType(RtType::COLOR3, Identifiers::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UIMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UIMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UISOFTMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UISOFTMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UISTEP, RtType::COLOR3);

            addInputAttributeByType(RtType::COLOR4, Identifiers::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UIMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UIMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UISOFTMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UISOFTMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UISTEP, RtType::COLOR4);

            addInputAttributeByType(RtType::FLOAT, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UIMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UIMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UISOFTMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UISOFTMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UISTEP, RtType::FLOAT);

            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UIMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UIMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UISOFTMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UISOFTMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UISTEP, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UIMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UIMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UISOFTMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UISOFTMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UISTEP, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UIMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UIMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UISOFTMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UISOFTMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UISTEP, RtType::VECTOR4);

            addOutputAttribute(Identifiers::DOC, RtType::STRING);
            addOutputAttribute(Identifiers::DEFAULTINPUT, RtType::TOKEN);
            addOutputAttribute(Identifiers::DEFAULT, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(Identifiers::NODE, RtType::TOKEN);
    prim->createRelationship(Identifiers::NODEIMPL);

    return primH;
}

const RtPrimSpec& RtNodeDef::getPrimSpec() const
{
    static const PvtNodeDefPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeDef::setNode(const RtIdentifier& node)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NODE, RtType::TOKEN);
    attr->asToken() = node;
}

const RtIdentifier& RtNodeDef::getNode() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NODE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

RtIdentifier RtNodeDef::getNamespacedNode() const
{
    const RtIdentifier& node = getNode();
    const RtIdentifier& namespaceString = getNamespace();
    if (namespaceString != EMPTY_IDENFITIER)
    {
        return RtIdentifier(namespaceString.str() + NAME_PREFIX_SEPARATOR + node.str());
    }
    return node;
}

void RtNodeDef::setNodeGroup(const RtIdentifier& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NODEGROUP, RtType::TOKEN);
    attr->asToken() = nodegroup;
}

const RtIdentifier& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NODEGROUP, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

void RtNodeDef::setTarget(const RtIdentifier& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::TARGET, RtType::TOKEN);
    attr->asToken() = nodegroup;
}

const RtIdentifier& RtNodeDef::getTarget() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::TARGET, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

void RtNodeDef::setIneritance(const RtIdentifier& inherit)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::INHERIT, RtType::TOKEN);
    attr->asToken() = inherit;
}

const RtIdentifier& RtNodeDef::getIneritance() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::INHERIT, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

void RtNodeDef::setVersion(const RtIdentifier& version)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::VERSION, RtType::TOKEN);
    attr->asToken() = version;
}

const RtIdentifier& RtNodeDef::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::VERSION, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::ISDEFAULTVERSION, RtType::BOOLEAN);
    attr->asBool() = isDefault;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::ISDEFAULTVERSION, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

void RtNodeDef::setNamespace(const RtIdentifier& space)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NAMESPACE, RtType::TOKEN);
    attr->asToken() = space;
}

const RtIdentifier& RtNodeDef::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NAMESPACE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

bool RtNodeDef::isVersionCompatible(const RtIdentifier& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(Identifiers::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtIdentifier& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getConnections())
    {
        if (obj.isA<RtPrim>())
        {
            RtPrim prim(obj);
            if (prim.hasApi<RtNodeImpl>() || prim.hasApi<RtNodeGraph>())
            {
                const RtTypedValue* attr = prim.getAttribute(Identifiers::TARGET, RtType::TOKEN);
                const RtIdentifier primTarget = attr ? attr->asToken() : EMPTY_IDENFITIER;
                if (primTarget == EMPTY_IDENFITIER || primTarget == target)
                {
                    return prim;
                }
            }
        }
    }
    return RtPrim();
}

RtNodeLayout RtNodeDef::getNodeLayout()
{
    RtNodeLayout layout;
    for (size_t i=0; i<numInputs(); ++i)
    {
        RtInput input = getInput(i);
        layout.order.push_back(input.getName());
        RtTypedValue* data = input.getAttribute(Identifiers::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->asString();
        }
    }
    return layout;
}

}
