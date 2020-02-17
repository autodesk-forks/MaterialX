//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

namespace MaterialX
{

ShaderNodeImplPtr CompoundNodeMdl::create()
{
    return std::make_shared<CompoundNodeMdl>();
}

void CompoundNodeMdl::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        // Emit functions for all child nodes
        shadergen.emitFunctionDefinitions(*_rootGraph, context, stage);

        if (_rootGraph->getOutputSockets().size() > 1)
        {
            throw ExceptionShaderGenError("Multiple outputs is not supported yet by this generator");
        }

        const bool isMaterialExpr = (
            _rootGraph->hasClassification(ShaderNode::Classification::CLOSURE) ||
            _rootGraph->hasClassification(ShaderNode::Classification::SHADER)
        );

        const ShaderGraphOutputSocket* outputSocket = _rootGraph->getOutputSocket();
        const string& outputType = syntax.getTypeName(outputSocket->getType());

        // Begin function signature.
        shadergen.emitLine(outputType + " " + _functionName, stage, false);

        shadergen.emitScopeBegin(stage, Syntax::PARENTHESES);

        const string uniformPrefix = syntax.getUniformQualifier() + " ";

        // Emit all inputs
        int count = int(_rootGraph->numInputSockets());
        for (ShaderGraphInputSocket* input : _rootGraph->getInputSockets())
        {
            const string& qualifier = input->isUniform() ? uniformPrefix : EMPTY_STRING;
            const string& type = syntax.getTypeName(input->getType());
            const string value = (input->getValue() ?
                syntax.getValue(input->getType(), *input->getValue()) :
                syntax.getDefaultValue(input->getType()));

            const string& delim = --count > 0 ? Syntax::COMMA : EMPTY_STRING;
            shadergen.emitLine(qualifier + type + " " + input->getVariable() + " = " + value + delim, stage, false);
        }

        // End function signature.
        shadergen.emitScopeEnd(stage);

        // Special case for material expresions.
        if (isMaterialExpr)
        {
            shadergen.emitLine(" = let", stage, false);
        }

        // Function body.
        shadergen.emitScopeBegin(stage);
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);

        // Emit final results
        const string result = shadergen.getUpstreamResult(outputSocket, context);
        if (isMaterialExpr)
        {
            shadergen.emitScopeEnd(stage);
            shadergen.emitLine("in material(" + result + ")", stage);
        }
        else
        {
            shadergen.emitLine("return " + result, stage);
            shadergen.emitScopeEnd(stage);
        }

        shadergen.emitLineBreak(stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

void CompoundNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        if (node.getOutputs().size() > 1)
        {
            throw ExceptionShaderGenError("Multiple outputs is not supported yet by this generaor");
        }

        const ShaderOutput* output = node.getOutput();

        // Declare the output variable
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(output, true, false, context, stage);

        // Begin function call.
        shadergen.emitString(" = " + _functionName + "(", stage);

        string delim = "";

        // Emit inputs.
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(delim, stage);
            shadergen.emitInput(input, context, stage);
            delim = ", ";
        }

        // End function call
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
