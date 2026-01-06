//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SHADERGRAPHOPTIMIZATIONPASS_H
#define MATERIALX_SHADERGRAPHOPTIMIZATIONPASS_H

/// @file
/// Shader graph optimization pass infrastructure

#include <MaterialXGenShader/Export.h>

#include <MaterialXCore/Document.h>

#include <memory>
#include <string>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

class ShaderGraph;
class GenContext;

/// @class ShaderGraphOptimizationPass
/// Base class for shader graph optimization passes.
/// Each pass encapsulates a single optimization transformation that can be
/// applied iteratively until the graph reaches a fixed point (no more changes).
class MX_GENSHADER_API ShaderGraphOptimizationPass
{
  public:
    virtual ~ShaderGraphOptimizationPass() = default;

    /// Return the name of this optimization pass
    virtual const string& getName() const = 0;

    /// Apply this optimization pass to the given shader graph.
    /// @param graph The shader graph to optimize
    /// @param context The generation context
    /// @return True if the graph was modified, false otherwise
    virtual bool run(ShaderGraph& graph, GenContext& context) = 0;
};

/// A shared pointer to an optimization pass
using ShaderGraphOptimizationPassPtr = std::shared_ptr<ShaderGraphOptimizationPass>;

/// @class ShaderGraphPassManager
/// Manages a sequence of optimization passes and runs them iteratively until convergence.
/// Similar to compiler optimization pass managers (LLVM, etc.)
class MX_GENSHADER_API ShaderGraphPassManager
{
  public:
    ShaderGraphPassManager() = default;

    /// Add an optimization pass to the pass manager
    void addPass(ShaderGraphOptimizationPassPtr pass);

    /// Run all enabled passes iteratively until no pass makes any changes (fixed point).
    /// @param graph The shader graph to optimize
    /// @param context The generation context
    /// @param maxIterations Maximum number of iterations to prevent infinite loops (default: 10)
    /// @return Total number of passes that made changes
    size_t runToFixedPoint(ShaderGraph& graph, GenContext& context, size_t maxIterations = 10);

    /// Get the list of registered passes
    const std::vector<ShaderGraphOptimizationPassPtr>& getPasses() const { return _passes; }

  private:
    std::vector<ShaderGraphOptimizationPassPtr> _passes;
};

MATERIALX_NAMESPACE_END

#endif

