//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHLOBEPRUNINGPASS_H
#define MATERIALX_SHADERGRAPHLOBEPRUNINGPASS_H

/// @file
/// Lobe pruning optimization pass

#include <MaterialXGenShader/ShaderGraphOptimizationPass.h>

MATERIALX_NAMESPACE_BEGIN

/// @class LobePruningPass
/// Eliminates unused BSDF lobes when weight parameters are compile-time constants (0 or 1).
/// 
/// Algorithm:
/// 1. **Mix nodes**: If mix factor is 0 or 1, forward one input and delete the node
/// 2. **Multiply nodes**: If either input is 0, propagate zero upstream and delete the node
/// 3. **BSDF nodes**: If weight parameter is 0, substitute with no-op "dark" BSDF
///
/// This creates specialized shader variants for different material configurations,
/// leading to runtime GPU performance improvements (up to 4x per OpenUSD PR #3525).
///
/// Implementation based on:
/// - Maya USD: lib/mayaUsd/render/MaterialXGenOgsXml/LobePruner.cpp
/// - OpenUSD: pxr/imaging/hdSt/materialXLobePruner.cpp
class MX_GENSHADER_API LobePruningPass : public ShaderGraphOptimizationPass
{
  public:
    const string& getName() const override;
    bool run(ShaderGraph& graph, GenContext& context) override;
    bool isEnabled(GenContext& context) const override;

  private:
    /// Check if an input is a prunable lobe weight parameter
    /// (float type, 0-1 range via uimin/uimax, compile-time constant)
    bool isLobeWeightInput(const ShaderInput* input) const;

    /// Get the compile-time constant value of an input, if available
    bool getConstantFloatValue(const ShaderInput* input, float& outValue) const;

    /// Check if a node is a BSDF node that can be pruned
    bool isPrunableBsdfNode(const ShaderNode* node) const;

    /// Replace a BSDF node with a "dark" (no-op) BSDF
    void replaceBsdfWithDark(ShaderGraph& graph, ShaderNode* node, GenContext& context);

    /// Prune a mix node where the mix factor is 0 or 1
    bool pruneMixNode(ShaderGraph& graph, ShaderNode* mixNode, float mixValue);

    /// Prune a multiply node where one input is 0
    bool pruneMultiplyNode(ShaderGraph& graph, ShaderNode* multiplyNode);
};

MATERIALX_NAMESPACE_END

#endif

