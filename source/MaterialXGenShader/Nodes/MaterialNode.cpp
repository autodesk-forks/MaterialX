//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/Nodes/MaterialNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGraph.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr MaterialNode::create()
{
    return std::make_shared<MaterialNode>();
}

void MaterialNode::addClassification(ShaderNode& node) const
{
    const ShaderInput* surfaceshaderInput = node.getInput(ShaderNode::SURFACESHADER);
    if (surfaceshaderInput && surfaceshaderInput->getConnection())
    {
        // This is a material node with a surfaceshader connected.
        // Add the classification from this shader.
        const ShaderNode* surfaceshaderNode = surfaceshaderInput->getConnection()->getNode();
        node.addClassification(surfaceshaderNode->getClassification());
    }
}

void MaterialNode::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        ShaderNode& node = const_cast<ShaderNode&>(_node);
        ShaderInput* surfaceshaderInput = node.getInput(ShaderNode::SURFACESHADER);

        if (!surfaceshaderInput->getConnection())
        {
            // Just declare the output variable with default value.
            emitOutputVariables(node, context, stage);
            return;
        }

        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        // Emit the function call for upstream surface shader.
        const ShaderNode* surfaceshaderNode = surfaceshaderInput->getConnection()->getNode();
        shadergen.emitFunctionCall(*surfaceshaderNode, context, stage);

        // Assign this result to the material output variable.
        // Generate a generic variable name instead of using the node name.
        ShaderOutput* output = const_cast<ShaderOutput*>(node.getOutput());
        const ShaderGraph* parentGraph = node.getParent();
        string materialVariableName;
        if (parentGraph)
        {
            // Cast away const to access identifier map (which needs to be modified for uniqueness)
            ShaderGraph* nonConstParentGraph = const_cast<ShaderGraph*>(parentGraph);
            IdentifierMap& identifierMap = nonConstParentGraph->getIdentifierMap();
            // Use "material" as base name with output port suffix to match naming convention
            string baseName = "material_" + output->getName();
            materialVariableName = syntax.getVariableName(baseName, output->getType(), identifierMap);
            // Update the output's variable name so references use the new name
            output->setVariable(materialVariableName);
        }
        else
        {
            // Fallback to original behavior if no parent graph
            materialVariableName = output->getVariable();
        }
        shadergen.emitLine(syntax.getTypeName(output->getType()) + " " + materialVariableName + " = " + surfaceshaderInput->getConnection()->getVariable(), stage);
    }
}

MATERIALX_NAMESPACE_END
