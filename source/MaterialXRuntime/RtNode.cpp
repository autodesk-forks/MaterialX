//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtStrings.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodePrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodePrimSpec()
        {
            addPrimAttribute(RtStrings::DOC, RtType::STRING);
            addPrimAttribute(RtStrings::XPOS, RtType::FLOAT);
            addPrimAttribute(RtStrings::YPOS, RtType::FLOAT);
            addPrimAttribute(RtStrings::WIDTH, RtType::INTEGER);
            addPrimAttribute(RtStrings::HEIGHT, RtType::INTEGER);
            addPrimAttribute(RtStrings::UICOLOR, RtType::COLOR3);
            addPrimAttribute(RtStrings::UINAME, RtType::STRING);
            addPrimAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::COLORSPACE, RtType::IDENTIFIER);
            addPrimAttribute(RtStrings::FILEPREFIX, RtType::STRING);

            addInputAttribute(RtStrings::DOC, RtType::STRING);
            addInputAttribute(RtStrings::MEMBER, RtType::STRING);
            addInputAttribute(RtStrings::CHANNELS, RtType::STRING);
            addInputAttribute(RtStrings::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(RtStrings::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, RtStrings::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::COLOR4, RtStrings::COLORSPACE, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::FLOAT, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FLOAT, RtStrings::UNITTYPE, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR2, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, RtStrings::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR3, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, RtStrings::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR4, RtStrings::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR4, RtStrings::UNITTYPE, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::FILENAME, RtStrings::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FILENAME, RtStrings::FILEPREFIX, RtType::STRING);

            addOutputAttribute(RtStrings::DOC, RtType::STRING);
            addOutputAttribute(RtStrings::MEMBER, RtType::STRING);
            addOutputAttribute(RtStrings::WIDTH, RtType::INTEGER);
            addOutputAttribute(RtStrings::HEIGHT, RtType::INTEGER);
            addOutputAttribute(RtStrings::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, RtStrings::COLORSPACE, RtType::IDENTIFIER);
            addOutputAttributeByType(RtType::COLOR4, RtStrings::COLORSPACE, RtType::IDENTIFIER);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    RtPrim nodedef = RtApi::get().getDefinition<RtNodeDef>(typeName);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("No nodedef registered with name '" + typeName.str() + "'");
    }
    return createNode(nodedef, name, parent);
}

RtPrim RtNode::createNode(RtPrim nodedef, const RtString& name, RtPrim parent)
{
    // Make sure this is a valid nodedef.
    RtNodeDef nodedefSchema(nodedef);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("Given nodedef with name '" + nodedef.getName().str() + "' is not valid");
    }

    PvtPrim* nodedefPrim = PvtObject::cast<PvtPrim>(nodedef);

    const RtString nodeName = name.empty() ? nodedefSchema.getNode() : name;
    PvtObjHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::cast<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(RtStrings::NODEDEF);
    nodedefRelation->connect(nodedefPrim);

    // Copy version tag if used.
    const RtString& version = nodedefSchema.getVersion();
    if (!version.empty())
    {
        RtTypedValue* attr = node->createAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
        attr->asIdentifier() = version;
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

const RtPrimSpec& RtNode::getPrimSpec() const
{
    static const PvtNodePrimSpec s_primSpec;
    return s_primSpec;
}

void RtNode::setNodeDef(RtPrim nodeDef)
{
    PvtRelationship* nodedefRel = prim()->getRelationship(RtStrings::NODEDEF);
    if (!nodedefRel)
    {
        nodedefRel = prim()->createRelationship(RtStrings::NODEDEF);
    }
    else
    {
        nodedefRel->clearConnections();
    }
    nodedefRel->connect(PvtObject::cast(nodeDef));
}

RtPrim RtNode::getNodeDef() const
{
    PvtRelationship* nodedefRel = prim()->getRelationship(RtStrings::NODEDEF);
    return nodedefRel && nodedefRel->hasConnections() ? nodedefRel->getConnection() : RtPrim();
}

void RtNode::setVersion(const RtString& version)
{
    RtTypedValue* attr = prim()->createAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
    attr->asIdentifier() = version;
}

const RtString& RtNode::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(RtStrings::VERSION, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : RtString::EMPTY;
}

}
