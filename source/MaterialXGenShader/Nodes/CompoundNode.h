//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_COMPOUNDNODE_H
#define MATERIALX_COMPOUNDNODE_H

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

/// Compound node implementation
class MX_GENSHADER_API CompoundNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void addClassification(ShaderNode& node) const override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    ShaderGraph* getGraph() const override { return _rootGraph.get(); }

    /// Return the permutation key for this compound node.
    /// Includes topology-based optimization flags (e.g., "coat=0,sheen=x").
    string getPermutationKey() const override { return _permutationKey; }

  protected:
    ShaderGraphPtr _rootGraph;
    string _functionName;
    string _permutationKey;  ///< Topology-based permutation identifier
};

MATERIALX_NAMESPACE_END

#endif
