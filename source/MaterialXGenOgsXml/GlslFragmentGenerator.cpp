//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

namespace Stage
{
    const string PRIVATE_UNIFORMS = "private_uniforms";
}

string GlslFragmentSyntax::getVariableName(const string& name, const TypeDesc* type, IdentifierMap& identifiers) const
{
    string variable = GlslSyntax::getVariableName(name, type, identifiers);
    // A filename input corresponds to a texture sampler uniform
    // which requires a special suffix in OGS XML fragments.
    if (type == Type::FILENAME)
    {
        // Make sure it's not already used.
        if (variable.size() <= OgsXmlGenerator::SAMPLER_SUFFIX.size() || 
            variable.substr(variable.size() - OgsXmlGenerator::SAMPLER_SUFFIX.size()) != OgsXmlGenerator::SAMPLER_SUFFIX)
        {
            variable += OgsXmlGenerator::SAMPLER_SUFFIX;
        }
    }
    return variable;
}

const string GlslFragmentGenerator::TARGET = "ogsxml";
const string GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX = "4";

GlslFragmentGenerator::GlslFragmentGenerator() :
    GlslShaderGenerator()
{
    // Use our custom syntax class
    _syntax = std::make_shared<GlslFragmentSyntax>();

    // Set identifier names to match OGS naming convention.
    _tokenSubstitutions[HW::T_POSITION_WORLD]       = "Pw";
    _tokenSubstitutions[HW::T_POSITION_OBJECT]      = "Pm";
    _tokenSubstitutions[HW::T_NORMAL_WORLD]         = "Nw";
    _tokenSubstitutions[HW::T_NORMAL_OBJECT]        = "Nm";
    _tokenSubstitutions[HW::T_TANGENT_WORLD]        = "Tw";
    _tokenSubstitutions[HW::T_TANGENT_OBJECT]       = "Tm";
    _tokenSubstitutions[HW::T_BITANGENT_WORLD]      = "Bw";
    _tokenSubstitutions[HW::T_BITANGENT_OBJECT]     = "Bm";
    _tokenSubstitutions[HW::T_VERTEX_DATA_INSTANCE] = "PIX_IN";
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE]       = "u_envIrradianceSampler";
    _tokenSubstitutions[HW::T_ENV_RADIANCE]         = "u_envRadianceSampler";
}

ShaderGeneratorPtr GlslFragmentGenerator::create()
{
    return std::make_shared<GlslFragmentGenerator>();
}

ShaderPtr GlslFragmentGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = GlslShaderGenerator::createShader(name, element, context);
    createStage(Stage::PRIVATE_UNIFORMS, *shader);
    return shader;
}

ShaderPtr GlslFragmentGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    ShaderStage& pixelStage = shader->getStage(Stage::PIXEL);
    ShaderGraph& graph = shader->getGraph();

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    ScopedFloatFormatting floatFormatting(Value::FloatFormatFixed);

    // Add global constants and type definitions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_defines.glsl", context, pixelStage);
    const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
    emitLine("#define MAX_LIGHT_SOURCES " + std::to_string(maxLights), pixelStage, false);
    emitLineBreak(pixelStage);
    emitTypeDefinitions(context, pixelStage);

    // Add all constants
    const VariableBlock& constants = pixelStage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), SEMICOLON, context, pixelStage);
        emitLineBreak(pixelStage);
    }

    bool lighting = graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE) ||
                    graph.hasClassification(ShaderNode::Classification::BSDF);

    // Emit lighting functions
    if (lighting)
    {
        const VariableBlock& lightData = pixelStage.getUniformBlock(HW::LIGHT_DATA);
        emitLine("struct " + lightData.getName(), pixelStage, false);
        emitScopeBegin(pixelStage);
        emitVariableDeclarations(lightData, EMPTY_STRING, SEMICOLON, context, pixelStage, false);
        emitScopeEnd(pixelStage, true);
        emitLineBreak(pixelStage);
        emitLine("uniform " + lightData.getName() + " " + lightData.getInstance() + "[MAX_LIGHT_SOURCES]", pixelStage);
        emitLineBreak(pixelStage);
    }

    // Emit common math functions
    emitInclude("pbrlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_math.glsl", context, pixelStage);
    emitLineBreak(pixelStage);

    if (lighting)
    {
        emitSpecularEnvironment(context, pixelStage);
    }

    // Emit sampling code if needed
    if (graph.hasClassification(ShaderNode::Classification::CONVOLUTION2D))
    {
        // Emit sampling functions
        emitInclude("stdlib/" + GlslShaderGenerator::LANGUAGE + "/lib/mx_sampling.glsl", context, pixelStage);
        emitLineBreak(pixelStage);
    }

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "stdlib/" + GlslShaderGenerator::LANGUAGE +
        (context.getOptions().fileTextureVerticalFlip ? "/lib/mx_transform_uv_vflip.glsl": "/lib/mx_transform_uv.glsl");

    // Add all functions for node implementations
    emitFunctionDefinitions(graph, context, pixelStage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add function signature
    // Keep track of arguments we changed from matrix3 to matrix4 as additional
    // code must be inserted to get back the matrix3 version
    StringVec convertMatrixStrings;

    string functionName = shader->getName();
    _syntax->makeIdentifier(functionName, graph.getIdentifierMap());
    setFunctionName(functionName, pixelStage);

    emitLine((context.getOptions().hwTransparency ? "vec4 " : "vec3 ") + functionName, pixelStage, false);

    // Emit public uniforms as function arguments
    //
    emitScopeBegin(pixelStage, Syntax::PARENTHESES);
    const VariableBlock& publicUniforms = pixelStage.getUniformBlock(HW::PUBLIC_UNIFORMS);
    const size_t numPublicUniforms = publicUniforms.size();
    for (size_t i = 0; i < numPublicUniforms; ++i)
    {
        emitLineBegin(pixelStage);
        if (publicUniforms[i]->getType() == Type::MATRIX33)
        {
            convertMatrixStrings.push_back(publicUniforms[i]->getVariable());
        }
        emitVariableDeclaration(publicUniforms[i], EMPTY_STRING, context, pixelStage, false);
        if (i < numPublicUniforms - 1)
        {
            emitString(COMMA, pixelStage);
        }
        emitLineEnd(pixelStage, false);
    }
    // Special case handling of world space normals which for now must be added 
    // as a "dummy" argument if it exists.
    const VariableBlock& streams = pixelStage.getInputBlock(HW::VERTEX_DATA);
    const ShaderPort* port = streams.find(HW::T_NORMAL_WORLD);
    if (port)
    { 
        emitLineBegin(pixelStage);
        emitString(COMMA, pixelStage);
        emitVariableDeclaration(port, EMPTY_STRING, context, pixelStage, false);
        emitLineEnd(pixelStage, false);
    }

    if (context.getOptions().hwTransparency)
    {
        // A dummy argument not used in the generated shader code but necessary to
        // map onto an OGS fragment parameter and a shading node DG attribute with
        // the same name that can be set to a non-0 value to let Maya know that the
        // surface is transparent.
        emitLineBegin(pixelStage);
        emitString(COMMA, pixelStage);
        emitString("float ", pixelStage);
        emitString(OgsXmlGenerator::VP_TRANSPARENCY_NAME, pixelStage);
        emitLineEnd(pixelStage, false);
    }

    emitScopeEnd(pixelStage);

    // Add function body
    emitScopeBegin(pixelStage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching 
        // to a surface shader, so just output black.
        emitLine("return vec3(0.0)", pixelStage);
    }
    else
    {
        // Insert matrix converters
        for (const string& argument : convertMatrixStrings)
        {
            emitLine("mat3 " + argument + " = mat3(" + argument + GlslFragmentGenerator::MATRIX3_TO_MATRIX4_POSTFIX + ")", pixelStage, true);
        }

        // Add all function calls
        emitFunctionCalls(graph, context, pixelStage);

        // Emit final result
        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection)
        {
            string finalOutput = outputConnection->getVariable();
            const string& channels = outputSocket->getChannels();
            if (!channels.empty())
            {
                finalOutput = _syntax->getSwizzledVariable(finalOutput, outputConnection->getType(), channels, outputSocket->getType());
            }

            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                if (context.getOptions().hwTransparency)
                {
                    emitLine("return vec4(" + finalOutput + ".color, clamp(1.0 - dot(" + finalOutput + ".transparency, vec3(0.3333)), 0.0, 1.0))", pixelStage);
                }
                else
                {
                    emitLine("return " + finalOutput + ".color", pixelStage);
                }
            }
            else
            {
                if (!outputSocket->getType()->isFloat3())
                {
                    toVec3(outputSocket->getType(), finalOutput);
                }
                emitLine("return " + finalOutput, pixelStage);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ? _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) : _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType()->isFloat3())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue, pixelStage);
                toVec3(outputSocket->getType(), finalOutput);
                emitLine("return " + finalOutput, pixelStage);
            }
            else
            {
                emitLine("return " + outputValue, pixelStage);
            }
        }
    }

    // End function
    emitScopeEnd(pixelStage);

    // Replace all tokens with real identifier names
    replaceTokens(_tokenSubstitutions, pixelStage);

    {
        const VariableBlock& privateUniforms = pixelStage.getUniformBlock(HW::PRIVATE_UNIFORMS);
        if (!privateUniforms.empty())
        {
            ShaderStage& uniformStage = shader->getStage(Stage::PRIVATE_UNIFORMS);
            emitVariableDeclarations(
                privateUniforms, _syntax->getUniformQualifier(), SEMICOLON, context, uniformStage
            );
            emitLineBreak(uniformStage);
        }
    }

    return shader;
}

void GlslFragmentGenerator::toVec3(const TypeDesc* type, string& variable)
{
    if (type->isFloat2())
    {
        variable = "vec3(" + variable + ", 0.0)";
    }
    else if (type->isFloat4())
    {
        variable = variable + ".xyz";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER)
    {
        variable = "vec3(" + variable + ", " + variable + ", " + variable + ")";
    }
    else if (type == Type::BSDF || type == Type::EDF)
    {
        variable = "vec3(" + variable + ")";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec3(0.0, 0.0, 0.0)";
    }
}

void GlslFragmentGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                    GenContext& context, ShaderStage& stage,
                                                    bool assignValue) const
{
    // We change matrix3 to matrix4 input arguments
    if (variable->getType() == Type::MATRIX33)
    {
        string qualifierPrefix = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(qualifierPrefix + "mat4 " + variable->getVariable() + MATRIX3_TO_MATRIX4_POSTFIX, stage);
    }
    else
    {
        GlslShaderGenerator::emitVariableDeclaration(variable, qualifier, context, stage, assignValue);
    }
}

} // namespace MaterialX
