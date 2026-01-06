//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHSTANDARDPASSES_H
#define MATERIALX_SHADERGRAPHSTANDARDPASSES_H

/// @file
/// Standard shader graph optimization passes

#include <MaterialXGenShader/ShaderGraphOptimizationPass.h>

MATERIALX_NAMESPACE_BEGIN

/// @class ConstantFoldingPass
/// Elides constant nodes by moving their values downstream
class MX_GENSHADER_API ConstantFoldingPass : public ShaderGraphOptimizationPass
{
  public:
    const string& getName() const override;
    bool run(ShaderGraph& graph, GenContext& context) override;
};

/// @class PremultipliedAddPass
/// Transforms mix(A, B, weight) into add(A * (1-weight), B * weight)
/// for BSDF nodes, exposing weight multiplications for GPU optimization.
/// Implemented by Lee Kerley in MaterialX PR #2499.
/// 
/// Note: With iterative optimization, nested mix nodes (MixMix pattern)
/// are automatically handled by applying this pass multiple times.
class MX_GENSHADER_API PremultipliedAddPass : public ShaderGraphOptimizationPass
{
  public:
    const string& getName() const override;
    bool run(ShaderGraph& graph, GenContext& context) override;

  private:
    bool optimizeMixBsdf(ShaderGraph& graph, ShaderNode* node, GenContext& context);
};

MATERIALX_NAMESPACE_END

#endif

