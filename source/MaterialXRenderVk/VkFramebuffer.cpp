//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkFramebuffer.h>
#include <MaterialXRenderVk/VkUtil.h>

#include <MaterialXRender/Image.h>

#include <algorithm>
#include <cstring>

MATERIALX_NAMESPACE_BEGIN

VkFormat colorFormatFor(Image::BaseType baseType, unsigned int channelCount, bool srgb)
{
    if (srgb)
    {
        if (baseType == Image::BaseType::UINT8)
        {
            switch (channelCount)
            {
                case 1: return VK_FORMAT_R8_SRGB;
                case 2: return VK_FORMAT_R8G8_SRGB;
                case 3: return VK_FORMAT_R8G8B8_SRGB;
                default: return VK_FORMAT_R8G8B8A8_SRGB;
            }
        }
        // sRGB only meaningful for 8-bit; fall through to linear for others.
    }
    if (baseType == Image::BaseType::UINT8)
    {
        switch (channelCount)
        {
            case 1: return VK_FORMAT_R8_UNORM;
            case 2: return VK_FORMAT_R8G8_UNORM;
            case 3: return VK_FORMAT_R8G8B8A8_UNORM; // expand 3->4 (R8G8B8 optional)
            default: return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }
    if (baseType == Image::BaseType::HALF)
    {
        switch (channelCount)
        {
            case 1: return VK_FORMAT_R16_SFLOAT;
            case 2: return VK_FORMAT_R16G16_SFLOAT;
            case 3: return VK_FORMAT_R16G16B16A16_SFLOAT;
            default: return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
    }
    // FLOAT and others.
    switch (channelCount)
    {
        case 1: return VK_FORMAT_R32_SFLOAT;
        case 2: return VK_FORMAT_R32G32_SFLOAT;
        case 3: return VK_FORMAT_R32G32B32A32_SFLOAT; // expand 3->4 (R32G32B32 optional)
        default: return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
}

VkFramebufferPtr VkFramebuffer::create(VkContextPtr context, unsigned int width, unsigned int height,
                                       unsigned int channelCount, Image::BaseType baseType)
{
    return VkFramebufferPtr(new VkFramebuffer(context, width, height, channelCount, baseType));
}

VkFramebuffer::VkFramebuffer(VkContextPtr context, unsigned int width, unsigned int height,
                             unsigned int channelCount, Image::BaseType baseType) :
    _context(context),
    _width(width),
    _height(height),
    _channelCount(channelCount),
    _baseType(baseType),
    _encodeSrgb(false)
{
    _colorFormat = colorFormatFor(baseType, channelCount, false);
    _depthFormat = context->findSupportedDepthFormat();
    createAttachments();
    createRenderPass();
}

VkFramebuffer::~VkFramebuffer()
{
    destroyAttachments();
    VkDevice device = _context->getDevice();
    if (_renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, _renderPass, nullptr);
    if (_framebuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(device, _framebuffer, nullptr);
}

void VkFramebuffer::setEncodeSrgb(bool encode)
{
    if (_encodeSrgb == encode)
        return;
    _encodeSrgb = encode;
    VkFormat newFormat = colorFormatFor(_baseType, _channelCount, encode);
    if (newFormat == _colorFormat)
        return;

    // Recreate the color attachment + render pass + framebuffer with the new format.
    VkDevice device = _context->getDevice();
    if (_framebuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, _framebuffer, nullptr);
        _framebuffer = VK_NULL_HANDLE;
    }
    if (_renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, _renderPass, nullptr);
        _renderPass = VK_NULL_HANDLE;
    }
    destroyAttachments();
    _colorFormat = newFormat;
    createAttachments();
    createRenderPass();
}

void VkFramebuffer::resize(unsigned int width, unsigned int height)
{
    if (_width == width && _height == height)
        return;
    _width = width;
    _height = height;

    VkDevice device = _context->getDevice();
    if (_framebuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(device, _framebuffer, nullptr);
        _framebuffer = VK_NULL_HANDLE;
    }
    destroyAttachments();
    createAttachments();
    createRenderPass();
}

void VkFramebuffer::createAttachments()
{
    VkDevice device = _context->getDevice();

    auto createImage = [&](VkFormat format, VkImageUsageFlags usage, VkImage& image,
                           VkDeviceMemory& memory, VkImageView& view) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.extent.width = _width;
        imageInfo.extent.height = _height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(device, &imageInfo, nullptr, &image), "Failed to create framebuffer image");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = _context->findMemoryType(
            memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &memory), "Failed to allocate framebuffer image memory");
        vkBindImageMemory(device, image, memory, 0);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = (format == _depthFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &view), "Failed to create image view");
    };

    // Color: eColorAttachment | eTransferSrc (readback) | eSampled (future interop).
    createImage(_colorFormat,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                _colorImage, _colorMemory, _colorView);
    // Depth.
    createImage(_depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                _depthImage, _depthMemory, _depthView);

    // Transition both to their attachment layouts.
    transitionImageLayout(_context, _colorImage, _colorFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    transitionImageLayout(_context, _depthImage, _depthFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VkFramebuffer::destroyAttachments()
{
    VkDevice device = _context->getDevice();
    if (_colorView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, _colorView, nullptr);
        _colorView = VK_NULL_HANDLE;
    }
    if (_colorImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, _colorImage, nullptr);
        _colorImage = VK_NULL_HANDLE;
    }
    if (_colorMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, _colorMemory, nullptr);
        _colorMemory = VK_NULL_HANDLE;
    }
    if (_depthView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, _depthView, nullptr);
        _depthView = VK_NULL_HANDLE;
    }
    if (_depthImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, _depthImage, nullptr);
        _depthImage = VK_NULL_HANDLE;
    }
    if (_depthMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, _depthMemory, nullptr);
        _depthMemory = VK_NULL_HANDLE;
    }
}

void VkFramebuffer::createRenderPass()
{
    VkAttachmentDescription attachments[2] = {};

    // Color.
    attachments[0].format = _colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth.
    attachments[1].format = _depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(_context->getDevice(), &renderPassInfo, nullptr, &_renderPass),
             "Failed to create Vulkan render pass");

    VkImageView attachmentViews[] = { _colorView, _depthView };
    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = _renderPass;
    fbInfo.attachmentCount = 2;
    fbInfo.pAttachments = attachmentViews;
    fbInfo.width = _width;
    fbInfo.height = _height;
    fbInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(_context->getDevice(), &fbInfo, nullptr, &_framebuffer),
             "Failed to create Vulkan framebuffer");
}

void VkFramebuffer::bind(VkCommandBuffer cmd, const Color3& clearColor)
{
    VkClearValue clearValues[2] = {};
    clearValues[0].color.float32[0] = clearColor[0];
    clearValues[0].color.float32[1] = clearColor[1];
    clearValues[0].color.float32[2] = clearColor[2];
    clearValues[0].color.float32[3] = 1.0f;
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = _renderPass;
    beginInfo.framebuffer = _framebuffer;
    beginInfo.renderArea.offset = { 0, 0 };
    beginInfo.renderArea.extent = { _width, _height };
    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VkFramebuffer::unbind(VkCommandBuffer cmd)
{
    vkCmdEndRenderPass(cmd);
}

ImagePtr VkFramebuffer::getColorImage(ImagePtr image)
{
    if (!image)
    {
        image = Image::create(_width, _height, _channelCount, _baseType);
        image->createResourceBuffer();
    }

    VkDevice device = _context->getDevice();

    // The Vulkan color attachment may have a different channel count than the
    // image (3-channel images are expanded to 4 on attachment creation). Compute
    // the attachment's bytes-per-pixel for the staging buffer.
    size_t baseStride = image->getBaseStride();
    unsigned int attachChannels = _channelCount;
    if (_channelCount == 3)
        attachChannels = 4; // we expanded 3->4 in colorFormatFor()
    size_t attachBytesPerPixel = attachChannels * baseStride;
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(_width) * _height * attachBytesPerPixel;

    // Create a host-visible staging buffer.
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer),
             "Failed to create staging buffer for readback");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _context->findMemoryType(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory),
             "Failed to allocate staging memory for readback");
    vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

    // Transition color image to transfer-src, copy, transition back.
    transitionImageLayout(_context, _colorImage, _colorFormat,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    _context->submitOneTimeCommands([&](VkCommandBuffer cmd)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { _width, _height, 1 };
        vkCmdCopyImageToBuffer(cmd, _colorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               stagingBuffer, 1, &region);
    });

    transitionImageLayout(_context, _colorImage, _colorFormat,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Map and copy into the image resource buffer.
    void* data = nullptr;
    vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);

    // Vulkan's vkCmdCopyImageToBuffer writes row 0 from the top of the attachment.
    // GLFramebuffer::getColorImage reads row 0 from the bottom (glReadPixels).
    // Per Background C, both map buffer row 0 <-> NDC y = -1, so the byte order
    // matches and the tester saves with verticalFlip = true (same as GLSL).
    unsigned char* dst = static_cast<unsigned char*>(image->getResourceBuffer());

    if (attachChannels == _channelCount)
    {
        // Channel counts match — straight copy.
        VkDeviceSize imageBytes = static_cast<VkDeviceSize>(_width) * _height * _channelCount * baseStride;
        VkDeviceSize copySize = (bufferSize < imageBytes) ? bufferSize : imageBytes;
        std::memcpy(dst, data, static_cast<size_t>(copySize));
    }
    else
    {
        // Channel counts differ (3 vs 4) — copy row by row, stripping the extra channel.
        size_t srcRowBytes = _width * attachChannels * baseStride;
        size_t dstRowBytes = _width * _channelCount * baseStride;
        unsigned char* src = static_cast<unsigned char*>(data);
        for (unsigned int y = 0; y < _height; y++)
        {
            for (unsigned int x = 0; x < _width; x++)
            {
                unsigned char* srcPixel = src + (y * _width + x) * attachChannels * baseStride;
                unsigned char* dstPixel = dst + (y * _width + x) * _channelCount * baseStride;
                std::memcpy(dstPixel, srcPixel, _channelCount * baseStride);
            }
        }
        (void)srcRowBytes;
        (void)dstRowBytes;
    }

    vkUnmapMemory(device, stagingMemory);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    return image;
}

MATERIALX_NAMESPACE_END
