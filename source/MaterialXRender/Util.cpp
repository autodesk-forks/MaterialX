//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

ShaderPtr createShader(const string& shaderName, GenContext& context, ElementPtr elem)
{
    if (!elem)
    {
        return nullptr;
    }

    context.getOptions().hwTransparency = isTransparentSurface(elem, context.getShaderGenerator());
    return context.getShaderGenerator().generate(shaderName, elem, context);
}

ShaderPtr createConstantShader(GenContext& context,
                            DocumentPtr stdLib,
                            const string& shaderName,
                            const Color3& color)
{
    // Construct the constant color nodegraph
    DocumentPtr doc = createDocument();
    doc->importLibrary(stdLib);
    NodeGraphPtr nodeGraph = doc->addNodeGraph();
    NodePtr constant = nodeGraph->addNode("constant");
    constant->setParameterValue("value", color);
    OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);

    // Generate the shader
    ShaderPtr shader = createShader(shaderName, context, output);
    return shader;
}

VariableBlock* getUniformBlock(ShaderPtr shader, const string& stageName, const string& blockName)
{
    try
    {
        ShaderStage& stage = shader->getStage(stageName);
        VariableBlock& block = stage.getUniformBlock(blockName);
        return &block;
    }
    catch (Exception& /*e*/)
    {
    }
    return nullptr;
}

ShaderPort* findUniform(const VariableBlock* block, const std::string& path)
{
    if (!block)
    {
        return nullptr;
    }

    for (auto uniform : block->getVariableOrder())
    {
        if (uniform->getPath() == path)
        {
            return uniform;
        }
    }

    return nullptr;
}


} // namespace MaterialX

