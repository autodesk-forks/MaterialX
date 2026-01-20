//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHMIXBSDFPRUNINGPASS_H
#define MATERIALX_SHADERGRAPHMIXBSDFPRUNINGPASS_H

/// @file
/// Mix BSDF pruning optimization pass

#include <MaterialXGenShader/ShaderGraphOptimizationPass.h>

MATERIALX_NAMESPACE_BEGIN

/// @class MixBsdfPruningPass
/// Prunes mix_bsdf nodes when the mix factor is a compile-time constant (0 or 1).
/// 
/// When mix=0, forwards the background input; when mix=1, forwards the foreground.
/// This eliminates dead branches in the shader graph, reducing shader complexity.
///
/// Example:
///   mix_bsdf(diffuse, glossy, 0.0) → diffuse (glossy branch eliminated)
///   mix_bsdf(diffuse, glossy, 1.0) → glossy (diffuse branch eliminated)
class MX_GENSHADER_API MixBsdfPruningPass : public ShaderGraphOptimizationPass
{
  public:
    const string& getName() const override;
    bool run(ShaderGraph& graph, GenContext& context) override;

  private:
    /// Get the compile-time constant value of an input, if available
    bool getConstantFloatValue(const ShaderInput* input, float& outValue) const;

    /// Prune a mix node where the mix factor is 0 or 1
    bool pruneMixNode(ShaderGraph& graph, ShaderNode* mixNode, float mixValue);
};

MATERIALX_NAMESPACE_END

#endif

