//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtStrings.h>

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
            addPrimAttribute(RtStrings::DOC, RtType::STRING);
            addPrimAttribute(RtStrings::NODE, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::INHERIT, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::NODEGROUP, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::ISDEFAULTVERSION, RtType::BOOLEAN);
            addPrimAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::UINAME, RtType::STRING);
            addPrimAttribute(RtStrings::INTERNALGEOMPROPS, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::NAMESPACE, RtType::IDENTIFIER);

            addInputAttribute(RtStrings::DOC, RtType::STRING);
            addInputAttribute(RtStrings::UNIFORM, RtType::BOOLEAN);
            addInputAttribute(RtStrings::DEFAULTGEOMPROP, RtType::IDENTIFIER);
            addInputAttribute(RtStrings::ENUM, RtType::STRING);
            addInputAttribute(RtStrings::ENUMVALUES, RtType::STRING);
            addInputAttribute(RtStrings::UINAME, RtType::STRING);
            addInputAttribute(RtStrings::UIFOLDER, RtType::STRING);
            addInputAttribute(RtStrings::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, RtStrings::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::COLOR3, RtStrings::UIMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtStrings::UIMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtStrings::UISOFTMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtStrings::UISOFTMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtStrings::UISTEP, RtType::COLOR3);

            addInputAttributeByType(RtType::COLOR4, RtStrings::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::COLOR4, RtStrings::UIMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtStrings::UIMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtStrings::UISOFTMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtStrings::UISOFTMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtStrings::UISTEP, RtType::COLOR4);

            addInputAttributeByType(RtType::FLOAT, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UIMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UIMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UISOFTMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UISOFTMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UISTEP, RtType::FLOAT);

            addInputAttributeByType(RtType::VECTOR2, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UIMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UIMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UISOFTMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UISOFTMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UISTEP, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR3, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UIMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UIMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UISOFTMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UISOFTMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UISTEP, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR4, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UIMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UIMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UISOFTMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UISOFTMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UISTEP, RtType::VECTOR4);

            addOutputAttribute(RtStrings::DOC, RtType::STRING);
            addOutputAttribute(RtStrings::DEFAULTINPUT, RtType::IDENTIFIER);
            addOutputAttribute(RtStrings::DEFAULT, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(RtStrings::NODE, RtType::IDENTIFIER);
    prim->createRelationship(RtStrings::NODEIMPL);

    return primH;
}

const RtPrimSpec& RtNodeDef::getPrimSpec() const
{
    static const PvtNodeDefPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeDef::setNode(const RtString& node)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::NODE, RtType::IDENTIFIER);
    attr->asIdentifier() = node;
}

const RtString& RtNodeDef::getNode() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::NODE, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

RtString RtNodeDef::getNamespacedNode() const
{
    const RtString& node = getNode();
    const RtString& namespaceString = getNamespace();
    if (namespaceString)
    {
        return RtString(namespaceString.str() + NAME_PREFIX_SEPARATOR + node.str());
    }
    return node;
}

void RtNodeDef::setNodeGroup(const RtString& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::NODEGROUP, RtType::IDENTIFIER);
    attr->asIdentifier() = nodegroup;
}

const RtString& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::NODEGROUP, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeDef::setTarget(const RtString& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
    attr->asIdentifier() = nodegroup;
}

const RtString& RtNodeDef::getTarget() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeDef::setIneritance(const RtString& inherit)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::INHERIT, RtType::IDENTIFIER);
    attr->asIdentifier() = inherit;
}

const RtString& RtNodeDef::getIneritance() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::INHERIT, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeDef::setVersion(const RtString& version)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
    attr->asIdentifier() = version;
}

const RtString& RtNodeDef::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::ISDEFAULTVERSION, RtType::BOOLEAN);
    attr->asBool() = isDefault;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::ISDEFAULTVERSION, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

void RtNodeDef::setNamespace(const RtString& space)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::NAMESPACE, RtType::IDENTIFIER);
    attr->asIdentifier() = space;
}

const RtString& RtNodeDef::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::NAMESPACE, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

void RtNodeDef::setDoc(const string& doc) {
    RtTypedValue* attr = prim()->createAttribute(RtStrings::DOC, RtType::STRING);
    attr->asString() = doc;
}

const string& RtNodeDef::getDoc() const {
    RtTypedValue* attr = prim()->getAttribute(RtStrings::DOC, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

bool RtNodeDef::isVersionCompatible(const RtString& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(RtStrings::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtString& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getConnections())
    {
        if (obj.isA<RtPrim>())
        {
            RtPrim prim(obj);
            if (prim.hasApi<RtNodeImpl>() || prim.hasApi<RtNodeGraph>())
            {
                const RtTypedValue* attr = prim.getAttribute(RtStrings::TARGET, RtType::IDENTIFIER);
                const RtString primTarget = attr ? attr->asIdentifier() : RtString::EMPTY;
                if (primTarget.empty() || primTarget == target)
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
        RtTypedValue* data = input.getAttribute(RtStrings::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->asString();
        }
    }
    return layout;
}

}
