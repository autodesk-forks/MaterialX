//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/Identifiers.h>

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
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::XPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::YPOS, RtType::FLOAT);
            addPrimAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addPrimAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Identifiers::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Identifiers::UINAME, RtType::STRING);
            addPrimAttribute(Identifiers::VERSION, RtType::TOKEN);
            addPrimAttribute(Identifiers::COLORSPACE, RtType::TOKEN);
            addPrimAttribute(Identifiers::FILEPREFIX, RtType::STRING);

            addInputAttribute(Identifiers::DOC, RtType::STRING);
            addInputAttribute(Identifiers::MEMBER, RtType::STRING);
            addInputAttribute(Identifiers::CHANNELS, RtType::STRING);
            addInputAttribute(Identifiers::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(Identifiers::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, Identifiers::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR4, Identifiers::COLORSPACE, RtType::TOKEN);

            addInputAttributeByType(RtType::FLOAT, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UNITTYPE, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNITTYPE, RtType::TOKEN);

            addInputAttributeByType(RtType::FILENAME, Identifiers::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::FILENAME, Identifiers::FILEPREFIX, RtType::STRING);

            addOutputAttribute(Identifiers::DOC, RtType::STRING);
            addOutputAttribute(Identifiers::MEMBER, RtType::STRING);
            addOutputAttribute(Identifiers::WIDTH, RtType::INTEGER);
            addOutputAttribute(Identifiers::HEIGHT, RtType::INTEGER);
            addOutputAttribute(Identifiers::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, Identifiers::COLORSPACE, RtType::TOKEN);
            addOutputAttributeByType(RtType::COLOR4, Identifiers::COLORSPACE, RtType::TOKEN);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    RtPrim nodedef = RtApi::get().getDefinition<RtNodeDef>(typeName);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("No nodedef registered with name '" + typeName.str() + "'");
    }
    return createNode(nodedef, name, parent);
}

RtPrim RtNode::createNode(RtPrim nodedef, const RtIdentifier& name, RtPrim parent)
{
    // Make sure this is a valid nodedef.
    RtNodeDef nodedefSchema(nodedef);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("Given nodedef with name '" + nodedef.getName().str() + "' is not valid");
    }

    PvtPrim* nodedefPrim = PvtObject::cast<PvtPrim>(nodedef);

    const RtIdentifier nodeName = name == EMPTY_IDENFITIER ? nodedefSchema.getNode() : name;
    PvtObjHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::cast<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(Identifiers::NODEDEF);
    nodedefRelation->connect(nodedefPrim);

    // Copy version tag if used.
    const RtIdentifier& version = nodedefSchema.getVersion();
    if (version != EMPTY_IDENFITIER)
    {
        RtTypedValue* attr = node->createAttribute(Identifiers::VERSION, RtType::TOKEN);
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

const RtPrimSpec& RtNode::getPrimSpec() const
{
    static const PvtNodePrimSpec s_primSpec;
    return s_primSpec;
}

void RtNode::setNodeDef(RtPrim nodeDef)
{
    PvtRelationship* nodedefRel = prim()->getRelationship(Identifiers::NODEDEF);
    if (!nodedefRel)
    {
        nodedefRel = prim()->createRelationship(Identifiers::NODEDEF);
    }
    else
    {
        nodedefRel->clearConnections();
    }
    nodedefRel->connect(PvtObject::cast(nodeDef));
}

RtPrim RtNode::getNodeDef() const
{
    PvtRelationship* nodedefRel = prim()->getRelationship(Identifiers::NODEDEF);
    return nodedefRel && nodedefRel->hasConnections() ? nodedefRel->getConnection() : RtPrim();
}

void RtNode::setVersion(const RtIdentifier& version)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::VERSION, RtType::TOKEN);
    attr->asToken() = version;
}

const RtIdentifier& RtNode::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::VERSION, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_IDENFITIER;
}

}
