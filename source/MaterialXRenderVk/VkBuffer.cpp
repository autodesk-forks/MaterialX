#include <MaterialXRenderVk/VkBuffer.h>
#include <MaterialXRenderVk/VkDevice.h>
#include <MaterialXRenderVk/VkFrameBuffer.h>
#include <cstring>
#include <stdexcept>

MATERIALX_NAMESPACE_BEGIN

VkBuffer::VkBuffer(VulkanDevicePtr device) :
    _devicePtr(device)
{
}

VkBuffer::~VkBuffer()
{
    destroy();
}

void VkBuffer::destroy()
{
    if (_buffer)
    {
        _devicePtr->GetDevice().destroyBuffer(_buffer);
        _buffer = nullptr;
    }
    if (_memory)
    {
        _devicePtr->GetDevice().freeMemory(_memory);
        _memory = nullptr;
    }
    _size = 0;
    _stride = 0;
}

void VkBuffer::upload(const void* data, vk::DeviceSize size)
{
    if (!_memory)
    {
        throw std::runtime_error("Cannot upload to buffer without allocated memory.");
    }
    _devicePtr->copyToDevice(_memory, data, static_cast<size_t>(size));
}

void VkBuffer::create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    destroy(); 

    _size = size;

    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
    _buffer = _devicePtr->GetDevice().createBuffer(bufferInfo);

    vk::MemoryRequirements memReqs = _devicePtr->GetDevice().getBufferMemoryRequirements(_buffer);
    uint32_t memTypeIndex = _devicePtr->FindMemoryType(memReqs.memoryTypeBits, properties);

    vk::MemoryAllocateInfo allocInfo(memReqs.size, memTypeIndex);
    _memory = _devicePtr->GetDevice().allocateMemory(allocInfo);

    _devicePtr->GetDevice().bindBufferMemory(_buffer, _memory, 0);
}

void VkBuffer::CreateVertexBuffer(const void* vertexData, vk::DeviceSize size, uint32_t stride)
{
    create(size,
           vk::BufferUsageFlagBits::eVertexBuffer,
           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    _stride = stride;

    upload(vertexData, size);
}

void VkBuffer::CreateIndexBuffer(const void* indexData, vk::DeviceSize size, uint32_t stride)
{
    create(size,
           vk::BufferUsageFlagBits::eIndexBuffer,
           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    _stride = stride;

    upload(indexData, size);
}


MATERIALX_NAMESPACE_END
