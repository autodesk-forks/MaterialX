//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "MetashadeNode.h"

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

// Standard library includes
#include <iostream>

// pybind11 for Python integration
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr MetashadeNode::create()
{
    return ShaderNodeImplPtr(new MetashadeNode());
}

void MetashadeNode::registerImplementations(ShaderGenerator& generator)
{
    // Initialize Python interpreter if not already done
    static bool pythonInitialized = false;
    if (!pythonInitialized)
    {
        py::initialize_interpreter();
        pythonInitialized = true;
    }
    
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
    
    // Set a specific hash for the metashade function so it's only emitted once
    _hash = std::hash<string>{}("mx_metashade_function");
}

void MetashadeNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    try
    {
        // Import the metashade module and call the function
        py::module_ metashade_module = py::module_::import("metashade_mtlx");
        py::object get_purple_func = metashade_module.attr("get_purple_glsl_function");

        // Call the Python function to get GLSL code
        py::str glsl_code = get_purple_func();

        // Convert to C++ string and emit
        std::string cppGlslCode = glsl_code.cast<std::string>();
        stage.addString(cppGlslCode);
    }
    catch (const py::error_already_set& e)
    {
        std::cerr << "Python error in MetashadeNode: " << e.what() << std::endl;
        
        // Fallback implementation
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine("vec3 mx_metashade_function()", stage, false);
        shadergen.emitScopeBegin(stage);
        shadergen.emitLine("return vec3(0.5, 0.0, 0.5); // Fallback purple color", stage);
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in MetashadeNode: " << e.what() << std::endl;
        
        // Fallback implementation
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitLine("vec3 mx_metashade_function()", stage, false);
        shadergen.emitScopeBegin(stage);
        shadergen.emitLine("return vec3(0.5, 0.0, 0.5); // Fallback purple color", stage);
        shadergen.emitScopeEnd(stage);
        shadergen.emitLineBreak(stage);
    }
}

void MetashadeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    
    // Emit the function call
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(node.getOutput(), true, false, context, stage);
    shadergen.emitString(" = mx_metashade_function()", stage);
    shadergen.emitLineEnd(stage);
}

MATERIALX_NAMESPACE_END
