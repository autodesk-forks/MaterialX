//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvNode.h>
#include <MaterialXRuntime/RtToken.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

PrvNode::Port::Port() :
    colorspace(EMPTY_TOKEN),
    unit(EMPTY_TOKEN)
{
}

PrvNode::PrvNode(const RtToken& name, const PrvObjectHandle& nd, RtObjType /*objType*/) :
    PrvValueStoringElement(RtObjType::NODE, name),
    _nodedef(nd)
{
    const size_t numPorts = nodedef()->numPorts();
    _ports.resize(numPorts);

    // Set indices and default values
    for (size_t i = 0; i < numPorts; ++i)
    {
        PrvPortDef* p = nodedef()->port(i);
        _ports[i].value = p->getValue();
    }
}

PrvNode::PrvNode(const RtToken& name, RtObjType /*objType*/) :
    PrvValueStoringElement(RtObjType::NODEGRAPH, name),
    _nodedef(nullptr)
{
}

PrvObjectHandle PrvNode::createNew(const RtToken& name, const PrvObjectHandle& nodedef)
{
    return std::make_shared<PrvNode>(name, nodedef);
}

void PrvNode::connect(const RtPort& source, const RtPort& dest)
{
    if (dest.isConnected())
    {
        throw ExceptionRuntimeError("Destination port is already connected");
    }
    if (!(source.isOutput() && source.isConnectable()))
    {
        throw ExceptionRuntimeError("Source port is not a connectable output");
    }
    if (!(dest.isInput() && dest.isConnectable()))
    {
        throw ExceptionRuntimeError("Destination port is not a connectable input");
    }

    PrvNode* sourceNode = source._data->asA<PrvNode>();
    PrvNode* destNode = dest._data->asA<PrvNode>();
    RtPortVec& sourceConnections = sourceNode->_ports[source._index].connections;
    RtPortVec& destConnections = destNode->_ports[dest._index].connections;

    // Make room for the new source.
    destConnections.resize(1);
    destConnections[0] = source;
    sourceConnections.push_back(dest);
}

void PrvNode::disconnect(const RtPort& source, const RtPort& dest)
{
    PrvNode* destNode = dest._data->asA<PrvNode>();
    RtPortVec& destConnections = destNode->_ports[dest._index].connections;
    if (destConnections.size() != 1 || destConnections[0] != source)
    {
        throw ExceptionRuntimeError("Given source and destination is not connected");
    }

    destConnections.clear();

    PrvNode* sourceNode = source._data->asA<PrvNode>();
    RtPortVec& sourceConnections = sourceNode->_ports[source._index].connections;
    for (auto it = sourceConnections.begin(); it != sourceConnections.end(); ++it)
    {
        if (*it == dest)
        {
            sourceConnections.erase(it);
            break;
        }
    }
}

}
