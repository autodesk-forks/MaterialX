//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

namespace
{
    static const string VIEW_POSITON_SEMANTIC = "ViewPosition";
}

const string MayaGlslPluginShaderGenerator::TARGET = "ogsfx_mayaglslplugin";

ShaderGeneratorPtr MayaGlslPluginShaderGenerator::create()
{
    return std::make_shared<MayaGlslPluginShaderGenerator>();
}

ShaderPtr MayaGlslPluginShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = OgsFxShaderGenerator::createShader(name, element, context);

    // Update view position semantic to match Maya's semantics.
    for (size_t i = 0; i < shader->numStages(); ++i)
    {
        ShaderStage& stage = shader->getStage(i);
        for (const auto& uniformMap : stage.getUniformBlocks())
        {
            ShaderPort* v = uniformMap.second->find(HW::T_VIEW_POSITION);
            if (v)
            {
                v->setSemantic(VIEW_POSITON_SEMANTIC);
            }
        }
    }

    return shader;
}

void MayaGlslPluginShaderGenerator::getTechniqueParams(const Shader& shader, string& params) const
{
    if (shader.hasAttribute(HW::ATTR_TRANSPARENT))
    {
        params = "string transparency = \"transparent\";";
    }
}

} // namespace MaterialX
