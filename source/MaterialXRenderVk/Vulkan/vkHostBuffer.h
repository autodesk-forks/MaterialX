#include "vkBuffer.h"

class VulkanHostBuffer : public VulkanBuffer
{
    public:
    VulkanHostBuffer(VulkanDevicePtr _device, VkBufferUsageFlags _bufferUsage, VkFormat _bufferFormat);
    virtual ~VulkanHostBuffer();

    virtual void Write(void *bufferData, VkDeviceSize bufferDataSize) override;

    virtual void* Map();
    virtual void UnMap();
};