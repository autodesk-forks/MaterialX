//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/HwImageNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr HwImageNode::create()
{
    return std::make_shared<HwImageNode>();
}

void HwImageNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    HwSourceCodeNode::emitFunctionCall(node, context, stage);
}

} // namespace MaterialX
