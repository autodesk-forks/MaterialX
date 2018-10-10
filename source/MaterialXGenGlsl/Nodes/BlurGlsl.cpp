#include <MaterialXGenGlsl/Nodes/BlurGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{
    BlurGlsl::BlurGlsl()
        : ParentClass()
        , _filterFunctionName("sx_blur_box_float")
        , _filterType("box")
        , _inputTypeString("float")
    {
        _filterSize = 2;
        _sampleCount = 9; // For box
        _sampleSizeFunctionUV.assign("sx_compute_sample_size_uv");
    }

    SgImplementationPtr BlurGlsl::create()
    {
        return std::shared_ptr<BlurGlsl>(new BlurGlsl());
    }

    void BlurGlsl::computeSampleOffsetStrings(const string& sampleSizeName, StringVec& offsetStrings)
    {
        // Build a 3x3 grid of samples that are offset by the provided sample size
        offsetStrings.clear();
        if (_filterType == "box")
        {
            for (int row = -1; row <= 1; row++)
            {
                for (int col = -1; col <= 1; col++)
                {
                    offsetStrings.push_back(" + " + sampleSizeName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
                }
            }
        }
        // TODO: gaussian
        else
        {
            for (int row = -1; row <= 1; row++)
            {
                for (int col = -1; col <= 1; col++)
                {
                    offsetStrings.push_back(" + " + sampleSizeName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
                }
            }
        }
    }

    bool BlurGlsl::acceptsInput(SgOutput& input)
    {
        // Float 1-4 is acceptable as input
        return (input.type == Type::FLOAT ||
                input.type->isScalar() || input.type->isFloat2() ||
                input.type->isFloat3() || input.type->isFloat4());
    }

    void BlurGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
    {
        const SgInput* inInput = node.getInput("in");
        _inputTypeString.clear();

        if (inInput->type == Type::FLOAT)
        {
            _inputTypeString = "float";
        }
        else if (inInput->type->isFloat2())
        {
            _inputTypeString = "vec2";
        }
        else if (inInput->type->isFloat3())
        {
            _inputTypeString = "vec3";
        }
        else if (inInput->type->isFloat4())
        {
            _inputTypeString = "vec4";
        }

        const SgInput* filterTypeInput = node.getInput("filtertype");
        if (!inInput || !filterTypeInput || _inputTypeString.empty())
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid Blur node");
        }

        // Check for type of filter to apply
        //
        if (filterTypeInput->value)
        {
            if (filterTypeInput->value->getValueString() == "gaussian")
            {
                _filterType = "gaussian";
                _sampleCount = 9;
            }
            else 
            {
                _filterType = "box";
                _sampleCount = 9;
            }
        }

        // Check size of filter. TO-DO.
        //const SgInput* sizeInput = node.getInput("size");     

        HwShader& shader = static_cast<HwShader&>(shader_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        {
            // Emit samples
            _sampleStrings.clear();
            emitInputSamples(node, context, shadergen, shader);

            string sampleName(node.getOutput()->name + "_samples");
            shader.addLine(_inputTypeString + " " + sampleName + "[" + std::to_string(_sampleCount) + "]");
            for (unsigned int i = 0; i < _sampleCount; i++)
            {
                shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + _sampleStrings[i]);
            }

            // Emit code to evaluate samples. Function to call depends on input type.
            //
            shader.beginLine();
            shadergen.emitOutput(context, node.getOutput(), true, false, shader);
            _filterFunctionName = "sx_blur_" + _filterType  + "_" + _inputTypeString;
            shader.addStr(" = " + _filterFunctionName);
            shader.addStr("(" + sampleName + ")");
            shader.endLine();
        }
        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
