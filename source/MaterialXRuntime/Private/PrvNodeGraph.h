//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PRVNODEGRAPH_H
#define MATERIALX_PRVNODEGRAPH_H

#include <MaterialXRuntime/Private/PrvElement.h>
#include <MaterialXRuntime/Private/PrvNode.h>

#include <MaterialXRuntime/RtObject.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

class PrvNodeGraph : public PrvNode
{
public:
    PrvNodeGraph(const RtToken& name);

    static PrvObjectHandle createNew(const RtToken& name);

    void addNode(PrvObjectHandle node);


    PrvObjectHandle createInterface(PrvObjectHandle nodedef);


    void addPort(PrvObjectHandle portdef);

    void removePort(const RtToken& name);


    RtPort getInternalPort(size_t index)
    {
        PrvPortDef* portdef = internalNodeDef()->port(index);
        return portdef ? RtPort(_internalNode, index) : RtPort();
    }

    RtPort findInternalPort(const RtToken& name)
    {
        const size_t index = findInternalPortIndex(name);
        return index != INVALID_INDEX ? RtPort(_internalNode, index) : RtPort();
    }

    size_t findInternalPortIndex(const RtToken& name)
    {
        return internalNodeDef()->findPortIndex(name);
    }

    string asStringDot() const;

    PrvNode* node(const RtToken& name) const { return (PrvNode*)findChildByName(name).get(); }
    PrvNode* node(size_t index) const { return (PrvNode*)getChild(index).get(); }

    PrvNodeDef* internalNodeDef() const { return (PrvNodeDef*)_internalNodeDef.get(); }
    PrvNode* internalNode() const { return (PrvNode*)_internalNode.get(); }

    // Token constants.
    static const RtToken INTERNAL_NODEDEF;
    static const RtToken INTERNAL_NODE;

protected:
    PrvObjectHandle _internalNodeDef;
    PrvObjectHandle _internalNode;
};

}

#endif
