//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkRenderer.h>

#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN

VkRendererPtr VkRenderer::create(unsigned int width, unsigned int height, Image::BaseType baseType)
{
    return VkRendererPtr(new VkRenderer(width, height, baseType));
}

VkRenderer::VkRenderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
    ShaderRenderer(width, height, baseType, MatrixConvention::Vulkan)
{
}

VkRenderer::~VkRenderer()
{
}

void VkRenderer::initialize(RenderContextHandle renderContextHandle)
{
    if (_initialized)
        return;

    if (renderContextHandle)
    {
        // Adopt an externally owned device passed in as a VkContext*.
        VkContext* external = static_cast<VkContext*>(renderContextHandle);
        _context = VkContext::createShared(external->getInstance(),
                                           external->getPhysicalDevice(),
                                           external->getDevice(),
                                           external->getQueueFamilyIndex(),
                                           external->getQueue());
    }
    else
    {
        _context = VkContext::create();
    }

    // Phase 3 will create the framebuffer here.
    _initialized = true;
}

ImageHandlerPtr VkRenderer::createImageHandler(ImageLoaderPtr imageLoader)
{
    // Phase 3: return VkTextureHandler::create(_context, imageLoader).
    (void)imageLoader;
    return nullptr;
}

void VkRenderer::createProgram(ShaderPtr shader)
{
    if (!_program)
    {
        _program = VkProgram::create(_context);
    }
    _program->setStages(shader);
    // Phase 3: _program->build(_framebuffer).
}

void VkRenderer::createProgram(const StageMap& stages)
{
    if (!_program)
    {
        _program = VkProgram::create(_context);
    }
    _program->clearStages();
    for (const auto& it : stages)
    {
        _program->addStage(it.first, it.second);
    }
    // Phase 3: _program->build(_framebuffer).
}

void VkRenderer::validateInputs()
{
    if (!_program)
    {
        throw ExceptionRenderError("No program to validate");
    }
    _program->getUniformsList();
    _program->getAttributesList();
}

void VkRenderer::updateUniform(const string& name, ConstValuePtr value)
{
    if (!_program)
    {
        throw ExceptionRenderError("No program to update");
    }
    _program->bindUniform(name, value);
}

void VkRenderer::setSize(unsigned int width, unsigned int height)
{
    _width = width;
    _height = height;
    // Phase 3: resize framebuffer.
}

void VkRenderer::render()
{
    // Phase 3.
}

void VkRenderer::renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax)
{
    (void)uvMin;
    (void)uvMax;
    // Phase 3.
}

ImagePtr VkRenderer::captureImage(ImagePtr image)
{
    (void)image;
    // Phase 3.
    return nullptr;
}

void VkRenderer::createFrameBuffer(bool encodeSrgb)
{
    (void)encodeSrgb;
    // Phase 3.
}

MATERIALX_NAMESPACE_END
