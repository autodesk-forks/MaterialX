//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXRenderVk/VkFrameBuffer.h>

MATERIALX_NAMESPACE_BEGIN

//
// VkFrameBuffer methods
//

VkFrameBufferPtr VkFrameBuffer::create(VulkanDevicePtr vulkanDevice,
    unsigned int width, unsigned int height,
    unsigned channelCount,
    Image::BaseType baseType)
{
    return VkFrameBufferPtr(new VkFrameBuffer(vulkanDevice, width, height, channelCount, baseType));
}

VkFrameBuffer::VkFrameBuffer(VulkanDevicePtr vulkanDevice,
    unsigned int width, unsigned int height,
    unsigned int channelCount,
    Image::BaseType baseType) :
    _width(0),
    _height(0),
    _channelCount(channelCount),
    _baseType(baseType),
    //_encodeSrgb(encodeSrgb),
    _vulkanDevice(vulkanDevice),
    //_colorTexture(colorTexture),
    _depthTexture(0)
{
    StringVec errors;
    const string errorType("Vulkan target creation failure.");

    resize(width, height);
}

VkFrameBuffer::~VkFrameBuffer()
{
    cleanup();
}

void VkFrameBuffer::cleanup()
{
    vk::Device device = _vulkanDevice->GetDevice();

    if (_colorBufferAttachment._imageView)
    {
        device.destroyImageView(_colorBufferAttachment._imageView);
        _colorBufferAttachment._imageView = nullptr;
    }

    if (_colorBufferAttachment._image)
    {
        device.destroyImage(_colorBufferAttachment._image);
        _colorBufferAttachment._image = nullptr;
    }

    if (_colorBufferAttachment._deviceMemory)
    {
        device.freeMemory(_colorBufferAttachment._deviceMemory);
        _colorBufferAttachment._deviceMemory = nullptr;
    }

    if (_depthBufferAttachment._imageView)
    {
        device.destroyImageView(_depthBufferAttachment._imageView);
        _depthBufferAttachment._imageView = nullptr;
    }

    if (_depthBufferAttachment._image)
    {
        device.destroyImage(_depthBufferAttachment._image);
        _depthBufferAttachment._image = nullptr;
    }

    if (_depthBufferAttachment._deviceMemory)
    {
        device.freeMemory(_depthBufferAttachment._deviceMemory);
        _depthBufferAttachment._deviceMemory = nullptr;
    }

    for (auto& framebuffer : _framebuffers)
    {
        device.destroyFramebuffer(framebuffer);
    }
    _framebuffers.clear();

    if (_renderPass)
    {
        device.destroyRenderPass(_renderPass);
        _renderPass = nullptr;
    }
}

void VkFrameBuffer::initRenderPassAndFramebuffers()
{
    _renderPass = _vulkanDevice->createRenderPass(_colorFormat,
                                                  _depthFormat,
                                                  vk::AttachmentLoadOp::eClear,
                                                  vk::ImageLayout::ePresentSrcKHR);

    _framebuffers = createFramebuffers(_renderPass);
}

void VkFrameBuffer::resize(unsigned int width, unsigned int height)
{
    cleanup();

    if (width * height <= 0)
    {
        return;
    }

    //NOTES:
    //  VkFramebuffer + VkRenderPass defines the render target.
    //  Renderpass defines which attachment will be written with colors.
    //  VkFramebuffer defines which VkImageView is to be which attachment.
    //  VkImageView defines which part of VkImage to use.
    //  VkImage defines which VkDeviceMemory is used and a format of the texel.
    
    vk::Device vkDeviceHandle = _vulkanDevice->GetDevice(); 
    // Create Vulkan Image for ColorBuffer
    _colorFormat = vk::Format::eB8G8R8A8Unorm;
    _colorBufferAttachment._image = vkDeviceHandle.createImage(vk::ImageCreateInfo(vk::ImageCreateFlags(),
                                                                                 vk::ImageType::e2D,
                                                                                 _colorFormat,
                                                                                 vk::Extent3D(width, height, 1),
                                                                                 1,
                                                                                 1,
                                                                                 vk::SampleCountFlagBits::e1,
                                                                                 vk::ImageTiling::eOptimal,
                                                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                                                 vk::ImageUsageFlagBits::eTransferSrc));
    // Allocate memory for ColorBuffer
    vk::MemoryRequirements colormemoryRequirements = vkDeviceHandle.getImageMemoryRequirements(_colorBufferAttachment._image);
    uint32_t memoryTypeIndex = _vulkanDevice->FindMemoryType(colormemoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _colorBufferAttachment._deviceMemory = vkDeviceHandle.allocateMemory(vk::MemoryAllocateInfo(colormemoryRequirements.size, memoryTypeIndex));
    // Bind memory to image
    vkDeviceHandle.bindImageMemory(_colorBufferAttachment._image, _colorBufferAttachment._deviceMemory, 0);
    // Create View
    _colorBufferAttachment._imageView = vkDeviceHandle.createImageView(vk::ImageViewCreateInfo(
                                                                                vk::ImageViewCreateFlags(), 
                                                                                _colorBufferAttachment._image, 
                                                                                vk::ImageViewType::e2D, 
                                                                                _colorFormat,
                                                                                {},
                                                                                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }));


    
    _depthFormat = vk::Format::eD16Unorm;
    vk::FormatProperties formatProperties = _vulkanDevice->GetPhysicalDevice().getFormatProperties(_depthFormat);
    vk::ImageTiling tiling;
    if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
    {
        tiling = vk::ImageTiling::eLinear;
    }
    else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
    {
        tiling = vk::ImageTiling::eOptimal;
    }
    else
    {
        throw std::runtime_error("DepthStencilAttachment is not supported for eD32Sfloat depth format.");
    }

    // Create Image
    vk::ImageCreateInfo depthimageCreateInfo(vk::ImageCreateFlags(),
                                        vk::ImageType::e2D,
                                        _depthFormat,
                                        vk::Extent3D(width, height, 1),
                                        1,
                                        1,
                                        vk::SampleCountFlagBits::e1,
                                        tiling,
                                        vk::ImageUsageFlagBits::eDepthStencilAttachment);
    _depthBufferAttachment._image = vkDeviceHandle.createImage(depthimageCreateInfo);
    
    // Allocate
    vk::MemoryRequirements depthMemoryRequirements = vkDeviceHandle.getImageMemoryRequirements(_depthBufferAttachment._image);
    uint32_t depthmemoryTypeIndex = _vulkanDevice->FindMemoryType(depthMemoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _depthBufferAttachment._deviceMemory = vkDeviceHandle.allocateMemory(vk::MemoryAllocateInfo(depthMemoryRequirements.size, depthmemoryTypeIndex));
    // Bind image to memory 
    vkDeviceHandle.bindImageMemory(_depthBufferAttachment._image, _depthBufferAttachment._deviceMemory, 0);
    // Create View
    _depthBufferAttachment._imageView = vkDeviceHandle.createImageView(vk::ImageViewCreateInfo(
        vk::ImageViewCreateFlags(), _depthBufferAttachment._image, vk::ImageViewType::e2D, _depthFormat, {}, { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }));
    
    _width = width;
    _height = height;

    initRenderPassAndFramebuffers();

}

vk::RenderPass VkFrameBuffer::createRenderPass(vk::AttachmentLoadOp loadOp,
                                               vk::ImageLayout colorFinalLayout)
{
    return _vulkanDevice->createRenderPass(_colorFormat, _depthFormat, loadOp, colorFinalLayout);
}

std::vector<vk::Framebuffer> VkFrameBuffer::createFramebuffers(vk::RenderPass renderPass)
{
    const auto& swapChainImageViews = _vulkanDevice->getImageViews(); // Get all swapchain image views

    std::vector<vk::Framebuffer> framebuffers;
    framebuffers.reserve(swapChainImageViews.size());

    for (const auto& swapchainImageView : swapChainImageViews)
    {
        std::array<vk::ImageView, 2> attachments = {
            swapchainImageView,               
            _depthBufferAttachment._imageView 
        };

        vk::FramebufferCreateInfo framebufferInfo(
            {},
            renderPass,
            static_cast<uint32_t>(attachments.size()),
            attachments.data(),
            _width,
            _height,
            1);

        framebuffers.push_back(_vulkanDevice->GetDevice().createFramebuffer(framebufferInfo));
    }
    return framebuffers;
}

ImagePtr VkFrameBuffer::getColorImage(ImagePtr image)
{
    if (!image)
    {
        image = Image::create(_width, _height, _channelCount, _baseType);
        image->createResourceBuffer();
    }
    return image;
}

MATERIALX_NAMESPACE_END
