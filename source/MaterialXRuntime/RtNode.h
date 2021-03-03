//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODE_H
#define MATERIALX_RTNODE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

/// @class RtNode
/// Schema for node prims.
class RtNode : public RtTypedSchema
{
    DECLARE_TYPED_SCHEMA(RtNode)

public:
    /// Constructor.
    RtNode(const RtPrim& prim) : RtTypedSchema(prim) {}

    /// Set the nodedef for this node.
    void setNodeDef(RtPrim prim);

    /// Return the nodedef for this node.
    RtPrim getNodeDef() const;

    /// Set the version for this node.
    void setVersion(const RtToken& version);

    /// Return the version for this node.
    const RtToken& getVersion() const;

    /// Return the number of inputs on the node.
    /// Shorthand for calling getPrim().numInputs().
    size_t numInputs() const
    {
        return getPrim().numInputs();
    }

    /// Return an input by index.
    /// Shorthand for calling getPrim().getInput().
    RtInput getInput(size_t index) const
    {
        return getPrim().getInput(index);
    }

    /// Return an input by name.
    /// Shorthand for calling getPrim().getInput().
    RtInput getInput(const RtToken& name) const
    {
        return getPrim().getInput(name);
    }

    /// Return an iterator over all inputs.
    /// Shorthand for calling getPrim().getInputs().
    RtInputIterator getInputs() const
    {
        return getPrim().getInputs();
    }

    /// Return the number of outputs on the node.
    /// Shorthand for calling getPrim().numOutputs().
    size_t numOutputs() const
    {
        return getPrim().numOutputs();
    }

    /// Return an output by index.
    /// Shorthand for calling getPrim().getOutput().
    RtOutput getOutput(size_t index = 0) const
    {
        return getPrim().getOutput(index);
    }

    /// Return an output by name.
    /// Shorthand for calling getPrim().getOutput().
    RtOutput getOutput(const RtToken& name) const
    {
        return getPrim().getOutput(name);
    }

    /// Return an iterator over all outputs.
    /// Shorthand for calling getPrim().getOutputs().
    RtOutputIterator getOutputs() const
    {
        return getPrim().getOutputs();
    }

    /// Return attribute names for the attributes that have been
    /// defined as standard for this schema.
    const RtTokenVec& getStandardAttributeNames() const override;

    /// Return attribute names for the attributes that have been
    /// defined as standard for the given port on this schema.
    const RtTokenVec& getStandardAttributeNames(const RtToken& portName) const override;

    /// Return true if the given attribute is a standard attribute
    /// defined for this schema.
    bool isStandardAttribute(const RtToken& attrName) const override;

    /// Return true if the given attribute is a standard attribute
    /// defined for the given port on this schema.
    bool isStandardAttribute(const RtToken& attrName, const RtToken& portName) const override;
};

}

#endif

