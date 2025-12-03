//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHDEBUG_H
#define MATERIALX_SHADERGRAPHDEBUG_H

/// @file
/// Debug utilities for ShaderGraph visualization

#include <MaterialXGenShader/Export.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXFormat/File.h>

#include <iosfwd>

MATERIALX_NAMESPACE_BEGIN

/// @name ShaderGraph Debug Utilities
/// @{

/// Write a ShaderGraph to Graphviz DOT format.
/// 
/// Usage:
/// @code
/// std::ofstream file("graph.dot");
/// writeShaderGraphDot(graph, file);
/// @endcode
/// 
/// Then visualize with: `dot -Tpng graph.dot -o graph.png`
///
MX_GENSHADER_API void writeShaderGraphDot(const ShaderGraph& graph, std::ostream& os);

/// Write a ShaderGraph to a DOT file at the given path.
MX_GENSHADER_API void writeShaderGraphDot(const ShaderGraph& graph, const FilePath& path);

/// @}

MATERIALX_NAMESPACE_END

#endif

