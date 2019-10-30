//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTTYPES_H
#define MATERIALX_RTTYPES_H

/// @file
/// Type identifiers and type descriptors for runtime data types.

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtToken.h>

namespace MaterialX
{

/// Class holding identifier tokens for the built in data types.
class RtType
{
public:
    static const RtToken BOOLEAN;
    static const RtToken INTEGER;
    static const RtToken FLOAT;
    static const RtToken VECTOR2;
    static const RtToken VECTOR3;
    static const RtToken VECTOR4;
    static const RtToken COLOR2;
    static const RtToken COLOR3;
    static const RtToken COLOR4;
    static const RtToken MATRIX33;
    static const RtToken MATRIX44;
    static const RtToken TOKEN;
    static const RtToken STRING;
    static const RtToken FILENAME;
    static const RtToken INTEGERARRAY;
    static const RtToken FLOATARRAY;
    static const RtToken BSDF;
    static const RtToken EDF;
    static const RtToken VDF;
    static const RtToken SURFACESHADER;
    static const RtToken VOLUMESHADER;
    static const RtToken DISPLACEMENTSHADER;
    static const RtToken LIGHTSHADER;
    static const RtToken SURFACEMATERIAL;
    static const RtToken VOLUMEMATERIAL;
    static const RtToken AUTO;
};

/// @class RtTypeDef
/// A type definition for MaterialX data types.
/// All types need to have a type definition registered in order for the runtime
/// to know about the type. A unique type definition pointer can be used for type
/// comparisons as well as getting more information about the type.
/// All standard library data types are registered by default. Custom types can be
/// registered using RtTypeDef::registerType().
class RtTypeDef
{
public:
    /// Identifiers for base types.
    static const RtToken BASETYPE_NONE;
    static const RtToken BASETYPE_BOOLEAN;
    static const RtToken BASETYPE_INTEGER;
    static const RtToken BASETYPE_FLOAT;
    static const RtToken BASETYPE_STRING;
    static const RtToken BASETYPE_STRUCT;

    /// Identifiers for type semantics.
    static const RtToken SEMANTIC_NONE;
    static const RtToken SEMANTIC_COLOR;
    static const RtToken SEMANTIC_VECTOR;
    static const RtToken SEMANTIC_MATRIX;
    static const RtToken SEMANTIC_FILENAME;
    static const RtToken SEMANTIC_CLOSURE;
    static const RtToken SEMANTIC_SHADER;

public:
    /// Constructor.
    RtTypeDef(const RtToken& name, const RtToken& basetype, const RtToken& semantic, size_t size);

    /// Destructor.
    ~RtTypeDef();

    /// Return the name of the type.
    const RtToken& getName() const;

    /// Return the basetype for the type.
    const RtToken& getBaseType() const;

    /// Return the semantic for the type.
    const RtToken& getSemantic() const;

    /// Return the number of elements the type is composed of.
    /// Will return 1 for scalar types and a size greater than 1 for aggregate type.
    /// For array types 0 is returned since the number of elements is undefined
    /// until an array is instantiated.
    size_t getSize() const;

    /// Returns the channel index for the supplied channel name.
    /// Will return -1 on failure to find a matching index.
    int getChannelIndex(char channel) const;

    /// Set the channel index for the supplied channel name.
    void setChannelIndex(char channel, int index);

    /// Return true if the type is a scalar type.
    bool isScalar() const
    {
        return getSize() == 1;
    }

    /// Return true if the type is an aggregate type.
    bool isAggregate() const
    {
        return getSize() > 1;
    }

    /// Return true if the type is an array type.
    bool isArray() const
    {
        return getSize() == 0;
    }

    /// Return true if the type is an aggregate of 2 floats.
    bool isFloat2() const 
    { 
        return getSize() == 2 && (getSemantic() == SEMANTIC_COLOR || getSemantic() == SEMANTIC_VECTOR);
    }

    /// Return true if the type is an aggregate of 3 floats.
    bool isFloat3() const
    {
        return getSize() == 3 && (getSemantic() == SEMANTIC_COLOR || getSemantic() == SEMANTIC_VECTOR);
    }

    /// Return true if the type is an aggregate of 4 floats.
    bool isFloat4() const
    {
        return getSize() == 4 && (getSemantic() == SEMANTIC_COLOR || getSemantic() == SEMANTIC_VECTOR);
    }

    /// Return a set of all types that this type can be connected to.
    /// The type itself is also included in this set.
    const RtTokenSet& getValidConnectionTypes() const;

    /// Register a type descriptor for a MaterialX data type.
    /// Throws an exception if a type with the same name is already registered.
    static RtTypeDef* registerType(const RtToken& name, const RtToken& basetype,
                                    const RtToken& semantic = SEMANTIC_NONE, size_t size = 1);

    /// Get a type descriptor for given type name.
    /// Returns nullptr if no such type is registered.
    static const RtTypeDef* findType(const RtToken& name);

private:
    void* _ptr;
};

} // namespace MaterialX

#endif
