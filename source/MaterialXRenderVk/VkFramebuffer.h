//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKFRAMEBUFFER_H
#define MATERIALX_VKFRAMEBUFFER_H

/// @file
/// Vulkan framebuffer handling

#include <MaterialXRenderVk/Export.h>

#include <MaterialXRender/ImageHandler.h>

#ifdef _USE_VULKAN_CPP
#include <MaterialXRenderVk/VkDevice.h>
#endif

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a VkFrameBuffer
using VkFrameBufferPtr = std::shared_ptr<class VkFrameBuffer>;

/// @class VkFrameBuffer
/// Wrapper for an Vulkan framebuffer
class MX_RENDERVK_API VkFrameBuffer
{
  public:
    /// Create a new framebuffer
    static VkFrameBufferPtr create(VulkanDevicePtr vulkanDevice,
                                   unsigned int width, 
                                   unsigned int height, 
                                   unsigned int channelCount,
                                   Image::BaseType baseType);

    /// Destructor
    virtual ~VkFrameBuffer();

    void cleanup();

    void initRenderPassAndFramebuffers();

    /// Resize the framebuffer
    void resize(unsigned int width, unsigned int height);

    /// PSO helper functions
    vk::RenderPass createRenderPass(vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
                                    vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);

    std::vector<vk::Framebuffer> createFramebuffers(vk::RenderPass renderPass);


    /// Return the framebuffer width
    unsigned int getWidth() const { return _width; }

    /// Return the framebuffer height
    unsigned int getHeight() const { return _height; }

    vk::RenderPass getRenderPass() const
    {
        return _renderPass;
    }

    const std::vector<vk::Framebuffer>& getFramebuffers() const
    {
        return _framebuffers;
    }

    vk::ImageView getDepthBufferImageView() const
    {
        return _depthBufferAttachment._imageView;
    }
    /// Bind the framebuffer for rendering.
    // void bind(MTLRenderPassDescriptor* renderpassDesc);
    void bind();

    /// Unbind the frame buffer after rendering.
    void unbind();

    /// Return our color texture handle.
    unsigned int getColorTexture() const
    {
        return _colorTexture;
    }


    unsigned int getDepthTexture() const
    {
        return _depthTexture;
    }

    ImagePtr getColorImage(ImagePtr image = nullptr);

  protected:
    VkFrameBuffer(VulkanDevicePtr vulkanDevice, unsigned int width, unsigned int height, 
        unsigned int channelCount, Image::BaseType baseType);

  protected:
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    Image::BaseType _baseType;
    bool _encodeSrgb;

    vk::RenderPass _renderPass;
    std::vector<vk::Framebuffer> _framebuffers;

    unsigned int _colorTexture;
    unsigned int _depthTexture;
    VulkanDevicePtr _vulkanDevice = nullptr;
    vk::Format _colorFormat;
    vk::Format _depthFormat;
    struct FrameBufferAttachment
    {
        vk::Image _image = nullptr;
        vk::DeviceMemory _deviceMemory = nullptr;
        vk::ImageView _imageView = nullptr;
    };

    FrameBufferAttachment _colorBufferAttachment;
    FrameBufferAttachment _depthBufferAttachment;

    bool _colorTextureOwned = false;
};

MATERIALX_NAMESPACE_END
#endif
