//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenWgsl/WgslShaderGenerator.h>
#include <MaterialXGenWgsl/WgslSyntax.h>

#include <MaterialXGenHw/Nodes/HwImageNode.h>
#include <MaterialXGenHw/Nodes/HwGeomColorNode.h>
#include <MaterialXGenHw/Nodes/HwGeomPropValueNode.h>
#include <MaterialXGenHw/Nodes/HwTexCoordNode.h>
#include <MaterialXGenHw/Nodes/HwTransformNode.h>
#include <MaterialXGenHw/Nodes/HwPositionNode.h>
#include <MaterialXGenHw/Nodes/HwNormalNode.h>
#include <MaterialXGenHw/Nodes/HwTangentNode.h>
#include <MaterialXGenHw/Nodes/HwBitangentNode.h>
#include <MaterialXGenHw/Nodes/HwFrameNode.h>
#include <MaterialXGenHw/Nodes/HwTimeNode.h>
#include <MaterialXGenHw/Nodes/HwViewDirectionNode.h>
#include <MaterialXGenHw/Nodes/HwLightNode.h>
#include <MaterialXGenHw/Nodes/HwLightShaderNode.h>
#include <MaterialXGenHw/Nodes/HwLightSamplerNode.h>
#include <MaterialXGenHw/Nodes/HwNumLightsNode.h>
#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Nodes/MaterialNode.h>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::TARGET = "genwgsl";
const string WgslShaderGenerator::VERSION = "1.0";

//
// WgslShaderGenerator methods
//

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    HwShaderGenerator(typeSystem, WgslSyntax::create(typeSystem))
{
    //
    // Register all custom node implementation classes
    //

    StringVec elementNames;

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + WgslShaderGenerator::TARGET, HwPositionNode::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + WgslShaderGenerator::TARGET, HwNormalNode::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + WgslShaderGenerator::TARGET, HwTangentNode::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + WgslShaderGenerator::TARGET, HwBitangentNode::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + WgslShaderGenerator::TARGET, HwTexCoordNode::create);
    registerImplementation("IM_texcoord_vector3_" + WgslShaderGenerator::TARGET, HwTexCoordNode::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + WgslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color3_" + WgslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color4_" + WgslShaderGenerator::TARGET, HwGeomColorNode::create);
    // <!-- <geompropvalue> -->
    elementNames = {
        "IM_geompropvalue_integer_" + WgslShaderGenerator::TARGET,
        "IM_geompropvalue_float_" + WgslShaderGenerator::TARGET,
        "IM_geompropvalue_color3_" + WgslShaderGenerator::TARGET,
        "IM_geompropvalue_color4_" + WgslShaderGenerator::TARGET,
        "IM_geompropvalue_vector2_" + WgslShaderGenerator::TARGET,
        "IM_geompropvalue_vector3_" + WgslShaderGenerator::TARGET,
        "IM_geompropvalue_vector4_" + WgslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwGeomPropValueNode::create);
    registerImplementation("IM_geompropvalue_boolean_" + WgslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + WgslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_filename_" + WgslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + WgslShaderGenerator::TARGET, HwFrameNode::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + WgslShaderGenerator::TARGET, HwTimeNode::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_" + WgslShaderGenerator::TARGET, HwViewDirectionNode::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + WgslShaderGenerator::TARGET, HwSurfaceNode::create);

    // <!-- <light> -->
    registerImplementation("IM_light_" + WgslShaderGenerator::TARGET, HwLightNode::create);

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + WgslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + WgslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + WgslShaderGenerator::TARGET, HwLightShaderNode::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + WgslShaderGenerator::TARGET, HwTransformPointNode::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + WgslShaderGenerator::TARGET, HwTransformVectorNode::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + WgslShaderGenerator::TARGET, HwTransformNormalNode::create);

    // <!-- <image> -->
    elementNames = {
        "IM_image_float_" + WgslShaderGenerator::TARGET,
        "IM_image_color3_" + WgslShaderGenerator::TARGET,
        "IM_image_color4_" + WgslShaderGenerator::TARGET,
        "IM_image_vector2_" + WgslShaderGenerator::TARGET,
        "IM_image_vector3_" + WgslShaderGenerator::TARGET,
        "IM_image_vector4_" + WgslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + WgslShaderGenerator::TARGET, MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", HwNumLightsNode::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", HwLightSamplerNode::create()));
}

ShaderPtr WgslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    return shader;
}

void WgslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitDirectives(context, stage);
    emitLineBreak(stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genwgsl/lib/mx_math.wgsl", context, stage);
    emitLineBreak(stage);

    emitFunctionDefinitions(graph, context, stage);

    // Add main function
    setFunctionName("main", stage);
    emitLine("@vertex", stage, false);
    emitLine("fn main(", stage, false);
    emitLine("    @location(0) " + HW::T_IN_POSITION + ": vec3<f32>,", stage, false);
    emitLine(") -> @builtin(position) vec4<f32> {", stage, false);
    
    emitScopeBegin(stage, Syntax::CURLY_BRACKETS);
    emitLine("var hPositionWorld: vec4<f32> = " + HW::T_WORLD_MATRIX + " * vec4<f32>(" + HW::T_IN_POSITION + ", 1.0);", stage);
    emitLine("return " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld;", stage);

    // Emit all function calls in order
    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(*node, context, stage);
    }

    emitScopeEnd(stage);
}

void WgslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_environment_fis.wgsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_environment_prefilter.wgsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_environment_none.wgsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitTransmissionRender(GenContext& context, ShaderStage& stage) const
{
    int transmissionMethod = context.getOptions().hwTransmissionRenderMethod;
    if (transmissionMethod == TRANSMISSION_REFRACTION)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_transmission_refract.wgsl", context, stage);
    }
    else if (transmissionMethod == TRANSMISSION_OPACITY)
    {
        emitLibraryInclude("pbrlib/genwgsl/lib/mx_transmission_opacity.wgsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid transmission render specified: '" + std::to_string(transmissionMethod) + "'");
    }
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
    emitComment("WGSL Shader Generated by MaterialX", stage);
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }
}

void WgslShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage) const
{
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            emitVariableDeclarations(uniforms, EMPTY_STRING, Syntax::SEMICOLON, context, stage);
            emitLineBreak(stage);
        }
    }
}

void WgslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitComment("Inputs block: " + vertexInputs.getName(), stage);
            emitVariableDeclarations(vertexInputs, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitComment("Vertex data inputs", stage);
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitLineBreak(stage);
        }
    }
}

void WgslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitComment("Vertex data outputs", stage);
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        emitComment("Pixel shader outputs", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        emitVariableDeclarations(outputs, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitLineBreak(stage);
    }
}

string WgslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

void WgslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    // Add directives
    emitDirectives(context, stage);
    emitLineBreak(stage);

    // Add type definitions
    emitTypeDefinitions(context, stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex data inputs block
    emitInputs(context, stage);

    // Add the pixel shader output
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genwgsl/lib/mx_math.wgsl", context, stage);
    emitLineBreak(stage);

    // Determine whether lighting is required
    bool lighting = requiresLighting(graph);

    // Add lighting support
    if (lighting)
    {
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);
    }

    // Set the include file to use for uv transformations
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv_vflip.wgsl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv.wgsl";
    }

    emitLightFunctionDefinitions(graph, context, stage);

    // Emit function definitions for all nodes in the graph.
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    setFunctionName("main", stage);
    emitLine("@fragment", stage, false);
    emitLine("fn main() -> @location(0) vec4<f32> {", stage, false);
    emitScopeBegin(stage, Syntax::CURLY_BRACKETS);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
        !graph.hasClassification(ShaderNode::Classification::SHADER))
    {
        // Handle the case where the graph is a direct closure.
        emitLine("return vec4<f32>(0.0, 0.0, 0.0, 1.0);", stage);
    }
    else
    {
        // Add all function calls.
        if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            // Emit all texturing nodes.
            emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);

            // Emit function calls for "root" closure/shader nodes.
            for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
            {
                if (socket->getConnection())
                {
                    const ShaderNode* upstream = socket->getConnection()->getNode();
                    if (upstream->getParent() == &graph &&
                        (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                         upstream->hasClassification(ShaderNode::Classification::SHADER)))
                    {
                        emitFunctionCall(*upstream, context, stage);
                    }
                }
            }
        }
        else
        {
            // No surface shader graph so just generate all function calls in order.
            emitFunctionCalls(graph, context, stage);
        }

        // Emit final output
        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection)
        {
            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                string outColor = outputConnection->getVariable() + ".color";
                string outTransparency = outputConnection->getVariable() + ".transparency";
                emitLine("var outAlpha: f32 = clamp(1.0 - dot(" + outTransparency + ", vec3<f32>(0.3333)), 0.0, 1.0);", stage);
                emitLine("return vec4<f32>(" + outColor + ", outAlpha);", stage);
            }
            else
            {
                string outValue = outputConnection->getVariable();
                if (!outputSocket->getType().isFloat4())
                {
                    toVec4(outputSocket->getType(), outValue);
                }
                emitLine("return " + outValue + ";", stage);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ?
                                 _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) :
                                 _syntax->getDefaultValue(outputSocket->getType());
            if (!outputSocket->getType().isFloat4())
            {
                toVec4(outputSocket->getType(), outputValue);
            }
            emitLine("return " + outputValue + ";", stage);
        }
    }

    // End main function
    emitScopeEnd(stage);
}

void WgslShaderGenerator::emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Emit Light functions if requested
        if (requiresLighting(graph) && context.getOptions().hwMaxActiveLightSources > 0)
        {
            // Emit functions for light sampling
            for (const auto& it : _lightSamplingNodes)
            {
                emitFunctionDefinition(*it, context, stage);
            }
        }
    }
}

void WgslShaderGenerator::toVec4(TypeDesc type, string& variable)
{
    if (type.isFloat3())
    {
        variable = "vec4<f32>(" + variable + ", 1.0)";
    }
    else if (type.isFloat2())
    {
        variable = "vec4<f32>(" + variable + ", 0.0, 1.0)";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER || type == Type::BOOLEAN)
    {
        variable = "vec4<f32>(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else if (type == Type::BSDF || type == Type::EDF)
    {
        variable = "vec4<f32>(" + variable + ", 1.0)";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec4<f32>(0.0, 0.0, 0.0, 1.0)";
    }
}

void WgslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                  GenContext&, ShaderStage& stage,
                                                  bool assignValue) const
{
    // A file texture input needs special handling in WGSL
    if (variable->getType() == Type::FILENAME)
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "@group(0) @binding(0) var " + variable->getVariable() + "_texture: texture_2d<f32>", stage);
        emitLineBreak(stage);
        emitString(str + "@group(0) @binding(1) var " + variable->getVariable() + "_sampler: sampler", stage);
    }
    else
    {
        string str = qualifier.empty() ? "var " : qualifier + " ";
        str += variable->getVariable() + ": " + _syntax->getTypeName(variable->getType());

        // If an array we need an array qualifier
        if (variable->getType().isArray() && variable->getValue())
        {
            str += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());
        }

        if (assignValue)
        {
            const string valueStr = (variable->getValue() ?
                                    _syntax->getValue(variable->getType(), *variable->getValue(), true) :
                                    _syntax->getDefaultValue(variable->getType(), true));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(str, stage);
    }
}

MATERIALX_NAMESPACE_END

