//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    const RtToken NODEDEF("nodedef");

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
        RtToken("Value"),
        RtToken("nodename"),
        RtToken("nodegraph"),
        RtToken("output"),
        RtToken("member"),
        RtToken("channels"),
        RtToken("colorspace")
    });

    const StdAttrRecord STD_ATTR_INPUT_FLOAT(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("Value"),
        RtToken("nodename"),
        RtToken("nodegraph"),
        RtToken("output"),
        RtToken("member"),
        RtToken("channels"),
        RtToken("unit"),
        RtToken("unittype")
    });

    const StdAttrRecord STD_ATTR_INPUT(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("Value"),
        RtToken("nodename"),
        RtToken("nodegraph"),
        RtToken("output"),
        RtToken("member"),
        RtToken("channels")
    });

    const StdAttrRecord STD_ATTR_OUTPUT_COLOR(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("nodename"),
        RtToken("output"),
        RtToken("member"),
        RtToken("colorspace"),
        RtToken("width"),
        RtToken("height"),
        RtToken("bitdepth")
    });

    const StdAttrRecord STD_ATTR_OUTPUT(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("nodename"),
        RtToken("output"),
        RtToken("member"),
        RtToken("width"),
        RtToken("height"),
        RtToken("bitdepth")
    });

    const StdAttrRecord STD_ATTR(
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("value"),
        RtToken("doc"),
        RtToken("xpos"),
        RtToken("ypos"),
        RtToken("width"),
        RtToken("height"),
        RtToken("uicolor"),
        RtToken("uiname"),
        RtToken("uivisible"),
        RtToken("uiadvanced"),
        RtToken("version"),
        RtToken("cms"),
        RtToken("cmsconfig"),
        RtToken("colorspace"),
        RtToken("namespace")
    });

    const StdAttrRecord STD_ATTR_EMPTY;

    const StdAttrRecord& getStandardAttributeRecord(const RtNode& /*node*/)
    {
        return STD_ATTR;
    }

    const StdAttrRecord& getStandardAttributeRecord(const RtNode& node, const RtToken& portName)
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
                const RtToken& type = output.getType();
                if (type == RtType::COLOR3 || type == RtType::COLOR4 || type == RtType::FILENAME)
                {
                    return STD_ATTR_OUTPUT_COLOR;
                }
                else
                {
                    return STD_ATTR_OUTPUT;
                }
            }
        }
        return STD_ATTR_EMPTY;
    }
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    const RtPrim prim = RtApi::get().getNodeDef(typeName);
    if (!prim)
    {
        throw ExceptionRuntimeError("No nodedef registered with name '" + typeName.str() + "'");
    }

    // Make sure this is a valid nodedef.
    RtNodeDef nodedef(prim);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("Nodedef with name '" + typeName.str() + "' is not valid");
    }

    const PvtPrim* nodedefPrim = PvtObject::ptr<PvtPrim>(prim);

    const RtToken nodeName = name == EMPTY_TOKEN ? nodedef.getNode() : name;
    PvtDataHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::ptr<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(NODEDEF);
    nodedefRelation->addTarget(nodedefPrim);

    // Copy version tag if used.
    const RtToken& version = nodedef.getVersion();
    if (version != EMPTY_TOKEN)
    {
        RtTypedValue* attr = node->createAttribute(Tokens::VERSION, RtType::TOKEN);
        attr->asToken() = version;
    }

    // Create the interface according to nodedef.
    for (PvtObject* obj : nodedefPrim->getInputs())
    {
        const PvtInput* port = obj->asA<PvtInput>();
        PvtInput* input = node->createInput(port->getName(), port->getType(), port->getFlags());
        RtValue::copy(port->getType(), port->getValue(), input->getValue());
    }
    for (PvtObject* obj : nodedefPrim->getOutputs())
    {
        const PvtOutput* port = obj->asA<PvtOutput>();
        PvtOutput* output = node->createOutput(port->getName(), port->getType(), port->getFlags());
        RtValue::copy(port->getType(), port->getValue(), output->getValue());
    }

    return nodeH;
}

void RtNode::setNodeDef(RtPrim nodeDef)
{
    PvtRelationship* nodedefRel = prim()->getRelationship(NODEDEF);
    if (!nodedefRel)
    {
        nodedefRel = prim()->createRelationship(NODEDEF);
    }
    else
    {
        nodedefRel->clearTargets();
    }
    nodedefRel->addTarget(PvtObject::ptr<PvtPrim>(nodeDef));
}

RtPrim RtNode::getNodeDef() const
{
    PvtRelationship* nodedef = prim()->getRelationship(NODEDEF);
    return nodedef && nodedef->hasTargets() ? nodedef->getAllTargets()[0] : RtPrim();
}

void RtNode::setVersion(const RtToken& version)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::VERSION, RtType::TOKEN);
    attr->asToken() = version;
}

const RtToken& RtNode::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::VERSION, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

const RtTokenVec& RtNode::getStandardAttributeNames() const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this);
    return record.vec;
}

const RtTokenVec& RtNode::getStandardAttributeNames(const RtToken& portName) const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this, portName);
    return record.vec;
}

bool RtNode::isStandardAttribute(const RtToken& attrName) const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this);
    return record.set.count(attrName) > 0;
}

bool RtNode::isStandardAttribute(const RtToken& portName, const RtToken& attrName) const
{
    const StdAttrRecord& record = getStandardAttributeRecord(*this, portName);
    return record.set.count(attrName) > 0;
}

}
