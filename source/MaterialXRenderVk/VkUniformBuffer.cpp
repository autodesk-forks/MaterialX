#include <MaterialXRenderVk/VkUniformBuffer.h>

MATERIALX_NAMESPACE_BEGIN

VkUniformBuffer::VkUniformBuffer(VulkanDevicePtr device) :
    _vulkanDevice(device)
{
    
}

VkUniformBuffer::~VkUniformBuffer()
{
    cleanup();
}

bool VkUniformBuffer::initialize(vk::DeviceSize bufferSize)
{


    _bufferSize = bufferSize;

    if (!createBuffer(bufferSize))
    {
        return false;
    }

    if (!createDescriptorSetLayout())
    {
        return false;
    }

    if (!allocateDescriptorSet())
    {
        return false;
    }

    // Update the uniform buffer descriptor (binding 0)
    if (_texture)
    {
        updateDescriptorSet(_texture->getImageView(), _texture->getSampler());
    }
    else
    {
        updateDescriptorSet(); // Update just the uniform buffer binding
    }

    return true;
}

void VkUniformBuffer::updateData(const void* data, size_t size)
{
    if (size > _bufferSize || !_uniformBufferMemory)
    {
        throw std::runtime_error("Invalid update size or uninitialized buffer");
    }

    void* mappedData = _vulkanDevice->GetDevice().mapMemory(_uniformBufferMemory, 0, size);
    memcpy(mappedData, data, size);
    _vulkanDevice->GetDevice().unmapMemory(_uniformBufferMemory);
}

void VkUniformBuffer::cleanup()
{
    if (_uniformBuffer)
    {
        _vulkanDevice->GetDevice().destroyBuffer(_uniformBuffer);
        _uniformBuffer = nullptr;
    }
    if (_uniformBufferMemory)
    {
        _vulkanDevice->GetDevice().freeMemory(_uniformBufferMemory);
        _uniformBufferMemory = nullptr;
    }
    if (_descriptorSetLayout)
    {
        _vulkanDevice->GetDevice().destroyDescriptorSetLayout(_descriptorSetLayout);
        _descriptorSetLayout = nullptr;
    }
    if (_descriptorPool)
    {
        _vulkanDevice->GetDevice().destroyDescriptorPool(_descriptorPool);
        _descriptorPool = nullptr;
    }
    _descriptorSet = nullptr;
    _bufferSize = 0;
    _texture.reset();
}

bool VkUniformBuffer::createBuffer(vk::DeviceSize size)
{
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    try
    {
        _uniformBuffer = _vulkanDevice->GetDevice().createBuffer(bufferInfo);
    }
    catch (const std::exception& e)
    {
        return false;
    }

    vk::MemoryRequirements memRequirements = _vulkanDevice->GetDevice().getBufferMemoryRequirements(_uniformBuffer);

    vk::PhysicalDeviceMemoryProperties memProperties = _vulkanDevice->GetMemoryProperties();

    uint32_t memoryTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) ==
                (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
        {
            memoryTypeIndex = i;
            break;
        }
    }

    if (memoryTypeIndex == UINT32_MAX)
    {
        return false;
    }

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    try
    {
        _uniformBufferMemory = _vulkanDevice->GetDevice().allocateMemory(allocInfo);
    }
    catch (const std::exception& e)
    {
        return false;
    }

    _vulkanDevice->GetDevice().bindBufferMemory(_uniformBuffer, _uniformBufferMemory, 0);

    return true;
}

bool VkUniformBuffer::createDescriptorSetLayout()
{
    std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};

    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    bindings[0].pImmutableSamplers = nullptr;

    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
    bindings[1].pImmutableSamplers = nullptr;

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    try
    {
        _descriptorSetLayout = _vulkanDevice->GetDevice().createDescriptorSetLayout(layoutInfo);
    }
    catch (const std::exception& e)
    {
        return false;
    }

    return true;
}

bool VkUniformBuffer::allocateDescriptorSet()
{
    if (!_descriptorPool)
    {
        std::array<vk::DescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 1;
        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
        poolSizes[1].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;

        try
        {
            _descriptorPool = _vulkanDevice->GetDevice().createDescriptorPool(poolInfo);
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_descriptorSetLayout;

    try
    {
        auto sets = _vulkanDevice->GetDevice().allocateDescriptorSets(allocInfo);
        _descriptorSet = sets[0];
    }
    catch (const std::exception& e)
    {
        return false;
    }

    return true;
}

void VkUniformBuffer::updateTextureDescriptor(vk::ImageView imageView, vk::Sampler sampler)
{
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = _descriptorSet;
    descriptorWrite.dstBinding = 1; 
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    _vulkanDevice->GetDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void VkUniformBuffer::updateDescriptorSet()
{
    // Update just the uniform buffer descriptor (binding 0)
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = _uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = _bufferSize;

    vk::WriteDescriptorSet uboWrite{};
    uboWrite.dstSet = _descriptorSet;
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

    _vulkanDevice->GetDevice().updateDescriptorSets(1, &uboWrite, 0, nullptr);
}

void VkUniformBuffer::updateDescriptorSet(vk::ImageView imageView = nullptr, vk::Sampler sampler = nullptr)
{
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = _uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = _bufferSize;

    vk::WriteDescriptorSet uboWrite{};
    uboWrite.dstSet = _descriptorSet;
    uboWrite.dstBinding = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &bufferInfo;

    descriptorWrites.push_back(uboWrite);

    if (imageView && sampler)
    {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        vk::WriteDescriptorSet samplerWrite{};
        samplerWrite.dstSet = _descriptorSet;
        samplerWrite.dstBinding = 1;
        samplerWrite.dstArrayElement = 0;
        samplerWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        samplerWrite.descriptorCount = 1;
        samplerWrite.pImageInfo = &imageInfo;

        descriptorWrites.push_back(samplerWrite);
    }

    _vulkanDevice->GetDevice().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

MATERIALX_NAMESPACE_END
