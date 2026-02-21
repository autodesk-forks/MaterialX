//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenGlsl/WgslSyntax.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

const string WgslShaderGenerator::LIGHTDATA_TYPEVAR_STRING = "light_type";

WgslShaderGenerator::WgslShaderGenerator(TypeSystemPtr typeSystem) :
    VkShaderGenerator(typeSystem)
{
    _syntax = WgslSyntax::create(typeSystem);
    
    // Set binding context to handle resource binding layouts
    _resourceBindingCtx = std::make_shared<MaterialX::WgslResourceBindingContext>(0);

    // For functions described in ::emitSpecularEnvironment()
    // override map value from HwShaderGenerator
    _tokenSubstitutions[HW::T_ENV_RADIANCE]             = HW::ENV_RADIANCE_SPLIT; 
    _tokenSubstitutions[HW::T_ENV_RADIANCE_SAMPLER2D]   = HW::ENV_RADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE]           = HW::ENV_IRRADIANCE_SPLIT;
    _tokenSubstitutions[HW::T_ENV_IRRADIANCE_SAMPLER2D] = HW::ENV_IRRADIANCE_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SAMPLER2D]    = HW::TEX_SAMPLER_SAMPLER2D_SPLIT;
    _tokenSubstitutions[HW::T_TEX_SAMPLER_SIGNATURE]    = HW::TEX_SAMPLER_SIGNATURE_SPLIT;
}

void WgslShaderGenerator::emitDirectives(GenContext& context, ShaderStage& stage) const
{
    VkShaderGenerator::emitDirectives(context, stage);
    // Add additional directives and #define statements here
    //   Example: emitLine("#define HW_SEPARATE_SAMPLERS", stage, false);
    emitLineBreak(stage);
}

// Called by CompoundNode::emitFunctionDefinition()
void WgslShaderGenerator::emitFunctionDefinitionParameter(const ShaderPort* shaderPort, bool isOutput, GenContext& context, ShaderStage& stage) const
{
    if (shaderPort->getType() == Type::FILENAME)
    {
        emitString("texture2D " + shaderPort->getVariable() + "_texture, sampler "+shaderPort->getVariable() + "_sampler", stage);
    }
    else
    {
        VkShaderGenerator::emitFunctionDefinitionParameter(shaderPort, isOutput, context, stage);
    }
}

// Called by SourceCodeNode::emitFunctionCall() and CompoundNode::emitFunctionCall()
void WgslShaderGenerator::emitInput(const ShaderInput* input, GenContext& context, ShaderStage& stage) const
{
    if (input->getType() == Type::FILENAME)
    {
        emitString(getUpstreamResult(input, context)+"_texture, "+getUpstreamResult(input, context)+"_sampler", stage);
    }
    else if (input->getType() == Type::BOOLEAN)
    {
        const string result = getUpstreamResult(input, context);
        emitString("bool(" + result + ")", stage);
    }
    else
    {
        VkShaderGenerator::emitInput(input, context, stage);
    }
}

void WgslShaderGenerator::replaceTokens(const StringMap& substitutions, ShaderStage& stage) const
{
    // Let the base handle all interface and code substitution with a safe map
    // (T_REFRACTION_TWO_SIDED -> u_refractionTwoSided, no bool wrap).
    StringMap safeSubstitutions = substitutions;
    safeSubstitutions[HW::T_REFRACTION_TWO_SIDED] = HW::REFRACTION_TWO_SIDED;
    ShaderGenerator::replaceTokens(safeSubstitutions, stage);

    // Now fix up code only: wrap bool-as-int uniform names in bool() at use sites,
    // but skip declaration lines (pattern: " name;").
    // Add more entries here if additional bool uniforms are introduced.
    const vector<std::pair<string, string>> boolUniforms = {
        { HW::REFRACTION_TWO_SIDED, "bool(" + HW::REFRACTION_TWO_SIDED + ")" },
    };
    string code = stage.getSourceCode();
    for (const auto& entry : boolUniforms)
    {
        const string& plainName = entry.first;
        const string& castName = entry.second;
        for (size_t pos = 0; (pos = code.find(plainName, pos)) != string::npos; )
        {
            bool isDecl = (pos > 0 && code[pos - 1] == ' ' &&
                           pos + plainName.size() < code.size() &&
                           code[pos + plainName.size()] == ';');
            if (isDecl)
            {
                pos += plainName.size();
            }
            else
            {
                code.replace(pos, plainName.size(), castName);
                pos += castName.size();
            }
        }
    }
    stage.setSourceCode(code);
}

MATERIALX_NAMESPACE_END
