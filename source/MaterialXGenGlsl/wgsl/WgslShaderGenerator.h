//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WGSLSHADERGENERATOR_H
#define MATERIALX_WGSLSHADERGENERATOR_H

/// @file
/// WGSL shader generator.

#include <MaterialXGenGlsl/vk/VkShaderGenerator.h>
#include <MaterialXGenGlsl/wgsl/WgslResourceBindingContext.h>
#include <MaterialXGenGlsl/wgsl/GlslToWgsl.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

using WgslShaderGeneratorPtr = shared_ptr<class WgslShaderGenerator>;

/// @class WgslShaderGenerator
/// A shader generator that emits WebGPU Shading Language (WGSL). It reuses the genglsl emit
/// pipeline (deriving from VkShaderGenerator) and converts the emitted GLSL to WGSL at emit
/// time via the WgslRewrite utilities, producing generic WGSL vertex and pixel stages plus a
/// neutral JSON reflection describing entry functions and resource bindings
/// (see getGeneratedManifest()).
class MX_GENGLSL_API WgslShaderGenerator : public VkShaderGenerator
{
  public:
    /// Constructor.
    WgslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<WgslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    const string& getTarget() const override { return TARGET; }

    ShaderNodeImplPtr getImplementation(const NodeDef& nodedef, GenContext& context) const override;

    void emitDirectives(GenContext& context, ShaderStage& stage) const override;

    const string& getLightDataTypevarString() const override { return LIGHTDATA_TYPEVAR_STRING; }

    void emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& context, ShaderStage& stage) const override;

    void emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const override;

    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                 bool assignValue = true) const override;

    void emitOutput(const ShaderOutput* output, bool includeType, bool assignValue, GenContext& context, ShaderStage& stage) const override;

    void emitString(const string& str, ShaderStage& stage) const override;

    void emitLine(const string& str, ShaderStage& stage, bool semicolon = true) const override;

    void emitLineEnd(ShaderStage& stage, bool semicolon = true) const override;

    void emitBlock(const string& str, const FilePath& sourceFilename, GenContext& context, ShaderStage& stage) const override;

    void emitLibraryInclude(const FilePath& filename, GenContext& context, ShaderStage& stage) const override;

    void emitTypeDefinitions(GenContext& context, ShaderStage& stage) const override;

    void emitInputs(GenContext& context, ShaderStage& stage) const override;

    void emitOutputs(GenContext& context, ShaderStage& stage) const override;

    void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    /// Emit a WGSL pixel stage with a single entry function `material_main` and record a
    /// neutral reflection manifest (bindings, entry parameters — no TSL semantics).
    void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const override;

    /// LightData is emitted in emitPixelUniforms(); skip the base-class duplicate.
    void emitLightData(GenContext& context, ShaderStage& stage) const override;

    /// The neutral reflection (JSON) produced by the most recent generate() call.
    const string& getGeneratedManifest() const { return _generatedManifest; }

    static const string TARGET;
    static const string PIXEL_ENTRY;
    static const string VERTEX_ENTRY;

  protected:
    static const string LIGHTDATA_TYPEVAR_STRING;

  private:
    string expandAndRewriteGlsl(const string& source, const FilePath& sourceFilename,
                                GenContext& context, WgslRewrite::LineRewriter& rewriter) const;

    void emitWgslSurfaceTypes(GenContext& context, ShaderStage& stage) const;

    void emitPixelUniforms(GenContext& context, ShaderStage& stage, bool lighting,
                           std::stringstream& bindingsJson) const;

    void emitVertexDataStruct(const VariableBlock& vertexData, ShaderStage& stage) const;

    void finalizeStageSource(ShaderStage& stage) const;

    mutable string _generatedManifest;
};

MATERIALX_NAMESPACE_END

#endif
