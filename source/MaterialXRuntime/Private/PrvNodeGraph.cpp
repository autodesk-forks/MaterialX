//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvNodeGraph.h>

#include <MaterialXRuntime/RtTypeDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtToken PrvNodeGraph::UNPUBLISHED_NODEDEF("__unpublished_nodedef__");
const RtToken PrvNodeGraph::INPUT_SOCKETS_NODEDEF("__inputsockets_nodedef__");
const RtToken PrvNodeGraph::OUTPUT_SOCKETS_NODEDEF("__outputsockets_nodedef__");
const RtToken PrvNodeGraph::INPUT_SOCKETS("__inputsockets__");
const RtToken PrvNodeGraph::OUTPUT_SOCKETS("__outputsockets__");
const RtToken PrvNodeGraph::SOCKETS_NODE_TYPE("__sockets__");

PrvNodeGraph::PrvNodeGraph(const RtToken& name) :
    PrvNode(name)
{
    _nodedef = PrvNodeDef::createNew(UNPUBLISHED_NODEDEF, UNPUBLISHED_NODEDEF);

    _inputSocketsNodeDef = PrvNodeDef::createNew(INPUT_SOCKETS_NODEDEF, SOCKETS_NODE_TYPE);
    _inputSockets = PrvNode::createNew(INPUT_SOCKETS, _inputSocketsNodeDef);

    _outputSocketsNodeDef = PrvNodeDef::createNew(OUTPUT_SOCKETS_NODEDEF, SOCKETS_NODE_TYPE);
    _outputSockets = PrvNode::createNew(OUTPUT_SOCKETS, _outputSocketsNodeDef);
}

PrvObjectHandle PrvNodeGraph::createNew(const RtToken& name)
{
    return std::make_shared<PrvNodeGraph>(name);
}

void PrvNodeGraph::addNode(PrvObjectHandle node)
{
    if (!node->hasApi(RtApiType::NODE))
    {
        throw ExceptionRuntimeError("Given object is not a valid node");
    }
    addChild(node);
}

void PrvNodeGraph::addPort(PrvObjectHandle portdef)
{
    const PrvPortDef* pd = portdef->asA<PrvPortDef>();
    const uint32_t flags = pd->getFlags();

    PrvNode::Port p;
    p.value = pd->getValue();
    p.colorspace = pd->getColorSpace();
    p.unit = pd->getUnit();

    // Add to external interface
    nodedef()->addPort(portdef);
    if (_ports.empty())
    {
        _ports.push_back(p);
    }
    else
    {
        size_t index = nodedef()->findPortIndex(pd->getName());
        _ports.insert(_ports.begin() + index, p);
    }

    // Add to internal interface
    if (pd->isOutput())
    {
        outputSocketsNodeDef()->addPort(PrvPortDef::createNew(pd->getName(), pd->getType(), pd->getValue(), flags & ~RtPortFlag::OUTPUT));
        outputSockets()->_ports.push_back(p);
    }
    else
    {
        inputSocketsNodeDef()->addPort(PrvPortDef::createNew(pd->getName(), pd->getType(), pd->getValue(), flags | RtPortFlag::OUTPUT));
        inputSockets()->_ports.push_back(p);
    }
}

void PrvNodeGraph::removePort(const RtToken& name)
{
    const PrvPortDef* portdef = nodedef()->findPort(name);
    if (!portdef)
    {
        return;
    }

    // Remove from internal interface
    if (portdef->isOutput())
    {
        const size_t index = outputSocketsNodeDef()->findPortIndex(name);
        if (index != INVALID_INDEX)
        {
            outputSockets()->_ports.erase(outputSockets()->_ports.begin() + index);
        }
        outputSocketsNodeDef()->removePort(name);
    }
    else
    {
        const size_t index = inputSocketsNodeDef()->findPortIndex(name);
        if (index != INVALID_INDEX)
        {
            inputSockets()->_ports.erase(inputSockets()->_ports.begin() + index);
        }
        inputSocketsNodeDef()->removePort(name);
    }

    // Remove from external interface
    size_t index = nodedef()->findPortIndex(name);
    if (index != INVALID_INDEX)
    {
        _ports.erase(_ports.begin() + index);
    }
    nodedef()->removePort(name);
}

string PrvNodeGraph::asStringDot() const
{
    string dot = "digraph {\n";

    // Add alla nodes.
    dot += "    \"inputs\" ";
    dot += "[shape=box];\n";
    dot += "    \"outputs\" ";
    dot += "[shape=box];\n";

    // Add alla nodes.
    for (size_t i = 0; i < numChildren(); ++i)
    {
        dot += "    \"" + node(i)->getName().str() + "\" ";
        dot += "[shape=ellipse];\n";
    }

    auto writeConnections = [](PrvNode* n, string& dot)
    {
        string dstName = n->getName().str();
        if (n->getName() == OUTPUT_SOCKETS)
        {
            dstName = "outputs";
        }
        for (size_t j = 0; j < n->numPorts(); ++j)
        {
            RtPort p = n->getPort(j);
            if (p.isInput() && p.isConnected())
            {
                RtPort src = p.getSourcePort();
                RtNode srcNode(src.getNode());
                string srcName = srcNode.getName().str();
                if (srcNode.getName() == INPUT_SOCKETS)
                {
                    srcName = "inputs";
                }
                dot += "    \"" + srcName;
                dot += "\" -> \"" + dstName;
                dot += "\" [label=\"" + p.getName().str() + "\"];\n";
            }
        }
    };

    // Add all connections.
    for (size_t i = 0; i < numChildren(); ++i)
    {
        writeConnections(node(i), dot);
    }

    writeConnections(outputSockets(), dot);
    writeConnections(inputSockets(), dot);

    dot += "}\n";

    return dot;
}

}
