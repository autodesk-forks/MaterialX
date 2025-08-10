//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_METASHADENODE_H
#define MATERIALX_METASHADENODE_H

#include "Export.h"
#include "Library.h"

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

class ShaderGenerator;

/// @class MetashadeNode
/// Custom shader node implementation that integrates with Python-based 
/// Metashade generator for runtime shader code generation.
/// 
/// This node embeds a Python interpreter and calls into Metashade
/// to generate shader code dynamically based on runtime conditions
/// such as input attribute values and context.
class MX_METASHADE_API MetashadeNode : public ShaderNodeImpl
{
  public:
    /// Create a new MetashadeNode instance
    static ShaderNodeImplPtr create();

    /// Register MetashadeNode implementations with a shader generator
    /// @param generator The shader generator to register implementations with
    static void registerImplementations(ShaderGenerator& generator);

    /// Initialize the node with the given implementation element
    void initialize(const InterfaceElement& element, GenContext& context) override;

    /// Emit function definition for the given node instance
    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    /// Emit the function call for the given node instance
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    /// Protected constructor
    MetashadeNode();
};

MATERIALX_NAMESPACE_END

#endif
