#include <MaterialXGenGlsl/Nodes/HeightToNormalGlsl.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

    SgImplementationPtr HeightToNormalGlsl::create()
    {
        return std::make_shared<HeightToNormalGlsl>();
    }

    void HeightToNormalGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
    {
        // Screen size in pixels to be set by client.
        HwShader& shader = static_cast<HwShader&>(shader_);
        //const string OGSFX_SIZE_SEMANTIC("ViewportPixelSize"); -- This causes bad code to be emitted.
        shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS, Type::VECTOR2, "u_screenSize" /*, OGSFX_SIZE_SEMANTIC*/);
    }

    void HeightToNormalGlsl::emitInputSamples(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, HwShader& shader,
                                              const unsigned int sampleCount, StringVec& sampleStrings) const
    {
        const SgInput* inInput = node.getInput("in");
        string inputName = inInput->name;

        // Require an upstream node to sample
        SgNode* upstreamNode = nullptr;
        SgOutput* inConnection = inInput->connection;
        if (inConnection && inConnection->type->isScalar())
        {
            upstreamNode = inConnection->node;
            if (upstreamNode)
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
                            const float filterSize = 1.0;
                            const float filterOffset = 0.0;

                            // Emit code to compute sample size
                            //
                            string sampleInputValue;
                            shadergen.getInput(context, samplingInput, sampleInputValue);

                            const string sampleOutputName(node.getOutput()->name + "_sample_size");
                            string sampleCall("vec2 " + sampleOutputName + " = " +
                                "sx_compute_sample_size(" +
                                sampleInputValue + "," +
                                std::to_string(filterSize) + "," +
                                std::to_string(filterOffset) + ");"
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
                            for (unsigned int i = 0; i < sampleCount; i++)
                            {
                                // Computation of the center sample has already been
                                // output so just use that output variable
                                if (i == CENTER_SAMPLE)
                                {
                                    sampleStrings.push_back(outputName);
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
                                    sampleStrings.push_back(outputName + outputSuffix);
                                }
                            }
                        }
                        else
                        {
                            // Use the same input for all samples.
                            // Note: This does not recomputed the output, but instead it reuses
                            // the output variable.
                            for (unsigned int i = 0; i < sampleCount; i++)
                            {
                                // On failure just call the unmodified function
                                sampleStrings.push_back(outputName);
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
        if (sampleStrings.empty())
        {
            string inValueString = inInput->value->getValueString();
            for (unsigned int i = 0; i < sampleCount; i++)
            {
                sampleStrings.push_back(inValueString);
            }
        }
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
            // We assume the grid is 3 x 3 in size and is organized as follows
            // in array index order.
            // 
            // ----+-----+----
            //  0  |  1  | 2
            // ----+-----+----
            //  3  |  4  | 5
            // ----+-----+----
            //  6  |  7  | 8
            // ----+-----+----
            //
            const unsigned int sampleCount = 9;
            StringVec sampleStrings;

            emitInputSamples(node, context, shadergen, shader, sampleCount, sampleStrings);

            // Emit code to evaluate samples.
            //
            string scaleValueString = scaleInput->value ? scaleInput->value->getValueString() : "1.0";

            string sampleName(node.getOutput()->name + "_samples");
            shader.addLine("float " + sampleName + "[" + std::to_string(sampleCount) + "]");
            for (unsigned int i = 0; i < sampleCount; i++)
            {
                shader.addLine(sampleName + "[" + std::to_string(i) + "] = " + sampleStrings[i]);
            }
            shader.beginLine();
            shadergen.emitOutput(context, node.getOutput(), true, false, shader);
            shader.addStr(" = sx_normal_from_samples_sobel");
            shader.addStr("(" + sampleName + ", " + scaleValueString + ")");
            shader.endLine();
        }
        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
