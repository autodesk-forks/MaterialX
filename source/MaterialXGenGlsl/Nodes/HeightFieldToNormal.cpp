#include <MaterialXGenGlsl/Nodes/HeightFieldToNormal.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

    SgImplementationPtr HeightFieldToNormalGlsl::create()
    {
        return std::make_shared<HeightFieldToNormalGlsl>();
    }

    void HeightFieldToNormalGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
    {
        // Screen size in pixels to be set by client.
        HwShader& shader = static_cast<HwShader&>(shader_);
        const string OGSFX_SIZE_SEMANTIC("ViewportPixelSize");
        shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS, Type::VECTOR2, "u_screenSize", OGSFX_SIZE_SEMANTIC);
    }

    void HeightFieldToNormalGlsl::emitFunctionDefinition(const SgNode& /*node*/, ShaderGenerator& shadergen_, Shader& shader_)
    {
        HwShader& shader = static_cast<HwShader&>(shader_);
        GlslShaderGenerator shadergen = static_cast<GlslShaderGenerator&>(shadergen_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        // Emit function signature. Need to add filter width and filter height
        shader.addLine("vec3 IM_heighttonormal_vector3_sx_glsl(float K[9], float _scale, float _width, float _height)", false);
        shader.beginScope();

        // Sobel filter computation. Can change this
        std::vector<std::string> computeFilter;
        computeFilter.push_back("float nx = K[0] - K[2] + (2.0*K[3]) - (2.0*K[5]) + K[6] - K[8];");
        computeFilter.push_back("float ny = K[0] + (2.0*K[1]) + K[2] - K[6] - (2.0*K[7]) - K[8];");
        computeFilter.push_back("float nz = _scale * sqrt(1.0 - nx*nx - ny*ny);");
        computeFilter.push_back("return vec3(nx, ny, nz);");
        for (auto f : computeFilter)
        {
            shader.addLine(f);
        }

        shader.endScope();
        shader.newLine();

        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

    void HeightFieldToNormalGlsl::emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
    {
        HwShader& shader = static_cast<HwShader&>(shader_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        const SgInput* inInput = node.getInput("in");
        const SgInput* scaleInput = node.getInput("scale");

        if (!inInput || !scaleInput)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid heighttonormal node");
        }

        string scaleValueString = scaleInput->value ? scaleInput->value->getValueString() : "1.0";

        string inputName = inInput->name;

        const unsigned int kernalSize(9);
        std::vector<std::string> kernalStrings;

        // TODO: Need to compute the filter values proper.
        // For 2d sample can get take <kernal size> / textureSize(sampler) 
        float filterDelta(0.0009765625f);
        string filterWidth(std::to_string(filterDelta));
        string filterHeight(std::to_string(filterDelta));
        vector<string> inputVec2Suffix;
        for (int row = -1; row <= 1; row++)
        {
            for (int col = -1; col <= 1; col++)
            {
                inputVec2Suffix.push_back(" + vec2(" + std::to_string(filterDelta*float(col)) + "," + std::to_string(filterDelta*float(row)) + ")");
            }
        }

        // Require an upstream node to sample
        string upstreamNodeName;
        SgNode* upstreamNode = nullptr;
        SgOutput* inConnection = inInput->connection;
        if (inConnection && inConnection->type->isScalar())
        {
            upstreamNode = inConnection->node;
            upstreamNodeName = inConnection->name;

            if (upstreamNode)
            {
                SgImplementation *impl = upstreamNode->getImplementation();
                if (impl)
                {
                    SgOutput* upstreamOutput = upstreamNode->getOutput();
                    if (upstreamOutput)
                    {
                        string outputName = upstreamOutput->name;

                        // Emit outputs for kernal input 
                        const unsigned int CENTER_SAMPLE(4);
                        for (unsigned int i = 0; i < kernalSize; i++)
                        {
                            // Computation of the center sample has already been
                            // output so just use that output variable
                            if (i == CENTER_SAMPLE)
                            {
                                kernalStrings.push_back(outputName);
                            }
                            else
                            {
                                // Add an input name suffix. Only for 2d texcoord inputs
                                // for now.
                                std::vector<SgInput*> sampleInputs;
                                for (SgInput* input : upstreamNode->getInputs())
                                {
                                    if (input->name == "texcoord")
                                    {
                                        sampleInputs.push_back(input);
                                    }
                                }

                                if (!sampleInputs.empty())
                                {
                                    // Add input suffixes for the emit call
                                    for (auto sampleInput : sampleInputs)
                                    {
                                        context.addInputSuffix(sampleInput, inputVec2Suffix[i]);
                                    }

                                    // Add a output name suffix for the emit call
                                    string outputSuffix("_" + node.getOutput()->name + std::to_string(i));
                                    context.addOutputSuffix(upstreamOutput, outputSuffix);
                                    
                                    impl->emitFunctionCall(*upstreamNode, context, shadergen, shader);
                                    
                                    // Remove suffixes
                                    for (auto sampleInput : sampleInputs)
                                    {
                                        context.removeInputSuffix(sampleInput);
                                    }
                                    context.removeOutputSuffix(upstreamOutput);

                                    // Keep track of the output name with the suffix
                                    kernalStrings.push_back(outputName + outputSuffix);
                                }
                                else
                                {
                                    // On failure just call the unmodified function
                                    kernalStrings.push_back(outputName);
                                }
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

        // Build kernal using constant value
        if (kernalStrings.empty())
        {
            string inValueString = inInput->value->getValueString();
            for (unsigned int i = 0; i < kernalSize; i++)
            {
                kernalStrings.push_back(inValueString);
            }
        }

        // Dump out kernal setting code
        //

        string kernalName(node.getOutput()->name + "_kernal"); 
        shader.addLine("float " + kernalName + "[9]"); 
        for (unsigned int i=0; i<9; i++)
        {
            shader.addLine(kernalName + "[" + std::to_string(i) + "] = " + kernalStrings[i]);
        }
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = IM_heighttonormal_vector3_sx_glsl");
        shader.addStr("(" + kernalName + ", " + scaleValueString + ", " + filterWidth + ", " + filterHeight + ")");
        shader.endLine();

        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
