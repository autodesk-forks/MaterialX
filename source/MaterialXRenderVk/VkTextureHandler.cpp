//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkTextureHandler.h>
#include <MaterialXRenderVk/VkUtil.h>

#include <MaterialXRender/Image.h>

#include <cstring>

MATERIALX_NAMESPACE_BEGIN

VkSamplerAddressMode VkTextureHandler::mapAddressModeToVk(ImageSamplingProperties::AddressMode addressModeEnum)
{
    switch (addressModeEnum)
    {
        case ImageSamplingProperties::AddressMode::PERIODIC: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case ImageSamplingProperties::AddressMode::CLAMP:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ImageSamplingProperties::AddressMode::MIRROR:   return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkFilter VkTextureHandler::mapFilterTypeToVk(ImageSamplingProperties::FilterType filterTypeEnum, bool /*mip*/)
{
    switch (filterTypeEnum)
    {
        case ImageSamplingProperties::FilterType::LINEAR: return VK_FILTER_LINEAR;
        case ImageSamplingProperties::FilterType::CLOSEST: return VK_FILTER_NEAREST;
        default: return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode VkTextureHandler::mapMipModeToVk(ImageSamplingProperties::FilterType filterTypeEnum)
{
    switch (filterTypeEnum)
    {
        case ImageSamplingProperties::FilterType::LINEAR: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case ImageSamplingProperties::FilterType::CLOSEST: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        default: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

VkFormat VkTextureHandler::mapTextureFormatToVk(Image::BaseType baseType, unsigned int channelCount, bool srgb)
{
    if (srgb && baseType == Image::BaseType::UINT8)
    {
        switch (channelCount)
        {
            case 1: return VK_FORMAT_R8_SRGB;
            case 2: return VK_FORMAT_R8G8_SRGB;
            default: return VK_FORMAT_R8G8B8A8_SRGB;
        }
    }
    if (baseType == Image::BaseType::UINT8)
    {
        switch (channelCount)
        {
            case 1: return VK_FORMAT_R8_UNORM;
            case 2: return VK_FORMAT_R8G8_UNORM;
            default: return VK_FORMAT_R8G8B8A8_UNORM; // expand 3->4
        }
    }
    if (baseType == Image::BaseType::HALF)
    {
        switch (channelCount)
        {
            case 1: return VK_FORMAT_R16_SFLOAT;
            case 2: return VK_FORMAT_R16G16_SFLOAT;
            default: return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
    }
    switch (channelCount)
    {
        case 1: return VK_FORMAT_R32_SFLOAT;
        case 2: return VK_FORMAT_R32G32_SFLOAT;
        default: return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
}

VkTextureHandler::VkTextureHandler(VkContextPtr context, ImageLoaderPtr imageLoader) :
    ImageHandler(imageLoader),
    _context(context)
{
}

bool VkTextureHandler::createRenderResources(ImagePtr image, bool generateMipMaps, bool useAsRenderTarget)
{
    if (!image || !_context)
        return false;

    unsigned int resourceId = image->getResourceId();
    if (_textures.find(resourceId) != _textures.end())
        return true; // already created

    VkDevice device = _context->getDevice();
    unsigned int width = image->getWidth();
    unsigned int height = image->getHeight();
    unsigned int channelCount = image->getChannelCount();
    Image::BaseType baseType = image->getBaseType();

    // Trap #6: expand 3-channel images to RGBA on upload. VK_FORMAT_R8G8B8_UNORM
    // and R32G32B32_SFLOAT are optional and usually unsupported for sampled images.
    bool expandToRGBA = (channelCount == 3);
    unsigned int uploadChannels = expandToRGBA ? 4 : channelCount;

    VkFormat format = mapTextureFormatToVk(baseType, uploadChannels, false);
    uint32_t mipLevels = generateMipMaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;
    if (mipLevels < 1) mipLevels = 1;

    // Create the image.
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                      (useAsRenderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0);
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkTextureResource res;
    VK_CHECK(vkCreateImage(device, &imageInfo, nullptr, &res.image), "Failed to create texture image");
    res.format = format;
    res.mipLevels = mipLevels;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, res.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _context->findMemoryType(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &res.memory), "Failed to allocate texture memory");
    vkBindImageMemory(device, res.image, res.memory, 0);

    // Upload the pixel data via a staging buffer.
    size_t baseStride = image->getBaseStride();
    size_t srcRowBytes = width * channelCount * baseStride;
    size_t dstRowBytes = width * uploadChannels * baseStride;
    VkDeviceSize stagingSize = dstRowBytes * height;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = stagingSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);

    VkMemoryRequirements stagingReqs;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &stagingReqs);
    VkMemoryAllocateInfo stagingAlloc{};
    stagingAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    stagingAlloc.allocationSize = stagingReqs.size;
    stagingAlloc.memoryTypeIndex = _context->findMemoryType(
        stagingReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(device, &stagingAlloc, nullptr, &stagingMemory);
    vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

    void* data = nullptr;
    vkMapMemory(device, stagingMemory, 0, stagingSize, 0, &data);
    unsigned char* src = static_cast<unsigned char*>(image->getResourceBuffer());
    unsigned char* dst = static_cast<unsigned char*>(data);
    if (expandToRGBA)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            for (unsigned int x = 0; x < width; x++)
            {
                unsigned char* srcPixel = src + (y * width + x) * channelCount * baseStride;
                unsigned char* dstPixel = dst + (y * width + x) * uploadChannels * baseStride;
                std::memcpy(dstPixel, srcPixel, channelCount * baseStride);
                // Set alpha to full.
                if (baseType == Image::BaseType::UINT8)
                    dstPixel[3] = 255;
                else if (baseType == Image::BaseType::FLOAT)
                {
                    float one = 1.0f;
                    std::memcpy(dstPixel + 3 * baseStride, &one, baseStride);
                }
            }
        }
    }
    else
    {
        std::memcpy(dst, src, static_cast<size_t>(stagingSize));
    }
    vkUnmapMemory(device, stagingMemory);

    // Transition + copy.
    transitionImageLayout(_context, res.image, format,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

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
        region.imageExtent = { width, height, 1 };
        vkCmdCopyBufferToImage(cmd, stagingBuffer, res.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        if (generateMipMaps && mipLevels > 1)
        {
            generateMipmaps(cmd, res.image, mipLevels, width, height);
        }
    });

    // Transition to shader-read.
    transitionImageLayout(_context, res.image, format,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    // Create the image view.
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = res.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &res.view), "Failed to create texture image view");

    _textures[resourceId] = res;
    return true;
}

void VkTextureHandler::generateMipmaps(VkCommandBuffer cmd, VkImage image, uint32_t mipLevels,
                                      unsigned int width, unsigned int height)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
}

bool VkTextureHandler::bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties)
{
    if (!image)
        return false;

    // Ensure render resources exist (created with mipmaps if requested).
    if (!createRenderResources(image, samplingProperties.enableMipmaps, false))
        return false;

    return true;
}

bool VkTextureHandler::unbindImage(ImagePtr image)
{
    if (!image)
        return false;
    releaseRenderResources(image);
    return true;
}

void VkTextureHandler::releaseRenderResources(ImagePtr image)
{
    VkDevice device = _context ? _context->getDevice() : VK_NULL_HANDLE;
    if (!device)
        return;

    if (image)
    {
        unsigned int resourceId = image->getResourceId();
        auto it = _textures.find(resourceId);
        if (it != _textures.end())
        {
            if (it->second.view != VK_NULL_HANDLE)
                vkDestroyImageView(device, it->second.view, nullptr);
            if (it->second.image != VK_NULL_HANDLE)
                vkDestroyImage(device, it->second.image, nullptr);
            if (it->second.memory != VK_NULL_HANDLE)
                vkFreeMemory(device, it->second.memory, nullptr);
            _textures.erase(it);
        }
    }
    else
    {
        for (auto& kv : _textures)
        {
            if (kv.second.view != VK_NULL_HANDLE)
                vkDestroyImageView(device, kv.second.view, nullptr);
            if (kv.second.image != VK_NULL_HANDLE)
                vkDestroyImage(device, kv.second.image, nullptr);
            if (kv.second.memory != VK_NULL_HANDLE)
                vkFreeMemory(device, kv.second.memory, nullptr);
        }
        _textures.clear();
        for (auto& kv : _samplers)
        {
            if (kv.second != VK_NULL_HANDLE)
                vkDestroySampler(device, kv.second, nullptr);
        }
        _samplers.clear();
    }
}

VkSampler VkTextureHandler::getSampler(const ImageSamplingProperties& samplingProperties)
{
    auto it = _samplers.find(samplingProperties);
    if (it != _samplers.end())
        return it->second;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = mapFilterTypeToVk(samplingProperties.filterType);
    samplerInfo.minFilter = mapFilterTypeToVk(samplingProperties.filterType);
    samplerInfo.mipmapMode = mapMipModeToVk(samplingProperties.filterType);
    samplerInfo.addressModeU = mapAddressModeToVk(samplingProperties.uaddressMode);
    samplerInfo.addressModeV = mapAddressModeToVk(samplingProperties.vaddressMode);
    samplerInfo.addressModeW = mapAddressModeToVk(samplingProperties.uaddressMode);
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler = VK_NULL_HANDLE;
    VK_CHECK(vkCreateSampler(_context->getDevice(), &samplerInfo, nullptr, &sampler),
             "Failed to create Vulkan sampler");
    _samplers[samplingProperties] = sampler;
    return sampler;
}

VkDescriptorImageInfo VkTextureHandler::getDescriptorInfo(ImagePtr image, const ImageSamplingProperties& samplingProperties)
{
    VkDescriptorImageInfo info{};
    if (!image)
        return info;

    unsigned int resourceId = image->getResourceId();
    auto it = _textures.find(resourceId);
    if (it == _textures.end())
        return info;

    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.imageView = it->second.view;
    info.sampler = getSampler(samplingProperties);
    return info;
}

MATERIALX_NAMESPACE_END
