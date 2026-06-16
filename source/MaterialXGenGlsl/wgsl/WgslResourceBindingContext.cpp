//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/wgsl/WgslResourceBindingContext.h>

#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

WgslResourceBindingContext::WgslResourceBindingContext(size_t uniformBindingLocation) :
    VkResourceBindingContext(uniformBindingLocation)
{
}

void WgslResourceBindingContext::emitDirectives(GenContext&, ShaderStage&)
{
    // WGSL has no #version or #pragma shader_stage directives.
}

string WgslResourceBindingContext::getWgslUniformType(const ShaderPort* port, const Syntax& syntax) const
{
    if (port->getType() == Type::BOOLEAN)
    {
        return "i32";
    }
    return syntax.getTypeName(port->getType());
}

void WgslResourceBindingContext::emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    for (auto uniform : uniforms.getVariableOrder())
    {
        const TypeDesc t = uniform->getType();
        if (t.isClosure() || t == Type::SURFACESHADER || t == Type::MATERIAL ||
            t == Type::DISPLACEMENTSHADER || t == Type::VOLUMESHADER || t == Type::LIGHTSHADER)
            continue;

        if (uniform->getType() == Type::FILENAME)
        {
            const string& name = uniform->getVariable();
            generator.emitLine("@group(0) @binding(" + std::to_string(_hwUniformBindLocation++) + ") var " + name +
                                   "_texture: texture_2d<f32>",
                               stage);
            generator.emitLine("@group(0) @binding(" + std::to_string(_hwUniformBindLocation++) + ") var " + name +
                                   "_sampler: sampler",
                               stage);
        }
        else
        {
            const string typeName = getWgslUniformType(uniform, syntax);
            generator.emitLine("@group(0) @binding(" + std::to_string(_hwUniformBindLocation++) + ") var<uniform> " +
                                   uniform->getVariable() + ": " + typeName,
                               stage);
        }
    }

    generator.emitLineBreak(stage);
}

void WgslResourceBindingContext::emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                                                ShaderStage& stage, const std::string& structInstanceName,
                                                                const std::string& arraySuffix)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    // Emit a WGSL struct for the uniform block members.
    generator.emitLine("struct " + uniforms.getName(), stage, false);
    generator.emitScopeBegin(stage);
    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        const string typeName = getWgslUniformType(uniforms[i], syntax);
        // WGSL struct members are comma-separated (not `;`-terminated like GLSL).
        generator.emitLine(uniforms[i]->getVariable() + ": " + typeName + ",", stage, false);
    }
    generator.emitScopeEnd(stage, false);

    if (!arraySuffix.empty())
    {
        // arraySuffix is e.g. "[4]" — extract count for WGSL array<> syntax.
        const string count = arraySuffix.substr(1, arraySuffix.size() - 2);
        generator.emitLine("@group(0) @binding(" + std::to_string(_hwUniformBindLocation++) + ") var<uniform> " +
                               structInstanceName + ": array<" + uniforms.getName() + ", " + count + ">",
                           stage);
    }
    else
    {
        generator.emitLine("@group(0) @binding(" + std::to_string(_hwUniformBindLocation++) + ") var<uniform> " +
                               structInstanceName + ": " + uniforms.getName(),
                           stage);
    }
    generator.emitLineBreak(stage);
}

MATERIALX_NAMESPACE_END
