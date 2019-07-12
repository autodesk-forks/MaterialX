//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/ImageNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

ShaderNodeImplPtr ImageNode::create()
{
    return std::make_shared<ImageNode>();
}

void ImageNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add additional scale and offset inputs
    ShaderInput* input = node.addInput("uv_scale", Type::VECTOR2);
    input->setValue(Value::createValue<Vector2>(Vector2(1.0f, 1.0f)));
    input = node.addInput("uv_offset", Type::VECTOR2);
    input->setValue(Value::createValue<Vector2>(Vector2(0.0f, 0.0f)));
}

void ImageNode::setValues(const Node& /*node*/, ShaderNode& /*shaderNode*/, GenContext& /*context*/) const
{
    // TODO: Share code with HWImageNode.
}

void ImageNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    SourceCodeNode::emitFunctionCall(node, context, stage);
}

} // namespace MaterialX
