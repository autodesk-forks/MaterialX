//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVNODE_H
#define MATERIALX_PRVNODE_H

#include <MaterialXRuntime/Private/PrvNodeDef.h>

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtValue.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

using RtPortVec = vector<RtPort>;

class PrvNode : public PrvValueStoringElement
{
public:
    // Constructor creating a node with a fixed interface
    // This is the constructor to use for ordinary nodes.
    PrvNode(const RtToken& name, const PrvObjectHandle& nodedef, RtObjType objType = RtObjType::NODE);

    // Constructor creating a node without a fixed interface.
    // Used for constructing nodegraphs.
    PrvNode(const RtToken& name, RtObjType objType = RtObjType::NODEGRAPH);

    static PrvObjectHandle createNew(const RtToken& name, const PrvObjectHandle& nodedef);

    PrvObjectHandle getNodeDef() const
    {
        return _nodedef;
    }

    const RtToken& getCategory() const
    {
        return nodedef()->getCategory();
    }

    size_t numPorts() const
    {
        return nodedef()->numPorts();
    }

    size_t numOutputs() const
    {
        return nodedef()->numOutputs();
    }

    size_t numInputs() const
    {
        return numPorts() - numOutputs();
    }

    size_t getOutputsOffset() const
    {
        return nodedef()->getOutputsOffset();
    }

    size_t getInputsOffset() const
    {
        return nodedef()->getInputsOffset();
    }

    RtPort getPort(size_t index)
    {
        PrvPortDef* portdef = nodedef()->getPort(index);
        return portdef ? RtPort(shared_from_this(), index) : RtPort();
    }

    RtPort findPort(const RtToken& name)
    {
        const size_t index = nodedef()->findPortIndex(name);
        return index != INVALID_INDEX ? RtPort(shared_from_this(), index) : RtPort();
    }

    static void connect(const RtPort& source, const RtPort& dest);

    static void disconnect(const RtPort& source, const RtPort& dest);

    RtLargeValueStorage& getValueStorage()
    {
        return _storage;
    }

    // Short syntax getter for convenience.
    PrvNodeDef* nodedef() { return (PrvNodeDef*)_nodedef.get(); }
    const PrvNodeDef* nodedef() const { return (PrvNodeDef*)_nodedef.get(); }

protected:
    struct Port
    {
        Port();
        RtValue value;
        RtToken colorspace;
        RtToken unit;
        RtPortVec connections;
    };

    PrvObjectHandle _nodedef;
    vector<Port> _ports;

    friend class RtPort;
    friend class PrvNodeGraph;
};

}

#endif
