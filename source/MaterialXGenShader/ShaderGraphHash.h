//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHHASH_H
#define MATERIALX_SHADERGRAPHHASH_H

/// @file
/// Structural hash computation for shader graphs

#include <MaterialXGenShader/Export.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/ShaderGraph.h>

MATERIALX_NAMESPACE_BEGIN

/// Compute a pure structural hash of a shader graph that captures its
/// topology and node types, independent of instance names and values.
///
/// Two graphs with identical structure (same node implementations,
/// same connection pattern, same port types) will produce the same
/// hash even if they differ in node names, variable names, or uniform
/// values. The hash is computed over the finalized graph and includes:
///   - Graph input/output socket counts and types
///   - Node implementation hashes and classifications
///   - Connection topology (source node topological index + output index)
///   - Structurally-significant port attributes: type, semantic,
///     colorspace, unit, geomprop, uniform/bind-input flags
///
/// Explicitly excluded: node names, port names, port values/defaults.
///
/// This is an external visitor function that only uses the public API
/// of ShaderGraph, ShaderNode, and ShaderPort.
MX_GENSHADER_API size_t computeStructuralHash(const ShaderGraph& graph);

/// Compute a structural hash that also incorporates the GenOptions
/// settings that influence graph topology.
///
/// Several GenOptions fields cause the ShaderGraph to be built with a
/// different set of nodes or connections (e.g. emitColorTransforms,
/// elideConstantNodes, shaderInterfaceType, addUpstreamDependencies,
/// targetColorSpaceOverride, targetDistanceUnit). Hashing these into
/// the seed makes the hash self-describing: two hashes are only
/// comparable when they were produced under the same generation
/// settings.
///
/// Prefer this overload when the hash will be used for caching or
/// equivalence testing across different generation configurations.
MX_GENSHADER_API size_t computeStructuralHash(const ShaderGraph& graph, const GenOptions& options);

MATERIALX_NAMESPACE_END

#endif
