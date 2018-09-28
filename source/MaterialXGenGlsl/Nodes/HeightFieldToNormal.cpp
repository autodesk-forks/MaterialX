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
        computeFilter.push_back("float nx = K[2] + (2.0*K[5]) + K[8] - (K[0] + (2.0*K[3]) + K[6]);");
        computeFilter.push_back("float ny = K[0] + (2.0*K[1]) + K[2] - (K[6] + (2.0*K[7]) + K[8]);");
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

    void HeightFieldToNormalGlsl::emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
    {
        HwShader& shader = static_cast<HwShader&>(shader_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        const SgInput* inInput = node.getInput("in");
        const SgInput* scaleInput = node.getInput("scale");

        if (!inInput || !scaleInput)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid heighttonormal node");
        }

        //const Syntax* syntax = shadergen.getSyntax();
        string scaleValueString = scaleInput->value ? scaleInput->value->getValueString() : "1.0";

        string inputName = inInput->name;

        const unsigned int kernalSize(9);
        std::vector<std::string> kernalStrings;

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
                        string oldName = upstreamOutput->name;
                        string upStreamOutputNameBase = upstreamOutput->name + "_" + node.getOutput()->name;

                        // Emit outputs for kernal input 
                        for (unsigned int i = 0; i < kernalSize; i++)
                        {
                            string upStreamOutputName = upStreamOutputNameBase + std::to_string(i);
                            upstreamOutput->name = upStreamOutputName;
                            impl->emitFunctionCall(*upstreamNode, context, shadergen, shader);
                            
                            kernalStrings.push_back(upStreamOutputName);
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
        // TODO: Determine how to specify these values
        string filterWidth("0.1");
        string filterHeight("0.1");

        string kernalName(node.getOutput()->name + "_kernal"); 
        shader.addLine("float " + kernalName + "[9]"); 
        for (unsigned int i=0; i<9; i++)
        {
            shader.addLine(kernalName + "[" + std::to_string(i) + "] = " + kernalStrings[i]);
        }
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = IM_heighttonormal_vector3_sx_glsl");
        shader.addStr("(" + kernalName + ", " + scaleValueString + ", " + filterWidth + ", " + filterHeight + ")");
        shader.endLine();

        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
