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

const RtToken PrvNodeGraph::INPUTS("inputs");
const RtToken PrvNodeGraph::OUTPUTS("outputs");
const RtToken PrvNodeGraph::GRAPH_INPUTS("graphoutputs");
const RtToken PrvNodeGraph::GRAPH_OUTPUTS("graphinputs");
const RtToken PrvNodeGraph::GRAPH_INTERFACE_CATEGORY("__graphinterface"); // internal name so use prefix to reduce risk of name collisions
const RtToken PrvNodeGraph::ATTR_NODEDEF("nodedef");

PrvNodeGraph::PrvNodeGraph(const RtToken& name) :
    PrvCompound(RtObjType::NODEGRAPH, name)
{
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
    PrvNode* n = node->asA<PrvNode>();
    if (_elementsByName.count(n->getName()))
    {
        throw ExceptionRuntimeError("A node named '" + n->getName().str() + "' already exists for nodegraph '" + getName().str() + "'");
    }
    _elements.push_back(node);
    _elementsByName[n->getName()] = node;
}

void PrvNodeGraph::setInterface(PrvObjectHandle nodedef)
{
    if (!nodedef->hasApi(RtApiType::NODEDEF))
    {
        throw ExceptionRuntimeError("Given object is not a valid nodedef");
    }

    PrvNodeDef* nd = nodedef->asA<PrvNodeDef>();

    if (nd->getCategory() != GRAPH_INTERFACE_CATEGORY)
    {
        addAttribute(ATTR_NODEDEF, RtType::TOKEN, RtValue(nd->getName()));
    }

    // Create nodedefs for the input and output interface nodes.
    _inputsDef = PrvNodeDef::createNew(INPUTS, GRAPH_INPUTS);
    _outputsDef = PrvNodeDef::createNew(OUTPUTS, GRAPH_OUTPUTS);

    for (size_t i = 0; i < nd->numElements(); ++i)
    {
        const PrvPortDef* p = nd->port(i);
        uint32_t flags = p->getFlags();
        PrvNodeDef* def;
        if (p->isInput())
        {
            // And interface input turns into an output on the inputs node.
            flags &= ~RtPortFlag::INPUT;
            flags |= RtPortFlag::OUTPUT | RtPortFlag::INTERFACE;
            def = _inputsDef->asA<PrvNodeDef>();
        }
        else
        {
            // And interface output turns into an input on the outputs node.
            flags &= ~RtPortFlag::OUTPUT;
            flags |= RtPortFlag::INPUT | RtPortFlag::INTERFACE;
            def = _outputsDef->asA<PrvNodeDef>();
        }
        PrvObjectHandle port = PrvPortDef::createNew(p->getName(), p->getType(), p->getValue(), flags);
        def->addPort(port);
    }

    // Instantiate the input and output interface nodes.
    _inputsNode = PrvNode::createNew(INPUTS, _inputsDef);
    _outputsNode = PrvNode::createNew(OUTPUTS, _outputsDef);
}

string PrvNodeGraph::asStringDot() const
{
    string dot = "digraph {\n";

    // Add alla nodes.
    PrvNode* inputs = inputsNode();
    dot += "    \"" + inputs->getName().str() + "\" ";
    dot += "[shape=box];\n";
    PrvNode* outputs = outputsNode();
    dot += "    \"" + outputs->getName().str() + "\" ";
    dot += "[shape=box];\n";

    // Add alla nodes.
    for (size_t i = 0; i < numNodes(); ++i)
    {
        dot += "    \"" + node(i)->getName().str() + "\" ";
        dot += "[shape=ellipse];\n";
    }

    auto writeConnections = [](PrvNode* n, string& dot)
    {
        for (size_t j = 0; j < n->numPorts(); ++j)
        {
            RtPort p = n->getPort(j);
            if (p.isInput() && p.isConnected())
            {
                RtPort src = p.getSourcePort();
                RtNode srcNode(src.getNode());
                dot += "    \"" + srcNode.getName().str();
                dot += "\" -> \"" + n->getName().str();
                dot += "\" [label=\"" + p.getName().str() + "\"];\n";
            }
        }
    };

    // Add all connections.
    for (size_t i = 0; i < numNodes(); ++i)
    {
        writeConnections(node(i), dot);
    }
    writeConnections(outputs, dot);

    dot += "}\n";

    return dot;
}

}
