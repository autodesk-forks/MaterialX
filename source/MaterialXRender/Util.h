//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RENDER_UTIL_H
#define MATERIALX_RENDER_UTIL_H

/// @file
/// Rendering utility methods

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

namespace MaterialX
{
    /// @name Shader utilities
    /// @{

    /// Create a shader for a given element
    ShaderPtr createShader(const string& shaderName, GenContext& context, ElementPtr elem);

    /// Create a shader with a constant color output for a given element
    ShaderPtr createConstantShader(GenContext& context,
                                   DocumentPtr stdLib,
                                   const string& shaderName,
                                   const Color3& color);

    /// @}
    /// @name Introspection utilities
    /// @{ 

    /// Get a named uniform block for a given shader stage
    VariableBlock* getUniformBlock(ShaderPtr shader, const string& stageName, const string& blockName);

    /// Find a variable in shader's variable block which matches a given Element path.
    ShaderPort* findUniform(const VariableBlock* block, const std::string& path);

    /// @}

} // namespace MaterialX

#endif