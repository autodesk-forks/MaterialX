//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include <vulkan/vulkan.hpp>
#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkDevice.h>
#include <MaterialXRender/Image.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/Util.h>




class VulkanDevice;
using VulkanDevicePtr = std::shared_ptr<VulkanDevice>;
MATERIALX_NAMESPACE_BEGIN

// Shared pointer to VkTexture
class MX_RENDERVK_API VkTexture;
using VkTexturePtr = std::shared_ptr<VkTexture>;


class MX_RENDERVK_API VkTexture
{
  public:
    static VkTexturePtr create(VulkanDevicePtr device)
    {
        return std::make_shared<VkTexture>(device);
    }

    /// Constructor
    explicit VkTexture(VulkanDevicePtr device);

    virtual ~VkTexture();


      struct ImageDesc
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channelCount = 4; //4 for RGBA
        uint32_t mipCount = 1;
        uint32_t rowStride = 0;   // in bytes
        uint32_t elementSize = 1; // bytes per channel

        bool isFloat = false;
        bool isHalf = false;
    };


    /// Initialize the texture from image data
    bool initialize(const ImageDesc& imageDesc,
                    const void* pixelData,
                    bool generateMipMaps,
                    bool sRGB = false);

    void cleanup();

    vk::Image getImage() const { return _image; }

    vk::ImageView getImageView() const { return _imageView; }

    uint32_t getWidth() const { return _width; }

    uint32_t getHeight() const { return _height; }

    uint32_t getMipLevels() const { return _mipLevels; }

    vk::Format getFormat() const { return _format; }

    vk::ImageView getTextureImageView() const { return _imageView; }

    vk::Sampler getSampler() const
    {
        if (!_sampler)
        {
            std::cerr << "Warning: Attempting to access invalid sampler!" << std::endl;
        }
        return _sampler;
    }

  private:
    VulkanDevicePtr _device;

    vk::Image _image = nullptr;
    vk::DeviceMemory _imageMemory = nullptr;
    vk::ImageView _imageView = nullptr;
    vk::Sampler _sampler = nullptr;

    uint32_t _width = 0;
    uint32_t _height = 0;
    uint32_t _mipLevels = 1;
    vk::Format _format = vk::Format::eUndefined;

    bool createImage(const ImageDesc& imageDesc,
                     const void* pixelData,
                     bool generateMipMaps,
                     bool sRGB);

    bool createImageView();

    bool createSampler();

    void transitionImageLayout(vk::Image image,
                               vk::Format format,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               uint32_t mipLevels);

    void copyBufferToImage(vk::Buffer buffer,
                           vk::Image image,
                           uint32_t width,
                           uint32_t height);

    void generateMipmaps(vk::Image image,
                         vk::Format imageFormat,
                         uint32_t texWidth,
                         uint32_t texHeight,
                         uint32_t mipLevels);
};

MATERIALX_NAMESPACE_END
