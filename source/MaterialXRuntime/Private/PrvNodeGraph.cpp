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

const RtToken PrvNodeGraph::INTERNAL_NODEDEF("__internal_nodedef__");
const RtToken PrvNodeGraph::INTERNAL_NODE("__internal_node__");

PrvNodeGraph::PrvNodeGraph(const RtToken& name) :
    PrvNode(name)
{
    _internalNodeDef = PrvNodeDef::createNew(INTERNAL_NODEDEF, INTERNAL_NODEDEF);
    _internalNode = PrvNode::createNew(INTERNAL_NODE, _internalNodeDef);
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

/*
void PrvNodeGraph::createInterface(PrvObjectHandle nodedef)
{
    _externalNodeDef = nodedef;

    PrvNodeDef* externDef = externalNodeDef();
    PrvNodeDef* internDef = internalNodeDef();

    // Create the internal interface as a mirror of the external interface.
    // 
    // First the inputs which turn into outputs.
    for (size_t i = externDef->numOutputs(); i < externDef->numPorts(); ++i)
    {
        const PrvPortDef* p = externDef->port(i);
        uint32_t flags = p->getFlags();
        flags &= ~RtPortFlag::INPUT;
        flags |= RtPortFlag::OUTPUT;
        PrvObjectHandle port = PrvPortDef::createNew(p->getName(), p->getType(), p->getValue(), flags);
        internDef->addPort(port);
    }
    // Then the outputs which turn into inputs.
    for (size_t i = 0; i < externDef->numOutputs(); ++i)
    {
        const PrvPortDef* p = externDef->port(i);
        uint32_t flags = p->getFlags();
        flags &= ~RtPortFlag::OUTPUT;
        flags |= RtPortFlag::INPUT;
        PrvObjectHandle port = PrvPortDef::createNew(p->getName(), p->getType(), p->getValue(), flags);
        internDef->addPort(port);
    }

    _externalNode = PrvNode::createNew(EXTERNAL_NODE, _externalNodeDef);
    _internalNode = PrvNode::createNew(INTERNAL_NODE, _internalNodeDef);
}
*/

void PrvNodeGraph::addPort(PrvObjectHandle portdef)
{
    nodedef()->addPort(portdef);

    PrvNodeDef* internDef = internalNodeDef();
    PrvNode* internNode = internalNode();

    const PrvPortDef* pd = portdef->asA<PrvPortDef>();
    uint32_t flags = pd->getFlags();

    PrvNode::Port p;
    p.value = pd->getValue();
    p.colorspace = pd->getColorSpace();
    p.unit = pd->getUnit();

    if (pd->isInput())
    {
        // Insert last among the inputs
        _ports.push_back(p);

        // Create a portdef for internal node
        // reversing type input<->output
        flags &= ~RtPortFlag::INPUT;
        flags |= RtPortFlag::OUTPUT;
        internDef->addPort(PrvPortDef::createNew(pd->getName(), pd->getType(), pd->getValue(), flags));

        // Insert last among the internal outputs
        auto it = internNode->_ports.begin() + internNode->numOutputs();
        internNode->_ports.insert(it, 1, p);
    }
    else // an output
    {
        // Insert last among the outputs
        auto it = _ports.begin() + numOutputs();
        _ports.insert(it, 1, p);

        // Create a portdef for internal node
        // reversing type input<->output
        flags &= ~RtPortFlag::OUTPUT;
        flags |= RtPortFlag::INPUT;
        internDef->addPort(PrvPortDef::createNew(pd->getName(), pd->getType(), pd->getValue(), flags));

        // Insert last among the internal inputs
        internNode->_ports.push_back(p);
    }
}

void PrvNodeGraph::removePort(const RtToken& name)
{
    size_t index = nodedef()->findPortIndex(name);
    if (index != INVALID_INDEX)
    {
        _ports.erase(_ports.begin() + index);
    }
    nodedef()->removePort(name);

    PrvNodeDef* internDef = internalNodeDef();
    PrvNode* internNode = internalNode();

    index = internDef->findPortIndex(name);
    if (index != INVALID_INDEX)
    {
        internNode->_ports.erase(internNode->_ports.begin() + index);
    }
    internDef->removePort(name);
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
        if (n->getName() == INTERNAL_NODE)
        {
            dstName += "outputs";
        }
        for (size_t j = 0; j < n->numPorts(); ++j)
        {
            RtPort p = n->getPort(j);
            if (p.isInput() && p.isConnected())
            {
                RtPort src = p.getSourcePort();
                RtNode srcNode(src.getNode());
                string srcName = srcNode.getName().str();
                if (srcNode.getName() == INTERNAL_NODE)
                {
                    srcName += "inputs";
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

    writeConnections(internalNode(), dot);

    dot += "}\n";

    return dot;
}

}
