//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_COMPOUNDNODEMDL_H
#define MATERIALX_COMPOUNDNODEMDL_H

#include <MaterialXGenShader/Nodes/CompoundNode.h>

namespace MaterialX
{

/// Compound node implementation
class CompoundNodeMdl : public CompoundNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
