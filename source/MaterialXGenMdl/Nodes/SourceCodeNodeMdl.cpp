//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

namespace MaterialX
{

ShaderNodeImplPtr SourceCodeNodeMdl::create()
{
    return std::make_shared<SourceCodeNodeMdl>();
}

void SourceCodeNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        if (_inlined)
        {
            SourceCodeNode::emitFunctionCall(node, context, stage);
        }
        else
        {
            // An ordinary source code function call
            shadergen.emitLineBegin(stage);

            // Declare the output variables. Only output 1 for now 
            // as multioutput requires building a struct
            if (node.numOutputs())
            {
                //shadergen.emitOutput(node.getOutput(0), true, true, context, stage);
                //shadergen.emitLineEnd(stage);

                // Emit node outputs. Only output 1 for now
                shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
                shadergen.emitString(" = ", stage);
            }

            // Emit function name.
            shadergen.emitString(_functionName + "(", stage);

            // Emit all inputs on the node.
            string delim = "";
            for (ShaderInput* input : node.getInputs())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitInput(input, context, stage);
                delim = ", ";
            }

            // End function call
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage);
        }
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
