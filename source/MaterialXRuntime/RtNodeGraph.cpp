//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtConnectableApi.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXCore/Element.h>

namespace MaterialX
{

namespace
{
    static const RtToken SOCKETS("_nodegraph_internal_sockets");
}

DEFINE_TYPED_SCHEMA(RtNodeGraph, "node:nodegraph");

const RtTypeInfo RtNodeGraph::SOCKETS_TYPE_INFO("_nodegraph_internal_sockets");

RtPrim RtNodeGraph::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name);

    static const RtToken DEFAULT_NAME("nodegraph1");
    const RtToken primName = name == EMPTY_TOKEN ? DEFAULT_NAME : name;
    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();

    // Add a child prim to hold the internal sockets.
    PvtDataHandle socketH = PvtPrim::createNew(&SOCKETS_TYPE_INFO, SOCKETS, prim);
    prim->addChildPrim(socketH->asA<PvtPrim>());

    return primH;
}

RtInput RtNodeGraph::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtInput input = prim()->createInput(name, type, flags)->hnd();
    socket->createOutput(input.getName(), type, flags | RtPortFlag::SOCKET);
    return input;
}

void RtNodeGraph::removeInput(const RtToken& name)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    prim()->removeInput(name);
    socket->removeOutput(name);
}

RtToken RtNodeGraph::renameInput(const RtToken& name, const RtToken& newName)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtToken newPortName = prim()->renameInput(name, newName);
    socket->renameOutput(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtOutput output = prim()->createOutput(name, type, flags)->hnd();
    socket->createInput(output.getName(), type, flags | RtPortFlag::SOCKET);
    return output;
}

void RtNodeGraph::removeOutput(const RtToken& name)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    prim()->removeOutput(name);
    socket->removeInput(name);
}

RtToken RtNodeGraph::renameOutput(const RtToken& name, const RtToken& newName)
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    RtToken newPortName = prim()->renameOutput(name, newName);
    socket->renameInput(name, newPortName);
    return newPortName;
}

RtOutput RtNodeGraph::getInputSocket(const RtToken& name) const
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    // Input socket is an output in practice.
    PvtOutput* output = socket->getOutput(name);
    return output ? output->hnd() : RtOutput();
}

RtInput RtNodeGraph::getOutputSocket(const RtToken& name) const
{
    PvtPrim* socket = prim()->getChild(SOCKETS);
    // Output socket is an input in practice.
    PvtInput* input = socket->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtNodeLayout RtNodeGraph::getNodeLayout()
{
    RtNodeLayout layout;
    for (PvtObject* input : prim()->getInputs())
    {
        layout.order.push_back(input->getName());
        RtTypedValue* attr = input->getAttribute(Tokens::UIFOLDER, RtType::STRING);
        if (attr)
        {
            layout.uifolder[input->getName()] = attr->asString();
        }
    }
    return layout;
}

void RtNodeGraph::setNodeLayout(const RtNodeLayout& layout)
{
    PvtPrim* p = prim();

    // Create new input/output lists with ports in the specifed order 
    RtTokenSet processed;
    PvtObjectList newInputList;
    PvtObjectList newOutputList;
    for (const RtToken& name : layout.order)
    {
        if (!processed.count(name))
        {
            PvtInput* input = p->getInput(name);
            if (input)
            {
                newInputList.add(input);
            }
            else
            {
                PvtOutput* output = p->getOutput(name);
                if (output)
                {
                    newOutputList.add(output);
                }
            }
            processed.insert(name);
        }
    }

    // Move over any attributes that were not specified in the new order.
    for (PvtObject* input : p->getInputs())
    {
        if (!processed.count(input->getName()))
        {
            newInputList.add(input);
            processed.insert(input->getName());
        }
    }
    for (PvtObject* output : p->getOutputs())
    {
        if (!processed.count(output->getName()))
        {
            newOutputList.add(output);
            processed.insert(output->getName());
        }
    }

    // Make sure all attributes were moved.
    if (newInputList.size() != p->numInputs() || newOutputList.size() != p->numOutputs())
    {
        throw ExceptionRuntimeError("Failed setting new node layout for '" + getName().str() + "'. Changing the port count is not allowed.");
    }

    // Switch to the new order.
    p->_inputs = newInputList;
    p->_outputs = newOutputList;

    // Assign uifolder metadata.
    for (PvtObject* input: p->getInputs())
    {
        auto it = layout.uifolder.find(input->getName());
        if (it != layout.uifolder.end() && !it->second.empty())
        {
            RtTypedValue* attr = input->createAttribute(Tokens::UIFOLDER, RtType::STRING);
            attr->asString() = it->second;
        }
        else
        {
            input->removeAttribute(Tokens::UIFOLDER);
        }
    }
}

RtPrim RtNodeGraph::getNode(const RtToken& name) const
{
    PvtPrim* p = prim()->getChild(name);
    return p && p->getTypeInfo()->isCompatible(RtNode::typeName()) ? p->hnd() : RtPrim();
}

RtPrimIterator RtNodeGraph::getNodes() const
{
    RtSchemaPredicate<RtNode> predicate;
    return RtPrimIterator(hnd(), predicate);
}

void RtNodeGraph::setDefinition(const RtToken& nodedef)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NODEDEF, RtType::TOKEN);
    attr->asToken() = nodedef;
}

const RtToken& RtNodeGraph::getDefinition() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NODEDEF, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

string RtNodeGraph::asStringDot() const
{
    string dot = "digraph {\n";

    // Add input/output interface boxes.
    dot += "    \"inputs\" ";
    dot += "[shape=box];\n";
    dot += "    \"outputs\" ";
    dot += "[shape=box];\n";

    // Add all nodes.
    for (const RtPrim prim : getNodes())
    {
        dot += "    \"" + prim.getName().str() + "\" ";
        dot += "[shape=ellipse];\n";
    }

    // Add connections inbetween nodes
    // and between nodes and input interface.
    for (const RtPrim prim : getNodes())
    {
        const string dstName = prim.getName().str();
        for (size_t i = 0; i < prim.numInputs(); ++i)
        {
            RtInput input = prim.getInput(i);
            if (input.isConnected())
            {
                const RtOutput src = input.getConnection();
                const string srcName = src.isSocket() ? "inputs" : src.getParent().getName().str();
                dot += "    \"" + srcName;
                dot += "\" -> \"" + dstName;
                dot += "\" [label=\"" + input.getName().str() + "\"];\n";
            }
        }
    }

    RtPrim sockets = getPrim().getChild(SOCKETS);

    // Add connections between nodes and output sockets.
    for (size_t i = 0; i < sockets.numInputs(); ++i)
    {
        RtInput socket = sockets.getInput(i);
        if (socket.isConnected())
        {
            const RtOutput src = socket.getConnection();
            const string srcName = src.isSocket() ? "inputs" : src.getParent().getName().str();
            dot += "    \"" + srcName;
            dot += "\" -> \"outputs";
            dot += "\" [label=\"" + socket.getName().str() + "\"];\n";
        }
    }

    dot += "}\n";

    return dot;
}

}
