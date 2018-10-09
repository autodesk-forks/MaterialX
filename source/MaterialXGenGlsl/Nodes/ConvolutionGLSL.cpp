#include <MaterialXGenGlsl/Nodes/ConvolutionGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{
    ConvolutionGlsl::ConvolutionGlsl()
        : _sampleCount(9)
        , _filterSize(1.0f)
        , _filterOffset(0.0f)
        , _sampleSizeFunctionUV("sx_compute_sample_size_uv")
    {
    }

    void ConvolutionGlsl::emitInputSamples(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, HwShader& shader)
    {
        const SgInput* inInput = node.getInput("in");
        string inputName = inInput->name;

        // Require an upstream node to sample
        SgNode* upstreamNode = nullptr;
        SgOutput* inConnection = inInput->connection;
        if (inConnection && inConnection->type->isScalar())
        {
            upstreamNode = inConnection->node;
            if (upstreamNode && upstreamNode->hasClassification(SgNode::Classification::SAMPLE2D))
            {
                SgImplementation *impl = upstreamNode->getImplementation();
                if (impl)
                {
                    SgOutput* upstreamOutput = upstreamNode->getOutput();
                    if (upstreamOutput)
                    {
                        string outputName = upstreamOutput->name;

                        // Find out which input needs to be sampled multiple times
                        SgInput* samplingInput = upstreamNode->getSamplingInput();
                        // TODO: For now we only support uv space sampling
                        if (samplingInput && samplingInput->type != Type::VECTOR2)
                        {
                            samplingInput = nullptr;
                        }

                        if (samplingInput)
                        {
                            // This is not exposed. Assume a filter size of 1 with no offset

                            // Emit code to compute sample size
                            //
                            string sampleInputValue;
                            shadergen.getInput(context, samplingInput, sampleInputValue);

                            const string sampleOutputName(node.getOutput()->name + "_sample_size");
                            string sampleCall("vec2 " + sampleOutputName + " = " +
                                _sampleSizeFunctionUV + "(" +
                                sampleInputValue + "," +
                                std::to_string(_filterSize) + "," +
                                std::to_string(_filterOffset) + ");"
                            );
                            shader.addLine(sampleCall);

                            // Build the sample offset strings
                            //
                            vector<string> inputVec2Suffix;
                            for (int row = -1; row <= 1; row++)
                            {
                                for (int col = -1; col <= 1; col++)
                                {
                                    inputVec2Suffix.push_back(" + " + sampleOutputName + " * vec2(" + std::to_string(float(col)) + "," + std::to_string(float(row)) + ")");
                                }
                            }

                            // Emit outputs for sample input 
                            const unsigned int CENTER_SAMPLE(4);
                            for (unsigned int i = 0; i < _sampleCount; i++)
                            {
                                // Computation of the center sample has already been
                                // output so just use that output variable
                                if (i == CENTER_SAMPLE)
                                {
                                    _sampleStrings.push_back(outputName);
                                }
                                else
                                {
                                    // Add an input name suffix. 
                                    context.addInputSuffix(samplingInput, inputVec2Suffix[i]);

                                    // Add a output name suffix for the emit call
                                    string outputSuffix("_" + node.getOutput()->name + std::to_string(i));
                                    context.addOutputSuffix(upstreamOutput, outputSuffix);

                                    impl->emitFunctionCall(*upstreamNode, context, shadergen, shader);

                                    // Remove suffixes
                                    context.removeInputSuffix(samplingInput);
                                    context.removeOutputSuffix(upstreamOutput);

                                    // Keep track of the output name with the suffix
                                    _sampleStrings.push_back(outputName + outputSuffix);
                                }
                            }
                        }
                        else
                        {
                            // Use the same input for all samples.
                            // Note: This does not recomputed the output, but instead it reuses
                            // the output variable.
                            for (unsigned int i = 0; i < _sampleCount; i++)
                            {
                                // On failure just call the unmodified function
                                _sampleStrings.push_back(outputName);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (!inInput->value)
            {
                throw ExceptionShaderGenError("No connection or value found on heighttonormal node '" + node.getName() + "'");
            }
        }

        // Build a set of samples with constant values
        if (_sampleStrings.empty())
        {
            string inValueString = inInput->value->getValueString();
            for (unsigned int i = 0; i < _sampleCount; i++)
            {
                _sampleStrings.push_back(inValueString);
            }
        }
    }
    

} // namespace MaterialX
