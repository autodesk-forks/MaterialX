//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SHADERNODEIMPL_H
#define MATERIALX_SHADERNODEIMPL_H

/// @file
/// Base class for shader node implementations

#include <MaterialXGenShader/Library.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

class InterfaceElement;
class Node;
using ShaderGraphInputSocket = ShaderOutput;

/// Shared pointer to a ShaderNodeImpl
using ShaderNodeImplPtr = shared_ptr<class ShaderNodeImpl>;

/// Flags for tagging shader node implementation.
enum class ShaderNodeImplFlag
{
    /// Ignore the function call for this node.
    IGNORE_FUNCTION_CALL = 0x1 << 0,
};

/// @class ShaderNodeImpl
/// Class handling the shader generation implementation for a node.
/// Responsible for emitting the function definition and function call 
/// that is the node implementation.
class ShaderNodeImpl
{
  public:
    virtual ~ShaderNodeImpl() { }

    /// Return an identifier for the language used by this implementation.
    /// By default an empty string is returned, representing any language.
    /// Only override this method if your derived node implementation class
    /// is for a specific language.
    virtual const string& getLanguage() const { return EMPTY_STRING; }

    /// Return an identifier for the target used by this implementation.
    /// By default an empty string is returned, representing all targets.
    /// Only override this method if your derived node implementation class
    /// is for a specific target.
    virtual const string& getTarget() const { return EMPTY_STRING; }

    /// Initialize with the given implementation element.
    /// Initialization must set the name and hash for the implementation,
    /// as well as any other data needed to emit code for the node.
    virtual void initialize(const InterfaceElement& element, GenContext& context);

    /// Return the name of this implementation.
    const string& getName() const
    {
        return _name;
    }

    /// Return a hash for this implementation.
    /// The hash should correspond to the function signature generated for the node,
    /// and can be used to compare implementations, e.g. to query if an identical
    /// function has already been emitted during shader generation.
    size_t getHash() const
    {
        return _hash;
    }

    /// Add additional inputs on the node 
    virtual void addInputs(ShaderNode& node, GenContext& context) const;

    /// Set values for additional inputs on the node 
    virtual void setValues(const Node& node, ShaderNode& shaderNode, GenContext& context) const;

    /// Create shader variables needed for the implementation of this node (e.g. uniforms, inputs and outputs).
    /// Used if the node requires input data from the application.
    virtual void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const;

    /// Emit function definition for the given node instance.
    virtual void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const;

    /// Emit the function call or inline source code for given node instance in the given context.
    virtual void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const;

    /// Return a pointer to the graph if this implementation is using a graph,
    /// or returns nullptr otherwise.
    virtual ShaderGraph* getGraph() const;

    /// Returns true if an input is editable by users.
    /// Editable inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    /// By default all inputs are considered to be editable.
    virtual bool isEditable(const ShaderInput& /*input*/) const
    {
        return true;
    }

    /// Returns true if a graph input is accessible by users.
    /// Accessible inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    /// By default all graph inputs are considered to be acessible.
    virtual bool isEditable(const ShaderGraphInputSocket& /*input*/) const
    {
        return true;
    }

    /// Set the on|off state of a given flag.
    void setFlag(ShaderNodeImplFlag flag, bool value)
    {
        _flags = value ? (_flags | uint32_t(flag)) : (_flags & ~uint32_t(flag));
    }

    /// Return the on|off state of a given flag.
    bool getFlag(ShaderNodeImplFlag flag) const
    {
        return _flags & uint32_t(flag);
    }

  protected:
    /// Protected constructor
    ShaderNodeImpl();

  protected:
    string _name;
    size_t _hash;
    uint32_t _flags;
};

} // namespace MaterialX

#endif
