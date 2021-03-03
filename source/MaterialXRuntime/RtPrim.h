//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPRIM_H
#define MATERIALX_RTPRIM_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtTypeInfo.h>
#include <MaterialXRuntime/RtPort.h>
#include <MaterialXRuntime/RtRelationship.h>

namespace MaterialX
{

class RtPrimIterator;
class RtInputIterator;
class RtOutputIterator;
class RtRelationshipIterator;
class RtSchemaBase;

/// @class RtPrim
class RtPrim : public RtObject
{
    RT_DECLARE_RUNTIME_OBJECT(RtPrim)

public:
    /// Empty constructor.
    /// Creating an invalid object.
    RtPrim() {}

    /// Construct from a data handle.
    RtPrim(PvtDataHandle hnd);

    /// Construct from an object.
    RtPrim(RtObject obj);

    /// Return the typeinfo for this prim.
    const RtTypeInfo* getTypeInfo() const;

    /// Return true if this prim supports the templated schema class.
    template<class T>
    bool hasApi() const
    {
        static_assert(std::is_base_of<RtSchemaBase, T>::value,
            "Templated type must be a concrete subclass of RtSchemaBase");
        return getTypeInfo()->isCompatible(T::typeName());
    }

    /// Return the typename for this prim.
    const RtToken& getTypeName() const
    {
        return getTypeInfo()->getShortTypeName();
    }

    /// Add a relationship to the prim.
    RtRelationship createRelationship(const RtToken& name);

    /// Remove a relationship from the prim.
    void removeRelationship(const RtToken& name);

    /// Return a relationship by name, or a null object
    /// if no such relationship exists.
    RtRelationship getRelationship(const RtToken& name) const;

    /// Return an iterator over all relationships of this prim.
    RtRelationshipIterator getRelationships() const;

    /// Add an input port to the prim.
    RtInput createInput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an input port from the prim.
    void removeInput(const RtToken& name);

    /// Return the number of inputs on the prim.
    size_t numInputs() const;

    /// Return an input port by index.
    RtInput getInput(size_t index) const;

    /// Return an input port by name, or a null object
    /// if no such input port exists.
    RtInput getInput(const RtToken& name) const;

    /// Return an iterator over all inputs.
    RtInputIterator getInputs() const;

    /// Add an output port to the prim.
    RtOutput createOutput(const RtToken& name, const RtToken& type, uint32_t flags = 0);

    /// Remove an output port from the prim.
    void removeOutput(const RtToken& name);

    /// Return the number of outputs on the prim.
    size_t numOutputs() const;

    /// Return an output port by index.
    RtOutput getOutput(size_t index = 0) const;

    /// Return an output port by name, or a null object
    /// if no such output port exists.
    RtOutput getOutput(const RtToken& name) const;

    /// Return an iterator over all outputs.
    RtOutputIterator getOutputs() const;

    /// Return the number of children in the prim.
    size_t numChildren() const;

    /// Return a child prim by index.
    RtPrim getChild(size_t index) const;

    /// Return a child prim by name, or a null object
    /// if no such child prim exists.
    RtPrim getChild(const RtToken& name) const;

    /// Return an iterator traversing all child prims (siblings).
    /// Using a predicate this method can be used to find all child prims
    /// of a specific object type, or all child prims supporting a
    /// specific API, etc.
    RtPrimIterator getChildren(RtObjectPredicate predicate = nullptr) const;
};

/// Function type for creating prims for a typed schema.
using RtPrimCreateFunc = std::function<RtPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)>;

}

#endif
