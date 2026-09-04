//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKTEXTUREHANDLER_H
#define MATERIALX_VKTEXTUREHANDLER_H

/// @file
/// Vulkan texture handler

#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkContext.h>

#include <MaterialXRender/ImageHandler.h>

#include <vulkan/vulkan.h>

#include <map>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a Vulkan texture handler
using VkTextureHandlerPtr = std::shared_ptr<class VkTextureHandler>;

/// @class VkTextureHandler
/// A Vulkan texture handler class. Mirrors GLTextureHandler, but Vulkan has no
/// "texture unit" concept — bindImage() just ensures the resource exists and
/// records sampling properties; the descriptor write happens via
/// getDescriptorInfo() in VkProgram::bindTextures().
class MX_RENDERVK_API VkTextureHandler : public ImageHandler
{
    friend class VkProgram;

  public:
    static ImageHandlerPtr create(VkContextPtr context, ImageLoaderPtr imageLoader)
    {
        return ImageHandlerPtr(new VkTextureHandler(context, imageLoader));
    }

    /// Bind an image. Ensures the Vulkan resource exists and records sampling
    /// properties. Has no "texture unit" to occupy in Vulkan.
    bool bindImage(ImagePtr image, const ImageSamplingProperties& samplingProperties) override;

    /// Unbind an image.
    bool unbindImage(ImagePtr image) override;

    /// Create rendering resources for the given image.
    bool createRenderResources(ImagePtr image, bool generateMipMaps,
                               bool useAsRenderTarget = false) override;

    /// Release rendering resources for the given image, or for all cached images
    /// if no image pointer is specified.
    void releaseRenderResources(ImagePtr image = nullptr) override;

    /// Return the descriptor image info for a bound image, for writing into
    /// a VkWriteDescriptorSet in VkProgram::bindTextures().
    VkDescriptorImageInfo getDescriptorInfo(ImagePtr image, const ImageSamplingProperties& samplingProperties);

    /// Return a sampler for the given sampling properties (cached).
    VkSampler getSampler(const ImageSamplingProperties& samplingProperties);

    /// Utility to map an address mode to a Vulkan address mode.
    static VkSamplerAddressMode mapAddressModeToVk(ImageSamplingProperties::AddressMode addressModeEnum);

    /// Utility to map a filter type to a Vulkan filter.
    static VkFilter mapFilterTypeToVk(ImageSamplingProperties::FilterType filterTypeEnum, bool mip = false);

    /// Utility to map a mipmap filter type to a Vulkan mipmap mode.
    static VkSamplerMipmapMode mapMipModeToVk(ImageSamplingProperties::FilterType filterTypeEnum);

    /// Utility to map generic texture properties to a Vulkan format.
    static VkFormat mapTextureFormatToVk(Image::BaseType baseType, unsigned int channelCount, bool srgb);

  protected:
    VkTextureHandler(VkContextPtr context, ImageLoaderPtr imageLoader);

    void generateMipmaps(VkCommandBuffer cmd, VkImage image, uint32_t mipLevels,
                         unsigned int width, unsigned int height);

    VkContextPtr _context;

    struct VkTextureResource
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        uint32_t mipLevels = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    // Keyed by Image::getResourceId().
    std::map<unsigned int, VkTextureResource> _textures;

    // Cached samplers keyed by sampling properties.
    std::unordered_map<ImageSamplingProperties, VkSampler, ImageSamplingKeyHasher> _samplers;
};

MATERIALX_NAMESPACE_END

#endif
