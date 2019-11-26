//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/IfNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

const StringVec IfNode::INPUT_NAMES = { "value1", "value2", "in1", "in2" };

ShaderNodeImplPtr IfNode::create()
{
    return std::make_shared<IfNode>();
}

void IfNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderGraph& graph = *node.getParent();

        // Declare the output variable
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, true, context, stage);
        shadergen.emitLineEnd(stage);

        const ShaderInput* value1 = node.getInput(INPUT_NAMES[0]);
        const ShaderInput* value2 = node.getInput(INPUT_NAMES[1]);

        // Process the if and else branches of the conditional
        for (int branch = 2; branch <= 3; ++branch)
        {
            const ShaderInput* input = node.getInput(INPUT_NAMES[branch]);

            if (branch > 2)
            {
                shadergen.emitLine("else", stage, false);
            }
            else
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitString("if (", stage);
                shadergen.emitInput(value1, context, stage);
                shadergen.emitString(" <= ", stage); // TODO: This needs to be generalized to "<,<=, and =="
                shadergen.emitInput(value2, context, stage);
                shadergen.emitString(")", stage);
                shadergen.emitLineEnd(stage, false);
            }

            shadergen.emitScopeBegin(stage);

            // Emit function calls for nodes that are ONLY needed in this scope
            for (const ShaderNode* otherNode : graph.getNodes())
            {
                const ShaderNode::ScopeInfo& scope = otherNode->getScopeInfo();
                if (scope.conditionalNode == &node && scope.usedByBranch(branch))
                {
                    // Force ignore scope otherwise the function call will be omitted.
                    shadergen.emitFunctionCall(*otherNode, context, stage, false);
                }
            }

            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), false, false, context, stage);
            shadergen.emitString(" = ", stage);
            shadergen.emitInput(input, context, stage);
            shadergen.emitLineEnd(stage);

            shadergen.emitScopeEnd(stage);
        }
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
