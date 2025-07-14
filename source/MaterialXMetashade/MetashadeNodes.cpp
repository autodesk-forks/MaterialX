//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXMetashade/MetashadeNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Register MetashadeNode implementations with a shader generator
/// 
/// @param generator The shader generator to register implementations with
/// 
/// Example usage:
/// @code
/// #ifdef MATERIALX_BUILD_METASHADE
///   registerMetashadeNodes(generator);
/// #endif
/// @endcode
void registerMetashadeNodes(ShaderGenerator& generator)
{
    // Register MetashadeNode for specific node types
    // You can register it for multiple node categories/names as needed
    
    // Example: Register for custom Metashade nodes
    generator.registerImplementation("metashade_custom", MetashadeNode::create);
    generator.registerImplementation("metashade_procedural", MetashadeNode::create);
    
    // Example: Register for specific node categories that should use Metashade
    // generator.registerImplementation("custom_texture", MetashadeNode::create);
    // generator.registerImplementation("procedural_noise", MetashadeNode::create);
}

MATERIALX_NAMESPACE_END
