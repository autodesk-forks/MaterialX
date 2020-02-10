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
            throw ExceptionShaderGenError("Multiple outputs is not supported yet by this generaor");
        }

        const ShaderGraphOutputSocket* outputSocket = _rootGraph->getOutputSocket();
        const string& outputType = syntax.getTypeName(outputSocket->getType());

        // Begin function signature.
        shadergen.emitLineBegin(stage);
        shadergen.emitString(outputType + " " + _functionName + + "(", stage);

        string delim = "";

        // Add all inputs
        for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
        {
            shadergen.emitString(delim + syntax.getTypeName(inputSocket->getType()) + " " + inputSocket->getVariable(), stage);
            delim = ", ";
        }

        // End function signature.
        shadergen.emitString(")", stage);
        shadergen.emitLineEnd(stage, false);

        // Begin function body.
        shadergen.emitScopeBegin(stage);
        shadergen.emitFunctionCalls(*_rootGraph, context, stage);

        // Emit final results
        const string result = shadergen.getUpstreamResult(outputSocket, context);
        shadergen.emitLine("return " + result, stage);

        // End function body.
        shadergen.emitScopeEnd(stage);
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
