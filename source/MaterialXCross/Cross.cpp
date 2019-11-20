#include "Cross.h"
#include "MaterialXCore/Library.h"
#include "MaterialXGenShader/HwShaderGenerator.h"

#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"

#include "spirv_hlsl.hpp"

namespace MaterialX
{
namespace Cross
{
namespace
{
// Copied from glslang/MachineIndependent/ShaderLang.cpp
//
const TBuiltInResource defaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,

    /* .limits = */ {
    /* .nonInductiveForLoops = */ 1,
    /* .whileLoops = */ 1,
    /* .doWhileLoops = */ 1,
    /* .generalUniformIndexing = */ 1,
    /* .generalAttributeMatrixVectorIndexing = */ 1,
    /* .generalVaryingIndexing = */ 1,
    /* .generalSamplerIndexing = */ 1,
    /* .generalVariableIndexing = */ 1,
    /* .generalConstantMatrixVectorIndexing = */ 1,
} };

std::vector<uint32_t> glslToSpirv(
    const std::string& glslGlobalDefinitions,
    const std::string& glslFragment
)
{
    static const std::string dummyMain =
        "void main()\n"
        "{\n"
        "}\n\n";

    const char* shaderStrings[]{
        glslGlobalDefinitions.data(),
        glslFragment.data(),
        dummyMain.data()
    };

    const int stringLengths[]{
        static_cast<int>(glslGlobalDefinitions.size()),
        static_cast<int>(glslFragment.size()),
        static_cast<int>(dummyMain.size())
    };

    glslang::TShader shader(EShLangFragment);
    shader.setStringsWithLengths(shaderStrings, stringLengths, 3);
    shader.setEntryPoint("main");

    // Permits uniforms without explicit layout locations
    shader.setAutoMapLocations(true);

    constexpr auto messages = static_cast<EShMessages>(
        EShMsgSpvRules | EShMsgKeepUncalled | EShMsgDebugInfo
    );

    {
        constexpr int defaultVersion = 450;
        constexpr bool forwardCompatible = false;
        
        glslang::TShader::ForbidIncluder forbidIncluder;

        if (!shader.parse(
            &defaultTBuiltInResource,
            defaultVersion,
            forwardCompatible,
            messages,
            forbidIncluder
        ))
        {
            const char* const log = shader.getInfoLog();
            throw Exception(
                std::string("glslang failed to parse the GLSL fragment:\n") + log
            );
        }
    }

    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages) /*&& program.mapIO()*/)
    {
        const char* const log = program.getInfoLog();
        throw Exception(
            std::string("glslang failed to link the GLSL fragment:\n") + log
        );
    }

    std::vector<uint32_t> spirv;

    {
        glslang::SpvOptions options;
        options.generateDebugInfo = true;
        options.disableOptimizer = true; 
        options.optimizeSize = false;

        glslang::GlslangToSpv(
            *program.getIntermediate(EShLangFragment), spirv, &options
        );
    }

    return spirv;
}

class HlslFragmentCrossCompiler : public spirv_cross::CompilerHLSL
{
public:
    using spirv_cross::CompilerHLSL::CompilerHLSL;

    void emit_uniform(const spirv_cross::SPIRVariable& var) override
    {
        if (to_name(var.self) == HW::LIGHT_DATA_INSTANCE)
        {
            spirv_cross::CompilerHLSL::emit_uniform(var);
        }
    }
};

std::string spirvToHlsl(
    std::vector<uint32_t>&& spirv,
    const std::string& fragmentName
)
{
    auto crossCompiler = std::make_unique<HlslFragmentCrossCompiler>(std::move(spirv));
    crossCompiler->set_entry_point("main", spv::ExecutionModelFragment);

    spirv_cross::CompilerHLSL::Options hlslOptions = crossCompiler->get_hlsl_options();
    hlslOptions.shader_model = 50;
    hlslOptions.exported_functions.insert(fragmentName);
    crossCompiler->set_hlsl_options(hlslOptions);

    return crossCompiler->compile();
}

} // anonymous namespace

void initialize()
{
    glslang::InitializeProcess();
}

void finalize()
{
    glslang::FinalizeProcess();
}

std::string glslToHlsl(
    const std::string& glslGlobalDefinitions,
    const std::string& glslFragment,
    const std::string& fragmentName
)
{
    std::vector<uint32_t> spirv = glslToSpirv(glslGlobalDefinitions, glslFragment);
    return spirvToHlsl(std::move(spirv), fragmentName);
}

}
}
