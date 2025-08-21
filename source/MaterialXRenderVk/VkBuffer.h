#pragma once

#include <vulkan/vulkan.hpp>
#include <MaterialXRenderVk/Export.h>
#include <memory>

// Forward declaration
class VulkanDevice;
using VulkanDevicePtr = std::shared_ptr<VulkanDevice>;

MATERIALX_NAMESPACE_BEGIN

class MX_RENDERVK_API VkBuffer;
using VkBufferPtr = std::shared_ptr<VkBuffer>;

class MX_RENDERVK_API VkBuffer
{
  public:
    static VkBufferPtr create(VulkanDevicePtr device)
    {
        return VkBufferPtr(new VkBuffer(device));
    }

    explicit VkBuffer(VulkanDevicePtr device);
    virtual ~VkBuffer();

    void create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);


    void CreateVertexBuffer(const void* vertexData, vk::DeviceSize size, uint32_t stride);

    void CreateIndexBuffer(const void* indexData, vk::DeviceSize size, uint32_t stride);

    /// Upload raw data to buffer
    void upload(const void* data, vk::DeviceSize size);

    /// Destroy Vulkan resources
    void destroy();

    /// Get Vulkan buffer handle
    vk::Buffer getBuffer() const { return _buffer; }

    /// Get Vulkan memory
    vk::DeviceMemory getMemory() const { return _memory; }

    /// Get buffer size
    vk::DeviceSize getSize() const { return _size; }

    /// Get stride (for vertex buffer)
    uint32_t getStride() const { return _stride; }

  protected:
  

  private:
    VulkanDevicePtr _devicePtr;

    vk::Buffer _buffer = nullptr;
    vk::DeviceMemory _memory = nullptr;
    vk::DeviceSize _size = 0;
    uint32_t _stride = 0;
};

MATERIALX_NAMESPACE_END
