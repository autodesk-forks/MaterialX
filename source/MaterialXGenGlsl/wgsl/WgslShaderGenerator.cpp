//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/wgsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/wgsl/WgslSyntax.h>

#include <MaterialXGenHw/HwConstants.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Util.h>

#include <MaterialXGenShader/GenUserData.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::TARGET = "genwgsl";
const string WgslShaderGenerator::PIXEL_ENTRY = "material_main";
const string WgslShaderGenerator::VERTEX_ENTRY = "vs_main";
const string WgslShaderGenerator::LIGHTDATA_TYPEVAR_STRING = "light_type";

namespace
{
// Tracks include files (by basename) already expanded during a generation, so the
// genglsl headers (closure types, math) that many BSDF sources #include are emitted
// once rather than producing duplicate WGSL struct/const definitions.
class WgslIncludeSet : public GenUserData
{
  public:
    std::set<string> basenames;
};
const string WGSL_INCLUDES = "WGSL_INCLUDES";
} // namespace

ShaderNodeImplPtr WgslShaderGenerator::getImplementation(const NodeDef& nodedef, GenContext& context) const
{
    InterfaceElementPtr implElement = nodedef.getImplementation(TARGET);
    if (!implElement)
    {
        // WGSL reuses genglsl node implementations until genwgsl-specific overrides exist.
        implElement = nodedef.getImplementation(VkShaderGenerator::TARGET);
    }
    if (!implElement)
    {
        return nullptr;
    }

    const string& name = implElement->getName();
    ShaderNodeImplPtr impl = context.findNodeImplementation(name);
    if (impl)
    {
        return impl;
    }

    if (implElement->isA<NodeGraph>())
    {
        impl = createShaderNodeImplForNodeGraph(*implElement->asA<NodeGraph>());
    }
    else if (implElement->isA<Implementation>())
    {
        if (getColorManagementSystem() && getColorManagementSystem()->hasImplementation(name))
        {
            impl = getColorManagementSystem()->createImplementation(name);
        }
        else
        {
            impl = _implFactory.create(name);
        }
        if (!impl)
        {
            impl = createShaderNodeImplForImplementation(*implElement->asA<Implementation>());
        }
    }
    if (!impl)
    {
        return nullptr;
    }

    impl->initialize(*implElement, context);
    context.addNodeImplementation(name, impl);
    return impl;
}

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    VkShaderGenerator(typeSystem)
{
    _syntax = WgslSyntax::create(typeSystem);

    // Set binding context to handle resource binding layouts
    _resourceBindingCtx = std::make_shared<MaterialX::WgslResourceBindingContext>(0);

    // For functions described in ::emitSpecularEnvironment()
    // override map value from HwShaderGenerator
    _tokenSubstitutions[HW::T_ENV_RADIANCE] = HW::ENV_RADIANCE_SPLIT;
    _tokenSubstitutions[HW::T_ENV_RADIANCE_SAMPLER2D] = HW::ENV_RADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE] = HW::ENV_IRRADIANCE_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE_SAMPLER2D] = HW::ENV_IRRADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SAMPLER2D] = HW::TEX_SAMPLER_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE] = HW::TEX_SAMPLER_SIGNATURE_SPLIT;
}

void WgslShaderGenerator::emitDirectives(GenContext&, ShaderStage&) const
{
    // WGSL has no `#version` / preprocessor directives.
}

void WgslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                  GenContext&, ShaderStage& stage, bool assignValue) const
{
    if (variable->getType() == Type::FILENAME)
    {
        // Combined GLSL sampler2D becomes a WGSL texture (sampler emitted separately).
        emitString("var " + variable->getVariable() + ": texture_2d<f32>", stage);
        return;
    }

    string typeName = _syntax->getTypeName(variable->getType());
    if (variable->getType().isArray() && variable->getValue())
        typeName += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());

    const bool isConst = (qualifier == _syntax->getConstantQualifier());
    const string keyword = isConst ? "const " : "var ";
    string line = keyword + variable->getVariable() + ": " + typeName;

    const bool isUniform = (qualifier == _syntax->getUniformQualifier());
    if (assignValue && !isUniform)
    {
        const string valueStr = variable->getValue()
                                    ? _syntax->getValue(variable->getType(), *variable->getValue())
                                    : _syntax->getDefaultValue(variable->getType());
        if (!valueStr.empty())
            line += " = " + valueStr;
    }

    emitString(line, stage);
}

void WgslShaderGenerator::emitOutput(const ShaderOutput* output, bool includeType, bool assignValue,
                                     GenContext& context, ShaderStage& stage) const
{
    if (includeType)
    {
        stage.addString("var " + output->getVariable() + ": " + _syntax->getTypeName(output->getType()));
    }
    else
    {
        // Function-call argument: WGSL out parameters are `ptr<function,T>`, passed as `&var`.
        stage.addString((assignValue ? "" : "&") + output->getVariable());
    }

    string suffix;
    context.getOutputSuffix(output, suffix);
    if (!suffix.empty())
        stage.addString(suffix);

    if (assignValue)
    {
        const string& value = _syntax->getDefaultValue(output->getType());
        if (!value.empty())
            stage.addString(" = " + value);
    }
}

void WgslShaderGenerator::emitString(const string& str, ShaderStage& stage) const
{
    if (str.size() >= 5 && str.compare(0, 5, "void ") == 0)
    {
        string replaced = str;
        replaced.replace(0, 5, "fn ");
        VkShaderGenerator::emitString(replaced, stage);
        return;
    }
    VkShaderGenerator::emitString(str, stage);
}

void WgslShaderGenerator::emitLineEnd(ShaderStage& stage, bool semicolon) const
{
    // Rewrite the line just built via emitString (e.g. by SourceCodeNode) to WGSL.
    const string& code = stage.getSourceCode();
    const string& newlineStr = _syntax->getNewline();
    const size_t newlineLen = newlineStr.empty() ? 1 : newlineStr.size();
    const size_t lastNewline = code.rfind(newlineStr);
    const string lastLine = (lastNewline == string::npos) ? code : code.substr(lastNewline + newlineLen);
    if (!lastLine.empty())
    {
        const string rewritten = WgslRewrite::rewriteAll(lastLine);
        if (rewritten != lastLine)
        {
            const string newCode = (lastNewline == string::npos)
                                       ? rewritten
                                       : code.substr(0, lastNewline + newlineLen) + rewritten;
            stage.setSourceCode(newCode);
        }
    }
    VkShaderGenerator::emitLineEnd(stage, semicolon);
}

void WgslShaderGenerator::emitLine(const string& str, ShaderStage& stage, bool semicolon) const
{
    // Split a run of statements onto separate lines, then apply the GLSL->WGSL rewrites.
    const size_t firstNewline = str.find('\n');
    if (firstNewline != string::npos)
    {
        emitLine(str.substr(0, firstNewline), stage, true);
        emitLine(str.substr(firstNewline + 1), stage, semicolon);
        return;
    }
    VkShaderGenerator::emitLine(WgslRewrite::rewriteAll(str), stage, semicolon);
}

string WgslShaderGenerator::expandAndRewriteGlsl(const string& source, const FilePath& sourceFilename,
                                                 GenContext& context, WgslRewrite::LineRewriter& rewriter) const
{
    // Apply token substitutions ($fileTransformUv etc.) up front so include paths
    // and sampler/texture rewrites see resolved text. Apply longest keys first: some
    // tokens are prefixes of others (`$envRadiance` of `$envRadianceSamples`), and the
    // stock GLSL substitution values mask this by accident (`u_envRadiance`+`Samples` ==
    // `u_envRadianceSamples`) — but the split sampler values used here do not, so an
    // unordered pass would corrupt the longer token.
    std::vector<std::pair<string, string>> subs(getTokenSubstitutions().begin(),
                                                getTokenSubstitutions().end());
    std::sort(subs.begin(), subs.end(),
              [](const std::pair<string, string>& a, const std::pair<string, string>& b)
    {
        return a.first.length() > b.first.length();
    });
    string substituted = source;
    for (const auto& pair : subs)
    {
        if (pair.first.empty())
            continue;
        size_t pos = 0;
        while ((pos = substituted.find(pair.first, pos)) != string::npos)
        {
            substituted.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }

    std::istringstream stream(substituted);
    string line;
    string out;
    out.reserve(substituted.size() + substituted.size() / 8);

    while (std::getline(stream, line))
    {
        const bool hadCr = !line.empty() && line.back() == '\r';
        if (hadCr)
            line.pop_back();

        const size_t firstNonSpace = line.find_first_not_of(" \t");
        const bool isInclude = (firstNonSpace != string::npos &&
                                line.compare(firstNonSpace, 8, "#include") == 0);
        if (isInclude)
        {
            const size_t q1 = line.find('"', firstNonSpace + 8);
            const size_t q2 = (q1 == string::npos) ? string::npos : line.find('"', q1 + 1);
            if (q1 == string::npos || q2 == string::npos || q2 <= q1 + 1)
                continue; // malformed include: drop it
            const string includePath = line.substr(q1 + 1, q2 - q1 - 1);
            // Expand each header only once per generation (avoids duplicate WGSL defs).
            auto seen = context.getUserData<WgslIncludeSet>(WGSL_INCLUDES);
            const string baseName = FilePath(includePath).getBaseName();
            if (seen && seen->basenames.count(baseName))
                continue;
            if (seen)
                seen->basenames.insert(baseName);
            const FilePath resolved = context.resolveSourceFile(includePath, sourceFilename.getParentPath());
            const string content = readFile(resolved);
            if (!content.empty())
                out += expandAndRewriteGlsl(content, resolved, context, rewriter);
            continue;
        }

        out += rewriter.rewrite(line);
        if (hadCr)
            out += '\r';
        out += '\n';
    }
    return out;
}

void WgslShaderGenerator::emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const
{
    if (sourceFilename.getExtension() == "glsl" && !str.empty())
    {
        WgslRewrite::LineRewriter rewriter;
        const string wgsl = expandAndRewriteGlsl(str, sourceFilename, context, rewriter);
        stage.addBlock(wgsl, sourceFilename, context);
        return;
    }
    VkShaderGenerator::emitBlock(str, sourceFilename, context, stage);
}

void WgslShaderGenerator::emitLibraryInclude(const FilePath& filename, GenContext& context, ShaderStage& stage) const
{
    FilePath libraryPrefix = context.getOptions().libraryPrefix;
    FilePath fullFilename = libraryPrefix.isEmpty() ? filename : libraryPrefix / filename;
    FilePath resolvedFilename = context.resolveSourceFile(fullFilename, FilePath());
    if (resolvedFilename.exists())
    {
        emitBlock(readFile(resolvedFilename), resolvedFilename, context, stage);
    }
}

void WgslShaderGenerator::emitTypeDefinitions(GenContext&, ShaderStage&) const
{
    // GLSL typedefs are `#define` directives (e.g. `#define EDF vec3`). WGSL has no
    // preprocessor — surface/closure aliases are emitted by emitWgslSurfaceTypes().
}

namespace
{
// Format a MaterialX port value as a JSON scalar or array (numbers/bools).
string valueToJson(const ShaderPort* port)
{
    ValuePtr value = port->getValue();
    if (!value)
        return "null";
    const string s = value->getValueString();
    if (s.find(',') != string::npos)
    {
        string out = "[";
        std::stringstream ss(s);
        string item;
        bool first = true;
        while (std::getline(ss, item, ','))
        {
            const size_t b = item.find_first_not_of(" \t");
            const size_t e = item.find_last_not_of(" \t");
            if (b == string::npos)
                continue;
            out += (first ? "" : ", ") + item.substr(b, e - b + 1);
            first = false;
        }
        return out + "]";
    }
    if (s == "true" || s == "false")
        return s;
    return s.empty() ? "0" : s;
}
} // namespace

void WgslShaderGenerator::emitWgslSurfaceTypes(GenContext&, ShaderStage& stage) const
{
    stage.addString(R"WGSL(// MaterialX closure result types (multi-line so the
// overload/broadcast type-inference pass can read their members).
struct BSDF {
    response: vec3f,
    throughput: vec3f,
}
struct VDF {
    response: vec3f,
    throughput: vec3f,
}
alias EDF = vec3<f32>;
struct surfaceshader {
    color: vec3f,
    transparency: vec3f,
}
struct volumeshader {
    color: vec3f,
    transparency: vec3f,
}
struct displacementshader {
    offset: vec3f,
    scale: f32,
}
alias material = surfaceshader;
struct lightshader {
    direction: vec3f,
    intensity: vec3f,
}
)WGSL");
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitVertexDataStruct(const VariableBlock& vertexData, ShaderStage& stage) const
{
    if (vertexData.empty())
        return;
    emitLine("struct " + vertexData.getName(), stage, false);
    emitScopeBegin(stage);
    for (size_t i = 0; i < vertexData.size(); ++i)
    {
        const string name = replaceSubstrings(vertexData[i]->getVariable(), getTokenSubstitutions());
        // WGSL struct members are comma-separated (not `;`-terminated like GLSL).
        emitLine(name + ": " + _syntax->getTypeName(vertexData[i]->getType()) + ",", stage, false);
    }
    emitScopeEnd(stage, false);
    emitLineBreak(stage);
}

void WgslShaderGenerator::emitPixelUniforms(GenContext& context, ShaderStage& stage, bool lighting,
                                            std::stringstream& bindingsJson) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (!resourceBindingCtx)
        return;

    auto wgslCtx = std::dynamic_pointer_cast<WgslResourceBindingContext>(resourceBindingCtx);
    wgslCtx->setBindingLocation(0);

    bool firstBinding = true;
    auto recordBinding = [&](size_t binding, const string& name, const string& type, const string& role,
                             const string& value = EMPTY_STRING, const string& key = EMPTY_STRING)
    {
        // Token substitution (`$envMatrix` -> `u_envMatrix`, etc.) runs on the stage source
        // after emit, but the manifest is a separate string. Apply it here so binding names
        // match the identifiers used in the generated WGSL body. Use tokenSubstitution (not
        // replaceSubstrings) so a `$`-token is matched as a whole identifier — a substring
        // replace would corrupt e.g. `$envRadianceSamples` via the `$envRadiance` split entry.
        string subName = name;
        tokenSubstitution(getTokenSubstitutions(), subName);
        string subKey = key;
        tokenSubstitution(getTokenSubstitutions(), subKey);
        bindingsJson << (firstBinding ? "" : ",\n")
                     << "    { \"stage\": \"pixel\", \"group\": 0, \"binding\": " << binding
                     << ", \"name\": \"" << subName << "\", \"type\": \"" << type << "\", \"role\": \"" << role << "\"";
        if (!value.empty())
            bindingsJson << ", \"value\": " << value;
        if (!subKey.empty())
            bindingsJson << ", \"key\": \"" << subKey << "\"";
        bindingsJson << " }";
        firstBinding = false;
    };

    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;
        if (uniforms.empty() || uniforms.getName() == HW::LIGHT_DATA)
            continue;

        emitComment("Uniform block: " + uniforms.getName(), stage);
        size_t binding = wgslCtx->getBindingLocation();
        for (auto uniform : uniforms.getVariableOrder())
        {
            const TypeDesc t = uniform->getType();
            if (t.isClosure() || t == Type::SURFACESHADER || t == Type::MATERIAL ||
                t == Type::DISPLACEMENTSHADER || t == Type::VOLUMESHADER || t == Type::LIGHTSHADER)
                continue;

            if (uniform->getType() == Type::FILENAME)
            {
                const string& name = uniform->getVariable();
                recordBinding(binding, name + "_texture", "texture_2d<f32>", "texture", "null", name);
                recordBinding(binding + 1, name + "_sampler", "sampler", "sampler", "null", name);
                binding += 2;
            }
            else
            {
                const string typeName = wgslCtx->getWgslUniformType(uniform, *_syntax);
                const string role = (uniforms.getName() == HW::PUBLIC_UNIFORMS) ? "uniform" : "host";
                recordBinding(binding, uniform->getVariable(), typeName, role, valueToJson(uniform));
                binding += 1;
            }
        }
        resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
        emitLineBreak(stage);
    }

    if (lighting && context.getOptions().hwMaxActiveLightSources > 0)
    {
        const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
        const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
        const size_t binding = wgslCtx->getBindingLocation();
        const string count = structArraySuffix.substr(1, structArraySuffix.size() - 2);
        recordBinding(binding, lightData.getInstance(), "array<" + lightData.getName() + ", " + count + ">", "lightData");
        resourceBindingCtx->emitStructuredResourceBindings(context, lightData, stage, lightData.getInstance(), structArraySuffix);
    }
}

void WgslShaderGenerator::emitLightData(GenContext&, ShaderStage&) const
{
    // emitPixelUniforms() already records the manifest binding and emits the LightData struct
    // plus u_lightData array. The GLSL base class would emit a second copy here.
}

void WgslShaderGenerator::finalizeStageSource(ShaderStage& stage) const
{
    string src = stage.getSourceCode();
    // HwNumLightsNode / HwLightSamplerNode emit GLSL function definitions that per-line
    // rewriteAll() does not convert. Rewrite only those stubs (not the whole stage).
    src = WgslRewrite::rewriteResidualGlslFunctions(src);
    src = WgslRewrite::dedupDefinitions(src);
    src = WgslRewrite::derefPointerParams(src);
    src = WgslRewrite::resolveOverloads(src);
    src = WgslRewrite::coerceBoolCallSites(src);
    src = WgslRewrite::repairEmptyElseCommentBlocks(src);
    stage.setSourceCode(src);

    for (const string& issue : WgslRewrite::findResidualGlsl(src))
    {
        std::cerr << "Warning: WgslShaderGenerator: " << issue << " in generated shader." << std::endl;
    }
}

void WgslShaderGenerator::emitInputs(GenContext& /*context*/, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitComment("Inputs block: " + vertexInputs.getName(), stage);
            emitLine("struct VertexInput", stage, false);
            emitScopeBegin(stage);
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const string name = replaceSubstrings(vertexInputs[i]->getVariable(), getTokenSubstitutions());
                emitLine("@location(" + std::to_string(i) + ") " + name + ": " +
                             _syntax->getTypeName(vertexInputs[i]->getType()),
                         stage);
            }
            emitScopeEnd(stage, false);
            emitLineBreak(stage);
        }
    }
}

void WgslShaderGenerator::emitOutputs(GenContext& /*context*/, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitVertexDataStruct(vertexData, stage);
            emitLine("struct VertexOutput", stage, false);
            emitScopeBegin(stage);
            emitLine("@builtin(position) position: vec4f", stage);
            emitLine("@location(" + std::to_string(vertexDataLocation) + ") " + vertexData.getInstance() + ": " +
                         vertexData.getName(),
                     stage);
            emitScopeEnd(stage, false);
            emitLineBreak(stage);
        }
    }
}

void WgslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    emitDirectives(context, stage);
    emitLineBreak(stage);

    emitConstants(context, stage);
    emitUniforms(context, stage);
    emitInputs(context, stage);
    emitOutputs(context, stage);

    emitLibraryInclude("stdlib/genglsl/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    emitFunctionDefinitions(graph, context, stage);

    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    const string vdInstance = vertexData.getInstance();

    setFunctionName(VERTEX_ENTRY, stage);
    emitLine("fn " + VERTEX_ENTRY + "(inputs: VertexInput) -> VertexOutput", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    emitLine("var output: VertexOutput", stage);
    emitLine("var " + vdInstance + ": " + vertexData.getName(), stage);
    emitLine("let hPositionWorld = " + HW::T_WORLD_MATRIX + " * vec4f(inputs." + HW::T_IN_POSITION + ", 1.0)", stage);
    emitLine("output.position = " + HW::T_VIEW_PROJECTION_MATRIX + " * hPositionWorld", stage);

    for (const ShaderNode* node : graph.getNodes())
        emitFunctionCall(*node, context, stage);

    emitLine("output." + vdInstance + " = " + vdInstance, stage);
    emitLine("return output", stage);
    emitFunctionBodyEnd(graph, context, stage);

    finalizeStageSource(stage);
}

void WgslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] =
        context.getOptions().fileTextureVerticalFlip ? "mx_transform_uv_vflip.glsl" : "mx_transform_uv.glsl";

    auto includeSet = std::make_shared<WgslIncludeSet>();
    context.pushUserData(WGSL_INCLUDES, includeSet);

    emitDirectives(context, stage);
    emitLineBreak(stage);

    const bool lighting = requiresLighting(graph);
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();
    const TypeDesc outSocketType = outputSocket->getType();
    const bool isSurface = lighting || outSocketType == Type::SURFACESHADER || outSocketType == Type::MATERIAL;

    if (isSurface)
        emitWgslSurfaceTypes(context, stage);

    emitTypeDefinitions(context, stage);
    emitConstants(context, stage);

    std::stringstream bindingsJson;
    emitPixelUniforms(context, stage, lighting, bindingsJson);

    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    emitVertexDataStruct(vertexData, stage);

    emitLibraryInclude("stdlib/genglsl/lib/mx_math.glsl", context, stage);
    emitLineBreak(stage);

    if (lighting || context.getOptions().hwWriteAlbedoTable || context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine("const DIRECTIONAL_ALBEDO_METHOD: i32 = " + std::to_string(int(context.getOptions().hwDirectionalAlbedoMethod)), stage);
        emitLineBreak(stage);
    }

    emitLine("const AIRY_FRESNEL_ITERATIONS: i32 = " + std::to_string(context.getOptions().hwAiryFresnelIterations), stage);
    emitLineBreak(stage);

    if (lighting)
    {
        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
            emitLine("const " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + ": i32 = " + std::to_string(maxLights), stage);
            emitLineBreak(stage);
        }
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);
        if (context.getOptions().hwMaxActiveLightSources > 0)
            emitLightData(context, stage);
    }

    emitLightFunctionDefinitions(graph, context, stage);
    emitFunctionDefinitions(graph, context, stage);
    emitLineBreak(stage);

    std::stringstream entryMembersJson;
    bool firstMember = true;
    for (size_t i = 0; i < vertexData.size(); ++i)
    {
        const string name = replaceSubstrings(vertexData[i]->getVariable(), getTokenSubstitutions());
        const string typeName = _syntax->getTypeName(vertexData[i]->getType());
        entryMembersJson << (firstMember ? "" : ",\n")
                         << "        { \"name\": \"" << name << "\", \"type\": \"" << typeName << "\" }";
        firstMember = false;
    }

    const string vdInstance = vertexData.getInstance();
    const string entryParam = vdInstance + ": " + vertexData.getName();
    emitLine("fn " + PIXEL_ENTRY + "(" + entryParam + ") -> vec4f", stage, false);
    emitScopeBegin(stage);

    if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);
        for (const ShaderGraphOutputSocket* socket : graph.getOutputSockets())
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
        emitFunctionCalls(graph, context, stage);
    }

    const ShaderOutput* outputConnection = outputSocket->getConnection();
    string outValue = outputConnection ? outputConnection->getVariable() : _syntax->getDefaultValue(Type::COLOR3);
    const TypeDesc outType = outputSocket->getType();
    string vec4Value;
    if (isSurface && outputConnection)
    {
        string outColor = outValue + ".color";
        const string outTransparency = outValue + ".transparency";
        if (context.getOptions().hwSrgbEncodeOutput)
            outColor = "mx_srgb_encode(" + outColor + ")";
        if (context.getOptions().hwTransparency)
        {
            emitLine("let outAlpha: f32 = clamp(1.0 - dot(" + outTransparency + ", vec3f(0.3333)), 0.0, 1.0)", stage);
            vec4Value = "vec4f(" + outColor + ", outAlpha)";
            emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")", stage, false);
            emitScopeBegin(stage);
            emitLine("discard", stage);
            emitScopeEnd(stage);
        }
        else
        {
            vec4Value = "vec4f(" + outColor + ", 1.0)";
        }
    }
    else if (outType.isFloat4())
    {
        vec4Value = outValue;
    }
    else if (outType.isFloat3())
    {
        if (context.getOptions().hwSrgbEncodeOutput)
            outValue = "mx_srgb_encode(" + outValue + ")";
        vec4Value = "vec4f(" + outValue + ", 1.0)";
    }
    else
    {
        vec4Value = "vec4f(vec3f(" + outValue + "), 1.0)";
    }
    emitLine("return " + vec4Value, stage);
    emitScopeEnd(stage);

    std::stringstream manifest;
    manifest << "{\n"
             << "  \"entry\": { \"vertex\": \"" << VERTEX_ENTRY << "\", \"pixel\": \"" << PIXEL_ENTRY << "\" },\n"
             << "  \"output\": \"vec4f\",\n"
             << "  \"entryParams\": [\n"
             << "    { \"name\": \"" << vdInstance << "\", \"type\": \"" << vertexData.getName() << "\", \"members\": [\n"
             << entryMembersJson.str() << (firstMember ? "" : "\n") << "    ] }\n"
             << "  ],\n"
             << "  \"bindings\": [\n"
             << bindingsJson.str() << "\n"
             << "  ]\n"
             << "}\n";
    _generatedManifest = manifest.str();

    finalizeStageSource(stage);
}

// Called by CompoundNode::emitFunctionDefinition(). Emits WGSL parameter syntax:
// `name: type`, output parameters as `name: ptr<function, type>`, and a FILENAME
// as a split texture + sampler pair.
void WgslShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& /*context*/, ShaderStage& stage) const
{
    const string& name = shaderPort->getVariable();
    if (shaderPort->getType() == Type::FILENAME)
    {
        VkShaderGenerator::emitString(name + "_texture: texture_2d<f32>, " + name + "_sampler: sampler", stage);
        return;
    }
    const string typeName = _syntax->getTypeName(shaderPort->getType());
    if (isOutput)
        VkShaderGenerator::emitString(name + ": ptr<function, " + typeName + ">", stage);
    else
        VkShaderGenerator::emitString(name + ": " + typeName, stage);
}

// Called by SourceCodeNode::emitFunctionCall()
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME)
    {
        emitString(getUpstreamResult(input, context) + "_texture, " + getUpstreamResult(input, context) + "_sampler", stage);
    }
    else
    {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}

MATERIALX_NAMESPACE_END
