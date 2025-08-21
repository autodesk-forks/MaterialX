#include <MaterialXRenderVk/VkTexture.h>
#include <MaterialXRenderVk/VkDevice.h>

MATERIALX_NAMESPACE_BEGIN

VkTexture::VkTexture(VulkanDevicePtr device) :
    _device(device)
{
}

VkTexture::~VkTexture()
{
    cleanup();
}

bool VkTexture::initialize(const ImageDesc& imageDesc, const void* pixelData, bool generateMipMaps, bool sRGB)
{
    cleanup();

    if (!createImage(imageDesc, pixelData, generateMipMaps, sRGB))
    {
        return false;
    }

    if (!createImageView())
    {
        return false;
    }

    if (!createSampler())
    {
        return false;
    }


    return true;
}

void VkTexture::cleanup()
{
    if (_sampler)
    {
        _device->GetDevice().destroySampler(_sampler);
        _sampler = nullptr;
    }

    if (_imageView)
    {
        _device->GetDevice().destroyImageView(_imageView);
        _imageView = nullptr;
    }

    if (_image)
    {
        _device->GetDevice().destroyImage(_image);
        _image = nullptr;
    }

    if (_imageMemory)
    {
        _device->GetDevice().freeMemory(_imageMemory);
        _imageMemory = nullptr;
    }
}

bool VkTexture::createImage(const ImageDesc& imageDesc, const void* pixelData, bool generateMipMaps, bool sRGB)
{
    _width = imageDesc.width;
    _height = imageDesc.height;
    _mipLevels = generateMipMaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height)))) + 1 : 1;

    _format = sRGB ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm;

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = _width;
    imageInfo.extent.height = _height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = _mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = _format;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst |
                      vk::ImageUsageFlagBits::eSampled |
                      (generateMipMaps ? vk::ImageUsageFlagBits::eTransferSrc : vk::ImageUsageFlags{});
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    _image = _device->GetDevice().createImage(imageInfo);

    vk::MemoryRequirements memRequirements = _device->GetDevice().getImageMemoryRequirements(_image);

    uint32_t memoryTypeIndex = _device->FindMemoryType(
        memRequirements.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    _imageMemory = _device->GetDevice().allocateMemory(allocInfo);
    _device->GetDevice().bindImageMemory(_image, _imageMemory, 0);

    vk::DeviceSize imageSize = imageDesc.width * imageDesc.height * 4;

    auto stagingBufferData = _device->createBuffer(
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = _device->GetDevice().mapMemory(stagingBufferData.memory, 0, imageSize);
    memcpy(data, pixelData, static_cast<size_t>(imageSize));
    _device->GetDevice().unmapMemory(stagingBufferData.memory);

    transitionImageLayout(_image, _format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, _mipLevels);

    copyBufferToImage(stagingBufferData.buffer, _image, _width, _height);

    if (generateMipMaps)
    {
        generateMipmaps(_image, _format, _width, _height, _mipLevels);
    }
    else
    {
        transitionImageLayout(_image, _format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, _mipLevels);
    }

    _device->GetDevice().destroyBuffer(stagingBufferData.buffer);
    _device->GetDevice().freeMemory(stagingBufferData.memory);

    return true;
}


bool VkTexture::createImageView()
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = _image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = _format;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = _mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    _imageView = _device->GetDevice().createImageView(viewInfo);
    return _imageView != nullptr;
}

bool VkTexture::createSampler()
{
    const auto& supportedFeatures = _device->GetPhysicalDevice().getFeatures();

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(_mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (supportedFeatures.samplerAnisotropy)
    {
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
    }
    else
    {
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
    }

    try
    {
        _sampler = _device->GetDevice().createSampler(samplerInfo);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to create sampler: " << e.what() << std::endl;
        return false;
    }

    return true;
}



void VkTexture::transitionImageLayout(vk::Image image,
                                      vk::Format format,
                                      vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout,
                                      uint32_t mipLevels)
{
    auto commandBuffer = _device->BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(
        sourceStage,
        destinationStage,
        {}, 0, nullptr, 0, nullptr, 1, &barrier);

    _device->EndSingleTimeCommands(commandBuffer);
}


void VkTexture::copyBufferToImage(vk::Buffer buffer,
                                  vk::Image image,
                                  uint32_t width,
                                  uint32_t height)
{
    vk::CommandBuffer commandBuffer = _device->BeginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0; 
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D{ 0, 0, 0 };
    region.imageExtent = vk::Extent3D{ width, height, 1 };

    commandBuffer.copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal,
        region);

    _device->EndSingleTimeCommands(commandBuffer);
}

void VkTexture::generateMipmaps(vk::Image image,
                                vk::Format imageFormat,
                                uint32_t texWidth,
                                uint32_t texHeight,
                                uint32_t mipLevels)
{
    vk::FormatProperties formatProperties = _device->GetPhysicalDevice().getFormatProperties(imageFormat);

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    vk::CommandBuffer commandBuffer = _device->BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = static_cast<int32_t>(texWidth);
    int32_t mipHeight = static_cast<int32_t>(texHeight);

    for (uint32_t i = 1; i < mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
            {}, nullptr, nullptr, barrier);

        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.dstOffsets[1] = vk::Offset3D{
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
        };
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(
            image, vk::ImageLayout::eTransferSrcOptimal,
            image, vk::ImageLayout::eTransferDstOptimal,
            blit, vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
            {}, nullptr, nullptr, barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
        {}, nullptr, nullptr, barrier);

    _device->EndSingleTimeCommands(commandBuffer);
}


MATERIALX_NAMESPACE_END
