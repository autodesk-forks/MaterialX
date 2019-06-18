//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/Nodes/NormalNodeGlsl.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

ShaderNodeImplPtr NormalNodeGlsl::create()
{
    return std::make_shared<NormalNodeGlsl>();
}

void NormalNodeGlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::IN_NORMAL, vs);

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;
    if (space == WORLD_SPACE)
    {
        addStageUniform(HW::PRIVATE_UNIFORMS, Type::MATRIX44, HW::WORLD_INVERSE_TRANSPOSE_MATRIX, vs);
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::NORMAL_WORLD, vs, ps);
    }
    else
    {
        addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::NORMAL_OBJECT, vs, ps);
    }
}

void NormalNodeGlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();

    const ShaderInput* spaceInput = node.getInput(SPACE);
    const int space = spaceInput ? spaceInput->getValue()->asA<int>() : OBJECT_SPACE;

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        if (space == WORLD_SPACE)
        {
            ShaderPort* normal = vertexData[HW::NORMAL_WORLD];
            if (!normal->isEmitted())
            {
                normal->setEmitted();
                shadergen.emitLine(prefix + normal->getVariable() + " = (" + HW::WORLD_INVERSE_TRANSPOSE_MATRIX + " * vec4(" + HW::IN_NORMAL + ",0.0)).xyz", stage);
            }
        }
        else
        {
            ShaderPort* normal = vertexData[HW::NORMAL_OBJECT];
            if (!normal->isEmitted())
            {
                normal->setEmitted();
                shadergen.emitLine(prefix + normal->getVariable() + " = " + HW::IN_NORMAL, stage);
            }
        }
    END_SHADER_STAGE(shader, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = vertexData.getInstance() + ".";
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        if (space == WORLD_SPACE)
        {
            const ShaderPort* normal = vertexData[HW::NORMAL_WORLD];
            shadergen.emitString(" = normalize(" + prefix + normal->getVariable() + ")", stage);
        }
        else
        {
            const ShaderPort* normal = vertexData[HW::NORMAL_OBJECT];
            shadergen.emitString(" = normalize(" + prefix + normal->getVariable() + ")", stage);
        }
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

} // namespace MaterialX
