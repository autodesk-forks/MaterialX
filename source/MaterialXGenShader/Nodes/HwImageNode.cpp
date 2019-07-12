//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/HwSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/HwImageNode.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

ShaderNodeImplPtr HwImageNode::create()
{
    return std::make_shared<HwImageNode>();
}

void HwImageNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add additional scale and offset inputs
    ShaderInput* input = node.addInput("uv_scale", Type::VECTOR2);
    input->setValue(Value::createValue<Vector2>(Vector2(1.0f, 1.0f)));
    input = node.addInput("uv_offset", Type::VECTOR2);
    input->setValue(Value::createValue<Vector2>(Vector2(0.0f, 0.0f)));
}

void HwImageNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    HwSourceCodeNode::emitFunctionCall(node, context, stage);
}

} // namespace MaterialX
