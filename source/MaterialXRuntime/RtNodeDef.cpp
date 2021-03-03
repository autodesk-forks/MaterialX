//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // Code for handling queries about schema attributes.
    //
    // TODO: Move this to a central location and use a
    //       data driven XML schema file to control this.
    //
    struct StdAttrRecord
    {
        RtTokenVec vec;
        RtTokenSet set;

        StdAttrRecord(const RtTokenVec& names = RtTokenVec()) :
            vec(names),
            set(names.begin(), names.end())
        {
        }
    };

    const StdAttrRecord STD_ATTR_INPUT_COLOR(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("value"),
        RtToken("uniform"),
        RtToken("defaultgeomprop"),
        RtToken("enum"),
        RtToken("enumvalues"),
        RtToken("colorspace"),
        RtToken("uiname"),
        RtToken("uifolder"),
        RtToken("uimin"),
        RtToken("uimax"),
        RtToken("uisoftmin"),
        RtToken("uisoftmax"),
        RtToken("uistep")
    });

    const StdAttrRecord STD_ATTR_INPUT_FLOAT(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("value"),
        RtToken("uniform"),
        RtToken("defaultgeomprop"),
        RtToken("enum"),
        RtToken("enumvalues"),
        RtToken("unittype"),
        RtToken("unit"),
        RtToken("uiname"),
        RtToken("uifolder"),
        RtToken("uimin"),
        RtToken("uimax"),
        RtToken("uisoftmin"),
        RtToken("uisoftmax"),
        RtToken("uistep")
    });

    const StdAttrRecord STD_ATTR_INPUT(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("value"),
        RtToken("uniform"),
        RtToken("defaultgeomprop"),
        RtToken("enum"),
        RtToken("enumvalues"),
        RtToken("uiname"),
        RtToken("uifolder"),
    });


    const StdAttrRecord STD_ATTR_OUTPUT(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("value"),
        RtToken("defaultinput"),
        RtToken("default")
    });

    const StdAttrRecord STD_ATTR(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("node"),
        RtToken("inherit"),
        RtToken("nodegroup"),
        RtToken("version"),
        RtToken("isdefaultversion"),
        RtToken("target"),
        RtToken("uiname"),
        RtToken("internalgeomprops")
    });

    const StdAttrRecord STD_ATTR_EMPTY;

    const StdAttrRecord& getStandardAttributeRecord(const RtNodeDef& /*node*/)
    {
        return STD_ATTR;
    }

    const StdAttrRecord& getStandardAttributeRecord(const RtNodeDef& node, const RtToken& portName)
    {
        RtInput input = node.getInput(portName);
        if (input)
        {
            const RtToken& type = input.getType();
            if (type == RtType::COLOR3 || type == RtType::COLOR4 || type == RtType::FILENAME)
            {
                return STD_ATTR_INPUT_COLOR;
            }
            else if (type == RtType::FLOAT || type == RtType::VECTOR2 || type == RtType::VECTOR3 || type == RtType::VECTOR4)
            {
                return STD_ATTR_INPUT_FLOAT;
            }
            else
            {
                return STD_ATTR_INPUT;
            }
        }
        else
        {
            RtOutput output = node.getOutput(portName);
            if (output)
            {
                return STD_ATTR_OUTPUT;
            }
        }
        return STD_ATTR_EMPTY;
    }
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(Tokens::NODE, RtType::TOKEN);
    prim->createRelationship(Tokens::NODEIMPL);

    return primH;
}

void RtNodeDef::setNode(const RtToken& node)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NODE, RtType::TOKEN);
    attr->asToken() = node;
}

const RtToken& RtNodeDef::getNode() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NODE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

RtToken RtNodeDef::getNamespacedNode() const
{
    const RtToken& node = getNode();
    const RtToken& namespaceString = getNamespace();
    if (namespaceString != EMPTY_TOKEN)
    {
        return RtToken(namespaceString.str() + NAME_PREFIX_SEPARATOR + node.str());
    }
    return node;
}

void RtNodeDef::setNodeGroup(const RtToken& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NODEGROUP, RtType::TOKEN);
    attr->asToken() = nodegroup;
}

const RtToken& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NODEGROUP, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setTarget(const RtToken& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::TARGET, RtType::TOKEN);
    attr->asToken() = nodegroup;
}

const RtToken& RtNodeDef::getTarget() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::TARGET, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setIneritance(const RtToken& inherit)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::INHERIT, RtType::TOKEN);
    attr->asToken() = inherit;
}

const RtToken& RtNodeDef::getIneritance() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::INHERIT, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setVersion(const RtToken& version)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::VERSION, RtType::TOKEN);
    attr->asToken() = version;
}

const RtToken& RtNodeDef::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::VERSION, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::IS_DEFAULT_VERSION, RtType::BOOLEAN);
    attr->asBool() = isDefault;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::IS_DEFAULT_VERSION, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

void RtNodeDef::setNamespace(const RtToken& space)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NAMESPACE, RtType::TOKEN);
    attr->asToken() = space;
}

const RtToken& RtNodeDef::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NAMESPACE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

bool RtNodeDef::isVersionCompatible(const RtToken& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(Tokens::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtToken& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getTargets())
    {
        if (obj.isA<RtPrim>())
        {
            RtNodeImpl impl(obj);
            if (impl.isValid() && (target == EMPTY_TOKEN || impl.getTarget() == target))
            {
                return impl.getPrim();
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
        RtTypedValue* data = input.getAttribute(Tokens::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->asString();
        }
    }
    return layout;
}

const RtTokenVec& RtNodeDef::getStandardAttributeNames() const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this);
    return record.vec;
}

const RtTokenVec& RtNodeDef::getStandardAttributeNames(const RtToken& portName) const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this, portName);
    return record.vec;
}

bool RtNodeDef::isStandardAttribute(const RtToken& attrName) const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this);
    return record.set.count(attrName) > 0;
}

bool RtNodeDef::isStandardAttribute(const RtToken& portName, const RtToken& attrName) const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this, portName);
    return record.set.count(attrName) > 0;
}

}
