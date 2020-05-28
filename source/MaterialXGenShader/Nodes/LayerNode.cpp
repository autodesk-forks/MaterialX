//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/LayerNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

const string LayerNode::TOP_STRING = "top";
const string LayerNode::BASE_STRING = "base";

ShaderNodeImplPtr LayerNode::create()
{
    return std::make_shared<LayerNode>();
}

void LayerNode::initialize(const InterfaceElement& element, GenContext& context)
{
}

void LayerNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* top = node.getInput(TOP_STRING);
        const ShaderInput* base = node.getInput(BASE_STRING);
        const ShaderOutput* out = node.getOutput();
        if (!(top && base && out))
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid layer node");
        }
        if (!top->getConnection())
        {
            throw ExceptionShaderGenError("No top BSDF is connected on layer node '" + node.getName() + "'");
        }
        const ShaderNode* topBsdf = top->getConnection()->getNode();
        const ShaderInput* topBsdfBase = topBsdf->getInput(BASE_STRING);
        if (!topBsdfBase)
        {
            throw ExceptionShaderGenError("Node connected as top layer '" + topBsdf->getName() + "' is not a layerable BSDF");
        }

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
