#include <MaterialXGenGlsl/Nodes/HeightToNormalGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{
    HeightToNormalGlsl::HeightToNormalGlsl()
        : ParentClass()
        , _filterFunctionName("sx_normal_from_samples_sobel")
    {
    }

    SgImplementationPtr HeightToNormalGlsl::create()
    {
        return std::shared_ptr<HeightToNormalGlsl>(new HeightToNormalGlsl());
    }

    void HeightToNormalGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
    {
        const SgInput* inInput = node.getInput("in");
        const SgInput* scaleInput = node.getInput("scale");

        if (!inInput || !scaleInput)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid heighttonormal node");
        }

        HwShader& shader = static_cast<HwShader&>(shader_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        {
            // Create the input "samples". This means to emit the calls to 
            // compute the sames and return a set of strings containaing
            // the variables to assign to the sample grid.
            //  
            _sampleStrings.clear();
            emitInputSamples(node, context, shadergen, shader);

            // Emit code to evaluate samples.
            //
            string scaleValueString = scaleInput->value ? scaleInput->value->getValueString() : "1.0";

            string sampleName(node.getOutput()->name + "_samples");
            shader.addLine("float " + sampleName + "[" + std::to_string(_sampleCount) + "]");
            for (unsigned int i = 0; i < _sampleCount; i++)
            {
                shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + _sampleStrings[i]);
            }
            shader.beginLine();
            shadergen.emitOutput(context, node.getOutput(), true, false, shader);
            shader.addStr(" = " + _filterFunctionName);
            shader.addStr("(" + sampleName + ", " + scaleValueString + ")");
            shader.endLine();
        }
        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
