//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXMetashade/MetashadeNode.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr MetashadeNode::create()
{
    return ShaderNodeImplPtr(new MetashadeNode());
}

void MetashadeNode::registerImplementations(ShaderGenerator& generator)
{
    // Register MetashadeNode for specific node types
    // You can register it for multiple node categories/names as needed
    
    // Example: Register for custom Metashade nodes
    generator.registerImplementation("IM_Metashade", MetashadeNode::create);
}

MetashadeNode::MetashadeNode()
{
    // Constructor - ready for future implementation
}

void MetashadeNode::initialize(const InterfaceElement& element, GenContext& context)
{
    ShaderNodeImpl::initialize(element, context);
    
    // Set a specific hash for the purple function so it's only emitted once
    _hash = std::hash<string>{}("mx_metashade_purple");
}

void MetashadeNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    
    // Emit a simple function that returns purple color
    shadergen.emitLine("vec3 mx_metashade_purple()", stage, false);
    shadergen.emitScopeBegin(stage);
    shadergen.emitLine("return vec3(0.5, 0.0, 1.0); // Purple color", stage);
    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
}

void MetashadeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    
    // Emit the function call to get purple color
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(node.getOutput(), true, false, context, stage);
    shadergen.emitString(" = mx_metashade_purple()", stage);
    shadergen.emitLineEnd(stage);
}

MATERIALX_NAMESPACE_END
