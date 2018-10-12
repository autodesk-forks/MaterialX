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
    offsetStrings.clear();
 
    // Build a NxN grid of samples that are offset by the provided sample size
    int offset = ((int)(sqrt(_sampleCount)) - 1) / 2;

    for (int row = -offset; row <= offset; row++)
    {
        for (int col = -offset; col <= offset; col++)
        {
            offsetStrings.push_back(" + " + sampleSizeName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
        }
    }
}

bool BlurGlsl::acceptsInputType(const TypeDesc* type)
{
    // Float 1-4 is acceptable as input
    return ((type == Type::FLOAT && type->isScalar()) ||
            type->isFloat2() || type->isFloat3() || type->isFloat4());
}

void BlurGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
{
    const SgInput* inInput = node.getInput("in");

    // Get intput type name string
    _inputTypeString.clear();
    if (acceptsInputType(inInput->type))
    {
        _inputTypeString = shadergen.getSyntax()->getTypeName(inInput->type);
    }

    const SgInput* filterTypeInput = node.getInput("filtertype");
    if (!inInput || !filterTypeInput || _inputTypeString.empty())
    {
        throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid Blur node");
    }

    // Check size of filter.
    const SgInput* sizeInput = node.getInput("size");     
    unsigned int filterSize = 3;
    if (sizeInput)
    {
        float sizeInputValue = sizeInput->value->asA<float>();
        if (sizeInputValue <= 0.25f)
        {
            filterSize = 3;
        }
        else if (sizeInputValue <= 0.55f)
        {
            filterSize = 5;
        }
        else
        {
            filterSize = 7;
        }
    }

    // Check for type of filter to apply
    // Default to box filter
    //
    const string BOX_FILTER("box");
    const string GAUSSIAN_FILTER("gaussian");
    _filterType = BOX_FILTER;
    string weightFunction = "sx_get_box_weights";
    if (filterTypeInput->value)
    {
        // Use Gaussian filter.
        if (filterTypeInput->value->getValueString() == GAUSSIAN_FILTER)
        {
            _filterType = GAUSSIAN_FILTER;
            weightFunction = "sx_get_gaussian_weights";
        }
    }

    // Sample count is quare of filter size
    _sampleCount = filterSize*filterSize;

    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    {
        // Emit samples
        // Note: The maximum sample count SX_MAX_SAMPLE_COUNT is defined in the shader code and 
        // is assumed to be 49 (7x7 kernel). If this changes that the filter size logic here 
        // needs to be adjusted.
        //
        StringVec sampleStrings;
        emitInputSamplesUV(node, context, shadergen, shader, sampleStrings);

        // Set up sample array
        string sampleName(node.getOutput()->name + "_samples");
        shader.addLine(_inputTypeString + " " + sampleName + "[SX_MAX_SAMPLE_COUNT]");
        for (unsigned int i = 0; i < _sampleCount; i++)
        {
            shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
        }

        // Set up weight array
        string weightName(node.getOutput()->name + "_weights");
        shader.addLine(_inputTypeString + " " + weightName + "[SX_MAX_SAMPLE_COUNT]");
        shader.addLine(weightFunction + "(" + weightName + ", " + std::to_string(filterSize) + ")");

        // Emit code to evaluate using input sample and weight arrays. 
        // The function to call depends on input type.
        //
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        _filterFunctionName = "sx_convolution_" + _filterType  + "_" + _inputTypeString;
        shader.addStr(" = " + _filterFunctionName);
        shader.addStr("(" + sampleName + ")");
        shader.endLine();
    }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
