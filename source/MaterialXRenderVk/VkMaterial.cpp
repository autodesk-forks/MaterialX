//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkMaterial.h>
#include <MaterialXRenderVk/VkTextureHandler.h>

#include <MaterialXRender/Util.h>

#include <MaterialXGenHw/HwConstants.h>

#include <MaterialXFormat/Util.h>

MATERIALX_NAMESPACE_BEGIN

const std::string DISTANCE_UNIT_TARGET_NAME = "u_distanceUnitTarget";

bool VkMaterial::loadSource(const FilePath& vertexShaderFile, const FilePath& pixelShaderFile, bool hasTransparency)
{
    _hasTransparency = hasTransparency;

    std::string vertexShader = readFile(vertexShaderFile);
    if (vertexShader.empty())
        return false;

    std::string pixelShader = readFile(pixelShaderFile);
    if (pixelShader.empty())
        return false;

    _vkProgram = VkProgram::create(nullptr);
    _vkProgram->addStage(Stage::VERTEX, vertexShader);
    _vkProgram->addStage(Stage::PIXEL, pixelShader);

    return true;
}

void VkMaterial::clearShader()
{
    _hwShader = nullptr;
    _vkProgram = nullptr;
}

bool VkMaterial::generateShader(GenContext& context)
{
    if (!_elem)
        return false;

    _hasTransparency = isTransparentSurface(_elem, context.getShaderGenerator().getTarget());

    GenContext materialContext = context;
    materialContext.getOptions().hwTransparency = _hasTransparency;

    clearShader();

    _hwShader = createShader("Shader", materialContext, _elem);
    if (!_hwShader)
        return false;

    _vkProgram = VkProgram::create(nullptr);
    _vkProgram->setStages(_hwShader);

    return true;
}

bool VkMaterial::generateShader(ShaderPtr hwShader)
{
    _hwShader = hwShader;
    _vkProgram = VkProgram::create(nullptr);
    _vkProgram->setStages(hwShader);
    return true;
}

bool VkMaterial::bindShader() const
{
    // Validation / lazy-build step. The actual Vulkan build (SPIR-V compile,
    // pipeline creation) requires a VkContext + VkFramebuffer, which the
    // renderer sets up. Here we just confirm the program exists and has been
    // built by the renderer. The renderer's createProgram() calls build().
    if (!_vkProgram)
        return false;
    return _vkProgram->hasBuiltData();
}

void VkMaterial::bindMesh(MeshPtr mesh)
{
    if (!mesh || !bindShader())
        return;

    if (mesh != _boundMesh)
    {
        _vkProgram->unbindGeometry();
    }
    _vkProgram->bindMesh(mesh);
    _boundMesh = mesh;
}

bool VkMaterial::bindPartition(MeshPartitionPtr part) const
{
    if (!bindShader())
        return false;
    _vkProgram->bindPartition(part);
    return true;
}

void VkMaterial::bindViewInformation(CameraPtr camera)
{
    if (!_vkProgram)
        return;
    _vkProgram->bindViewInformation(camera);
}

void VkMaterial::unbindImages(ImageHandlerPtr imageHandler)
{
    for (ImagePtr image : _boundImages)
    {
        imageHandler->unbindImage(image);
    }
}

void VkMaterial::bindImages(ImageHandlerPtr imageHandler, const FileSearchPath& searchPath, bool enableMipmaps)
{
    if (!_vkProgram)
        return;

    _boundImages.clear();

    const VariableBlock* publicUniforms = getPublicUniforms();
    if (!publicUniforms)
        return;

    for (const auto& uniform : publicUniforms->getVariableOrder())
    {
        if (uniform->getType() != Type::FILENAME)
            continue;

        const std::string& uniformVariable = uniform->getVariable();
        std::string filename;
        if (uniform->getValue())
        {
            filename = searchPath.find(uniform->getValue()->getValueString());
        }

        ImageSamplingProperties samplingProperties;
        samplingProperties.setProperties(uniformVariable, *publicUniforms);
        samplingProperties.enableMipmaps = enableMipmaps;

        ImagePtr image = bindImage(filename, uniformVariable, imageHandler, samplingProperties);
        if (image)
        {
            _boundImages.push_back(image);
        }
    }
}

ImagePtr VkMaterial::bindImage(const FilePath& filePath, const std::string& uniformName,
                               ImageHandlerPtr imageHandler, const ImageSamplingProperties& samplingProperties)
{
    if (!_vkProgram)
        return nullptr;

    StringResolverPtr resolver = StringResolver::create();
    if (!getUdim().empty())
    {
        resolver->setUdimString(getUdim());
    }
    imageHandler->setFilenameResolver(resolver);

    ImagePtr image = imageHandler->acquireImage(filePath, samplingProperties.defaultColor);
    if (!image)
        return nullptr;

    if (imageHandler->bindImage(image, samplingProperties))
    {
        // In Vulkan, the descriptor write happens in VkProgram::bindTextures()
        // using the VkTextureHandler's getDescriptorInfo(). The material's
        // bindImage just ensures the resource exists.
        return image;
    }
    return nullptr;
}

void VkMaterial::bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler, const ShadowState& shadowState)
{
    if (!_vkProgram)
        return;

    _vkProgram->bindLighting(lightHandler, imageHandler);

    // Shadow map / ambient occlusion binding (Phase 6 viewer concern; the
    // baker doesn't use these). Stubbed for now.
    (void)shadowState;
}

void VkMaterial::drawPartition(MeshPartitionPtr part) const
{
    // The command buffer is set by the renderer before the pass. The material's
    // drawPartition delegates to the program, but the program needs a command
    // buffer. For the baker (which uses VkRenderer directly), the renderer
    // handles the draw. This is a no-op stub for the ShaderMaterial contract.
    (void)part;
}

void VkMaterial::unbindGeometry()
{
    if (!_boundMesh)
        return;

    if (bindShader())
    {
        _vkProgram->unbindGeometry();
    }
    _boundMesh = nullptr;
}

VariableBlock* VkMaterial::getPublicUniforms() const
{
    if (!_hwShader)
        return nullptr;

    ShaderStage& stage = _hwShader->getStage(Stage::PIXEL);
    VariableBlock& block = stage.getUniformBlock(HW::PUBLIC_UNIFORMS);
    return &block;
}

ShaderPort* VkMaterial::findUniform(const std::string& path) const
{
    ShaderPort* shaderPort = nullptr;
    VariableBlock* publicUniforms = getPublicUniforms();
    if (publicUniforms)
    {
        shaderPort = publicUniforms->find([path](ShaderPort* port)
        {
            return (port && stringEndsWith(port->getPath(), path));
        });
        if (!shaderPort)
        {
            shaderPort = publicUniforms->find([path](ShaderPort* port)
            {
                return (port && stringEndsWith(path, port->getName()));
            });
        }

        if (shaderPort && !_vkProgram->getUniformsList().count(shaderPort->getVariable()))
        {
            shaderPort = nullptr;
        }
    }
    return shaderPort;
}

void VkMaterial::modifyUniform(const std::string& path, ConstValuePtr value, std::string valueString)
{
    if (!bindShader())
        return;

    ShaderPort* uniform = findUniform(path);
    if (!uniform)
        return;

    _vkProgram->bindUniform(uniform->getVariable(), value);

    if (valueString.empty())
    {
        valueString = value->getValueString();
    }
    uniform->setValue(uniform->getType().createValueFromStrings(valueString));
    if (_doc)
    {
        ElementPtr element = _doc->getDescendant(uniform->getPath());
        if (element)
        {
            element->setAttribute("value", valueString);
        }
    }
}

MATERIALX_NAMESPACE_END
