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
        // TODO: Figure out how to set and use screen spaced sizes
        // which would be used for 2D filtering
        HwShader& shader = static_cast<HwShader&>(shader_);
        shader.createAppData(Type::FLOAT, "i_screenWidth");
        shader.createAppData(Type::FLOAT, "i_screenHeight");
    }

    void HeightFieldToNormalGlsl::emitFunctionDefinition(const SgNode& /*node*/, ShaderGenerator& shadergen_, Shader& shader_)
    {
        HwShader& shader = static_cast<HwShader&>(shader_);
        GlslShaderGenerator shadergen = static_cast<GlslShaderGenerator&>(shadergen_);

        BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

            // Emit function signature. Need to add filter width and filter height
            shader.addLine("vec3 IM_heighttonormal_vector3_sx_glsl(float _in[9], float _scale, float _width, float _height)"
                , false);
        shader.beginScope();

        vector<string> kernalStrings;
        kernalStrings.push_back("K[0] = ");
        kernalStrings.push_back("K[1] = ");
        kernalStrings.push_back("K[2] = ");

        kernalStrings.push_back("K[3] = ");
        kernalStrings.push_back("K[4] = ");
        kernalStrings.push_back("K[5] = ");

        kernalStrings.push_back("K[6] = ");
        kernalStrings.push_back("K[7] = ");
        kernalStrings.push_back("K[8] = ");

        string inValueString = "_in";
        for (unsigned int i = 0; i < kernalStrings.size(); i++)
        {
            kernalStrings[i].append(inValueString + "[" + std::to_string(i) + "];");
        }

        shader.addLine("float K[9];");
        for (auto kernalString : kernalStrings)
        {
            shader.addLine(kernalString);
        }

        // Sobel filter computation. Can change this
        std::vector<std::string> computeFilter;
        computeFilter.push_back("float nx = K[2] + (2.0*K[5]) + K[8] - (K[0] + (2.0*K[3]) + K[6]);");
        computeFilter.push_back("float ny = K[0] + (2.0*K[1]) + K[2] - (K[6] + (2.0*K[7]) + K[8]);");
        computeFilter.push_back("float nz = _scale * sqrt(1.0 - dx*dx - dy*dy);");
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
        string upstreamNode;
        if (inInput->connection)
        {
            upstreamNode = inInput->connection->name;
        }
        else
        {
            if (!inInput->value)
            {
                throw ExceptionShaderGenError("No connection or value found on heighttonormal node '" + node.getName() + "'");
            }
        }

        // Build kernal inputs strings
        // TODO: Handle connections on input
        string inValueString = inInput->value->getValueString();
        for (unsigned int i = 0; i < kernalSize; i++)
        {
            kernalStrings.push_back(inValueString);
        }

        // Dump out kernal setting code
        shader.beginLine();

        // TODO: Determine how to specify these values
        string filterWidth("0.1");
        string filterHeight("0.1");

        string kernalName("_kernal" + context.getFunctionSuffix()); // Does this make it unique?
        shader.addLine("float " + kernalName + "[9]"); 
        for (unsigned int i=0; i<9; i++)
        {
            shader.addLine(kernalName + "[" + kernalStrings[i] + "]");
        }
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.addStr(" = IM_heighttonormal_vector3_sx_glsl");
        shader.addStr("(" + kernalName + ", " + scaleValueString + ", " + filterWidth + ", " + filterHeight + ")");
        shader.endLine();

        END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
    }

} // namespace MaterialX
