//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODEGRAPH_H
#define MATERIALX_RTNODEGRAPH_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtElement.h>

namespace MaterialX
{

/// @class RtNodeGraph
/// API for creating and editing nodegraphs. This API can only be
/// attached to objects of type NODEGRAPH.
class RtNodeGraph : public RtElement
{
public:
    /// Constructor attaching and object to the API.
    RtNodeGraph(const RtObject& obj);

    /// Create a new nodegraph and add it to a stage if specified.
    static RtObject createNew(const RtToken& name, RtObject stage = RtObject());

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Add a node to the graph.
    void addNode(RtObject node);

    /// Return the node count.
    size_t numNodes() const;

    /// Return a node by index, or a null object 
    /// if no such node exists.
    RtObject getNode(size_t index) const;

    /// Find a node by name. Return a null object 
    /// if no such node is found.
    RtObject findNode(const RtToken& name) const;

    /// Return the port count.
    size_t numPorts() const;

    /// Return the port count.
    size_t numOutputs() const;

    /// Return a port by index, or a null object 
    /// if no such port exists.
    RtPort getPort(size_t index) const;

    /// Find a port by name, or a null object 
    /// if no such port is found.
    RtPort findPort(const RtToken& name) const;

    /// Convert this graph to a string in the DOT language syntax. This can be
    /// used to visualise the graph using GraphViz (http://www.graphviz.org).
    string asStringDot() const;
};

}

#endif
