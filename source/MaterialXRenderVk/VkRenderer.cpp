//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkRenderer.h>
#include <MaterialXRenderVk/VkFramebuffer.h>

#include <MaterialXRender/ShaderRenderer.h>
#include <MaterialXRender/StbImageLoader.h>

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

    // Create the offscreen framebuffer (color + depth).
    createFrameBuffer(false);
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
    _program->build(_framebuffer);
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
    _program->build(_framebuffer);
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
    if (_framebuffer)
    {
        _framebuffer->resize(width, height);
    }
}

void VkRenderer::render()
{
    if (!_framebuffer)
        return;

    // Record a one-time command buffer: begin render pass, bind program, draw,
    // end render pass, submit. Even without a program, the render pass clears
    // the framebuffer to _screenColor (clear-only mode).
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    _context->submitOneTimeCommands([&](VkCommandBuffer cb)
    {
        cmd = cb;

        _framebuffer->bind(cb, _screenColor);

        if (_program && _program->hasBuiltData())
        {
            _program->setViewport(_width, _height);
            _program->bind(cb);
            // Phase 3 step 2+: draw the bound mesh partition.
        }

        _framebuffer->unbind(cb);
    });
    (void)cmd;
}

void VkRenderer::renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax)
{
    (void)uvMin;
    (void)uvMax;
    // Phase 4 (TextureBakerVk) uses this; the render path is the same as render().
}

ImagePtr VkRenderer::captureImage(ImagePtr image)
{
    if (!_framebuffer)
        return nullptr;
    return _framebuffer->getColorImage(image);
}

void VkRenderer::createFrameBuffer(bool encodeSrgb)
{
    unsigned int channelCount = (_baseType == Image::BaseType::UINT8) ? 4 : 4;
    _framebuffer = VkFramebuffer::create(_context, _width, _height, channelCount, _baseType);
    _framebuffer->setEncodeSrgb(encodeSrgb);
}

MATERIALX_NAMESPACE_END
