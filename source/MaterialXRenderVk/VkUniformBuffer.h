#ifndef MATERIALX_VKUNIFORMBUFFER_H
#define MATERIALX_VKUNIFORMBUFFER_H
#include <vulkan/vulkan.hpp>
#include <MaterialXRenderVk/Export.h>
#include <MaterialXRenderVk/VkDevice.h>
#include <MaterialXRenderVk/VkTexture.h>


class VulkanDevice;
using VulkanDevicePtr = std::shared_ptr<VulkanDevice>;

MATERIALX_NAMESPACE_BEGIN

class MX_RENDERVK_API VkUniformBuffer;
using VkUniformBufferPtr = std::shared_ptr<class VkUniformBuffer>;

/// @class VkUniformBuffer
/// A class encapsulating Vulkan uniform buffer, descriptor set, and descriptor set layout management.
class MX_RENDERVK_API VkUniformBuffer
{

    public:
    static VkUniformBufferPtr create(VulkanDevicePtr device)
    {
        return VkUniformBufferPtr(new VkUniformBuffer(device));
    }

    explicit VkUniformBuffer(VulkanDevicePtr device);

    /// Destructor cleans up Vulkan resources.
    virtual ~VkUniformBuffer();

    /// Initialize uniform buffer and descriptor set layout with specified size.
    bool initialize(vk::DeviceSize bufferSize);

    /// Update the contents of the uniform buffer.
    void updateData(const void* data, size_t size);

    /// Get the Vulkan buffer handle.
    vk::Buffer getBuffer() const { return _uniformBuffer; }

    /// Get the descriptor set layout for this uniform buffer.
    vk::DescriptorSetLayout getDescriptorSetLayout() const { return _descriptorSetLayout; }

    /// Get the descriptor set allocated for this uniform buffer.
    vk::DescriptorSet getDescriptorSet() const { return _descriptorSet; }

    /// Clean up all Vulkan resources managed by this object.
    void cleanup();

    bool createBuffer(vk::DeviceSize size);
    bool createDescriptorSetLayout();
    bool allocateDescriptorSet();
    void updateTextureDescriptor(vk::ImageView imageView, vk::Sampler sampler);
    void updateDescriptorSet();
    void updateDescriptorSet(vk::ImageView imageView, vk::Sampler sampler);
  protected:
    VkUniformBuffer() = delete;

  private:

  private:
    VulkanDevicePtr _vulkanDevice;
    VkTexturePtr _texture;
    vk::Buffer _uniformBuffer = nullptr;
    vk::DeviceMemory _uniformBufferMemory = nullptr;

    vk::DescriptorSetLayout _descriptorSetLayout = nullptr;
    vk::DescriptorPool _descriptorPool = nullptr;
    vk::DescriptorSet _descriptorSet = nullptr;
    
    vk::DeviceSize _bufferSize = 0;
};

MATERIALX_NAMESPACE_END

#endif
