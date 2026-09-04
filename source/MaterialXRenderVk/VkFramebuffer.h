//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKFRAMEBUFFER_H
#define MATERIALX_VKFRAMEBUFFER_H

/// @file
/// Vulkan framebuffer handling

#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkContext.h>

#include <MaterialXRender/ImageHandler.h>

#include <vulkan/vulkan.h>

MATERIALX_NAMESPACE_BEGIN

class VkFramebuffer;

/// Shared pointer to a VkFramebuffer
using VkFramebufferPtr = std::shared_ptr<VkFramebuffer>;

/// @class VkFramebuffer
/// Wrapper for a Vulkan offscreen framebuffer (color + depth attachments).
/// Headless: no surface, no swapchain. The color attachment is created with
/// eColorAttachment | eTransferSrc | eSampled usage flags.
class MX_RENDERVK_API VkFramebuffer
{
  public:
    /// Create a new framebuffer.
    static VkFramebufferPtr create(VkContextPtr context, unsigned int width, unsigned int height,
                                   unsigned int channelCount, Image::BaseType baseType);

    /// Destructor
    virtual ~VkFramebuffer();

    /// Return the width of the framebuffer.
    unsigned int getWidth() const { return _width; }

    /// Return the height of the framebuffer.
    unsigned int getHeight() const { return _height; }

    /// Set the encode sRGB flag. Recreates the render pass / color attachment
    /// format if the encoding changes.
    void setEncodeSrgb(bool encode);

    /// Return the encode sRGB flag.
    bool getEncodeSrgb() const { return _encodeSrgb; }

    /// Resize the framebuffer.
    void resize(unsigned int width, unsigned int height);

    /// Begin the render pass. Unlike GLFramebuffer::bind(), Vulkan needs the
    /// command buffer, so the signature necessarily differs — same as
    /// MetalFramebuffer::bind(MTLRenderPassDescriptor*).
    void bind(VkCommandBuffer cmd, const Color3& clearColor);

    /// End the render pass.
    void unbind(VkCommandBuffer cmd);

    VkRenderPass  getRenderPass()     const { return _renderPass; }
    ::VkFramebuffer getFramebuffer()  const { return _framebuffer; }
    VkFormat      getColorFormat()    const { return _colorFormat; }
    VkFormat      getDepthFormat()     const { return _depthFormat; }
    VkImage       getColorImage()      const { return _colorImage; }
    VkImageView   getColorImageView()  const { return _colorView; }

    /// Read the color attachment back into an mx::Image via a host-visible
    /// staging buffer. Row order matches GLFramebuffer::getColorImage() —
    /// see Background C in the plan (Vulkan buffer row 0 <-> NDC y = -1).
    ImagePtr getColorImage(ImagePtr image = nullptr);

  protected:
    VkFramebuffer(VkContextPtr context, unsigned int width, unsigned int height,
                  unsigned int channelCount, Image::BaseType baseType);

    void createAttachments();
    void destroyAttachments();
    void createRenderPass();

    VkContextPtr _context;
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    Image::BaseType _baseType;
    bool _encodeSrgb;

    VkFormat _colorFormat;
    VkFormat _depthFormat;

    VkImage _colorImage = VK_NULL_HANDLE;
    VkDeviceMemory _colorMemory = VK_NULL_HANDLE;
    VkImageView _colorView = VK_NULL_HANDLE;

    VkImage _depthImage = VK_NULL_HANDLE;
    VkDeviceMemory _depthMemory = VK_NULL_HANDLE;
    VkImageView _depthView = VK_NULL_HANDLE;

    VkRenderPass _renderPass = VK_NULL_HANDLE;
    ::VkFramebuffer _framebuffer = VK_NULL_HANDLE;
};

MATERIALX_NAMESPACE_END

#endif
