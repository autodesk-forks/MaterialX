#include <MaterialXRenderVk/vkDevice.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <numeric>
#include <iterator>
#include <algorithm> 
#include <cstdint>
#include <iostream>
#include <iterator>
#include <chrono>
#include <thread>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

VulkanDevice::VulkanDevice(std::vector<const char *> requestedExtensions)
:   _instance(nullptr),
    _physicalDevice(nullptr),
    _device(nullptr),
    _queue(nullptr),
    _commandPool(nullptr),
    _surface(nullptr),
    _pipelineCache(nullptr)
{
    enableValidationLayers = true;
    extensions = requestedExtensions;
    this->CreateInstance();
}

VulkanDevice::~VulkanDevice()
{
    if (_device)
    {
        _device.waitIdle();
    }

    cleanupFramebuffers();

    if (_imageAvailableSemaphore)
    {
        _device.destroySemaphore(_imageAvailableSemaphore);
        _imageAvailableSemaphore = nullptr;
    }

    if (_renderingFinishedSemaphore)
    {
        _device.destroySemaphore(_renderingFinishedSemaphore);
        _renderingFinishedSemaphore = nullptr;
    }

    for (auto& [cmdBuffer, pools] : _commandBufferDescriptorPools)
    {
        for (auto& pool : pools)
        {
            if (pool != VK_NULL_HANDLE)
            {
                _device.destroyDescriptorPool(pool);
            }
        }
    }
    _commandBufferDescriptorPools.clear();

    if (_descriptorSetLayout)
    {
        _device.destroyDescriptorSetLayout(_descriptorSetLayout);
        _descriptorSetLayout = nullptr;
    }

    if (_pipelineCache)
    {
        _device.destroyPipelineCache(_pipelineCache);
        _pipelineCache = nullptr;
    }

    for (auto& [name, texture] : textureCache)
    {
        texture.reset(); // Ensures destructor of VulkanTexture is called
    }
    textureCache.clear();

    if (_commandBuffer)
    {
        _device.freeCommandBuffers(_commandPool, _commandBuffer);
        _commandBuffer = nullptr;
    }

    if (_commandPool)
    {
        _device.destroyCommandPool(_commandPool);
        _commandPool = nullptr;
    }

    _swapChainData.clear(_device);

    if (_device)
    {
        _device.destroy();
        _device = nullptr;
    }

    if (_surface)
    {
        _instance.destroySurfaceKHR(_surface);
        _surface = nullptr;
    }

    if (debugReportCallback != VK_NULL_HANDLE && debugReportCallback != nullptr)
    {
        auto destroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT"));
        if (destroyDebugReportCallbackEXT)
        {
            destroyDebugReportCallbackEXT(_instance, debugReportCallback, nullptr);
            debugReportCallback = VK_NULL_HANDLE;
        }
    }

    if (_instance)
    {
        _instance.destroy();
        _instance = nullptr;
    }
}


void VulkanDevice::InitializeDevice(vk::SurfaceKHR windowSurface, uint32_t width, uint32_t height)
{
    _surface = windowSurface;
    FindPhysicalDevice();
    CreateDevice();

    // Create a swapchain for testing purpose
    _surfaceExtent = vk::Extent2D(width, height);
    _swapChainData = SwapChainData(_physicalDevice,
                                   _device,
                                   _surface,
                                   _surfaceExtent,
                                   vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                                   {},
                                   _graphicsandPresentQueueIndex.first,
                                   _graphicsandPresentQueueIndex.second);

    //_swapImageLayouts.resize(_swapChainData.size(), vk::ImageLayout::ePresentSrcKHR);
}

vk::Semaphore VulkanDevice::createSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo{};
    return _device.createSemaphore(semaphoreInfo);
}

vk::Fence VulkanDevice::createFence(bool signaled = false)
{
    vk::FenceCreateInfo fenceInfo{};
    if (signaled)
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    return _device.createFence(fenceInfo);
}

void VulkanDevice::AppendValidationLayerExtension()
{
    bool foundLayer = false;
    for (const vk::LayerProperties& prop : vk::enumerateInstanceLayerProperties())
    {
        std::cout << prop.layerName << std::endl;
        if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0)
        {
            foundLayer = true;
            break;
        }
    }

    if (!foundLayer)
        throw std::runtime_error("Layer VK_LAYER_KHRONOS_validation not supported\n");

    layers.push_back("VK_LAYER_KHRONOS_validation");

    bool foundExtension = false;
    for (const vk::ExtensionProperties& prop : vk::enumerateInstanceExtensionProperties())
    {
        if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0)
        {
            foundExtension = true;
            break;
        }
    }

    if (!foundExtension)
        throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

void VulkanDevice::CreateInstance()
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    // Initialize debug callback to nullptr
    debugReportCallback = nullptr;

    vk::ApplicationInfo applicationInfo("MaterialX", 1, "MaterialXRender", 1, VK_API_VERSION_1_2);
    if (enableValidationLayers)
    {
        AppendValidationLayerExtension();
    }
    _instance = vk::createInstance(vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo, layers, extensions));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(_instance);

    // Create debug report callback if validation layers are enabled
    if (enableValidationLayers)
    {
        CreateDebugReportCallback();
    }
}

void VulkanDevice::CreateDebugReportCallback()
{
    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    callbackCreateInfo.pfnCallback = debugReportCallbackFn;

    auto createDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT"));

    if (createDebugReportCallbackEXT && 
        createDebugReportCallbackEXT(_instance, &callbackCreateInfo, nullptr, &debugReportCallback) != VK_SUCCESS)
    {
        std::cerr << "Failed to create debug report callback" << std::endl;
        debugReportCallback = VK_NULL_HANDLE;
    }
}

void VulkanDevice::FindPhysicalDevice()
{
    _physicalDevice = _instance.enumeratePhysicalDevices().front();
    
    uint32_t apiVersion = _physicalDevice.getProperties().apiVersion;
    uint32_t driverVersion = _physicalDevice.getProperties().driverVersion;

    VK_LOG << "API: " << VK_VERSION_MAJOR(apiVersion) << "." << VK_VERSION_MINOR(apiVersion) << "." << VK_VERSION_PATCH(apiVersion) << std::endl;
    VK_LOG << "Driver: " << VK_VERSION_MAJOR(driverVersion) << "." << VK_VERSION_MINOR(driverVersion) << "." << VK_VERSION_PATCH(driverVersion) << std::endl;
    VK_LOG << "Device: " << _physicalDevice.getProperties().deviceName << std::endl;
}

vk::DescriptorSet VulkanDevice::createDescriptorSet(vk::Buffer buffer)
{
    // Only create descriptor set layout if it doesn't exist
    if (!_descriptorSetLayout)
    {
        _descriptorSetLayout = createDescriptorSetLayout({ { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } });
    }
    
    vk::DescriptorPool descriptorPool = createDescriptorPool({ { vk::DescriptorType::eUniformBuffer, 1 } });
    vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, _descriptorSetLayout);
    vk::DescriptorSet descriptorSet = _device.allocateDescriptorSets(allocInfo).front();

    // Track this descriptor pool for cleanup
    std::vector<VkDescriptorPool> pools = { descriptorPool };
    _commandBufferDescriptorPools[VK_NULL_HANDLE] = pools;

    updateDescriptorSets(descriptorSet,
                         { { vk::DescriptorType::eUniformBuffer, buffer, VK_WHOLE_SIZE, {} } },
                         {});
    return descriptorSet;
}

VulkanDevice::BufferData VulkanDevice::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
   
    BufferData data{};
    data.size = size;

    // Create the buffer
    data.buffer = _device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage));

    // Get memory requirements
    vk::MemoryRequirements memRequirements = _device.getBufferMemoryRequirements(data.buffer);

    // Find suitable memory type
    vk::PhysicalDeviceMemoryProperties memProps = _physicalDevice.getMemoryProperties();
    uint32_t typeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties)
        {
            typeIndex = i;
            break;
        }
    }
    if (typeIndex == UINT32_MAX)
    {
        throw std::runtime_error("Failed to find suitable memory type for buffer.");
    }

    // Allocate memory
    data.memory = _device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, typeIndex));

    // Bind memory to buffer
    _device.bindBufferMemory(data.buffer, data.memory, 0);

    return data;
}

vk::DescriptorSetLayout VulkanDevice::createDescriptorSetLayout(
    const std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>>& bindingData,
    vk::DescriptorSetLayoutCreateFlags flags)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings(bindingData.size());
    for (size_t i = 0; i < bindingData.size(); ++i)
    {
        assert(i <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
        bindings[i] = vk::DescriptorSetLayoutBinding(
            static_cast<uint32_t>(i),
            std::get<0>(bindingData[i]),
            std::get<1>(bindingData[i]),
            std::get<2>(bindingData[i]));
    }

    return _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(flags, bindings));
}

vk::DescriptorPool VulkanDevice::createDescriptorPool(std::vector<vk::DescriptorPoolSize> const& poolSizes)
{
    assert(!poolSizes.empty());
    uint32_t maxSets =
        std::accumulate(poolSizes.begin(), poolSizes.end(), 0, [](uint32_t sum, vk::DescriptorPoolSize const& dps)
    {
        return sum + dps.descriptorCount;
    });
    assert(0 < maxSets);

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, poolSizes);
    return _device.createDescriptorPool(descriptorPoolCreateInfo);
}

void VulkanDevice::updateDescriptorSets(vk::DescriptorSet const& descriptorSet,
                          std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize, vk::BufferView const&>> const& bufferData,
                          uint32_t bindingOffset)
{
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(bufferData.size());

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(bufferData.size()  ? 0 : 1);
    uint32_t dstBinding = bindingOffset;
    for (auto const& bd : bufferData)
    {
        bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
        writeDescriptorSets.emplace_back(descriptorSet, dstBinding++, 0, 1, std::get<0>(bd), nullptr, &bufferInfos.back(), &std::get<3>(bd));
    }

    std::vector<vk::DescriptorImageInfo> imageInfos;

    _device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

glm::mat4x4 VulkanDevice::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
    glm::vec3 f = glm::normalize(center - eye);      // forward
    glm::vec3 r = glm::normalize(glm::cross(f, up)); // right
    glm::vec3 u = glm::cross(r, f);                  // true up

    glm::mat4x4 result(1.0f);
    result[0][0] = r.x;
    result[1][0] = r.y;
    result[2][0] = r.z;

    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;

    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;

    result[3][0] = -glm::dot(r, eye);
    result[3][1] = -glm::dot(u, eye);
    result[3][2] = glm::dot(f, eye);
    return result;
}

glm::mat4x4 VulkanDevice::perspective(float fovY, float aspect, float nearP, float farP)
{
    float tanHalfFovy = tan(fovY / 2.0f);

    glm::mat4x4 result(0.0f);
    result[0][0] = 1.0f / (aspect * tanHalfFovy);
    result[1][1] = 1.0f / (tanHalfFovy);
    result[2][2] = -(farP + nearP) / (farP - nearP);
    result[2][3] = -1.0f;
    result[3][2] = -(2.0f * farP * nearP) / (farP - nearP);

    return result;
}

glm::mat4x4 VulkanDevice::createModelViewProjectionClipMatrix(vk::Extent2D const& extent)
{
    {
        float fov = glm::radians(45.0f);
        if (extent.width > extent.height)
        {
            fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);
        }

        glm::mat4x4 model = glm::mat4x4(1.0f);
        glm::mat4x4 view = lookAt(glm::vec3(-5.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4x4 projection = perspective(fov, 1.0f, 0.1f, 100.0f);
        // clang-format off
        glm::mat4x4 clip = glm::mat4x4( 1.0f,  0.0f, 0.0f, 0.0f,
                                      0.0f, -1.0f, 0.0f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 1.0f );  // vulkan clip space has inverted y and half z !
      // clang-format on 
      return clip * projection * view * model;
    }
}


std::vector<char const*> gatherExtensions(std::vector<std::string> const& extensions
#if !defined(NDEBUG)
                                          ,
                                          std::vector<vk::ExtensionProperties> const& extensionProperties
#endif
)
{
    std::vector<char const*> enabledExtensions;
    enabledExtensions.reserve(extensions.size());
    for (auto const& ext : extensions)
    {
        assert(std::any_of(
            extensionProperties.begin(), extensionProperties.end(), [ext](vk::ExtensionProperties const& ep)
        {
            return ext == ep.extensionName;
        }));
        enabledExtensions.push_back(ext.data());
    }
#if !defined(NDEBUG)
    if (std::none_of(
            extensions.begin(), extensions.end(), [](std::string const& extension)
    {
        return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }) &&
        std::any_of(extensionProperties.begin(), extensionProperties.end(), [](vk::ExtensionProperties const& ep)
    {
        return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ep.extensionName) == 0);
    }))
    {
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#endif
    return enabledExtensions;
}


vk::RenderPass VulkanDevice::createRenderPass(
    vk::Format colorFormat,
    vk::Format depthFormat,
    vk::AttachmentLoadOp loadOp,
    vk::ImageLayout colorFinalLayout
)
{
    std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
    attachmentDescriptions[0] = vk::AttachmentDescription(
        {},
        colorFormat,
        vk::SampleCountFlagBits::e1,
        loadOp,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        colorFinalLayout
    );

    attachmentDescriptions[1] = vk::AttachmentDescription(
        {},
        depthFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depthRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass(
        {},
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorRef,
        nullptr,
        &depthRef
    );

    vk::SubpassDependency dependency(
        VK_SUBPASS_EXTERNAL, 0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    );

    return _device.createRenderPass(
        vk::RenderPassCreateInfo({}, attachmentDescriptions, subpass, dependency)
    );
}


vk::SurfaceFormatKHR VulkanDevice::pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats)
{
    if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
    {
        return vk::SurfaceFormatKHR{
            vk::Format::eB8G8R8A8Unorm,
            vk::ColorSpaceKHR::eSrgbNonlinear
        };
    }

    for (const auto& availableFormat : formats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return formats[0];
}

std::vector<vk::Framebuffer> VulkanDevice::createFramebuffers(
    vk::RenderPass renderPass,
    const std::vector<vk::ImageView>& colorImageViews,
    vk::ImageView depthImageView,
    vk::Extent2D extent)
{
    std::vector<vk::Framebuffer> framebuffers;
    framebuffers.reserve(colorImageViews.size());

    for (const auto& colorView : colorImageViews)
    {
        std::array<vk::ImageView, 2> attachments = { colorView, depthImageView };

        vk::FramebufferCreateInfo framebufferInfo(
            vk::FramebufferCreateFlags(),
            renderPass,
            static_cast<uint32_t>(attachments.size()),
            attachments.data(),
            extent.width,
            extent.height,
            1);

        vk::Framebuffer framebuffer = _device.createFramebuffer(framebufferInfo);
        framebuffers.push_back(framebuffer);
        
        addFramebufferForCleanup(framebuffer);
    }

    return framebuffers;
}

std::pair<uint32_t, uint32_t> VulkanDevice::findGraphicsAndPresentQueueFamilyIndex( vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const & surface )
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    assert( queueFamilyProperties.size() < ( std::numeric_limits<uint32_t>::max )() );

    // look for a queueFamilyIndex that supports graphics and present
    auto combinedIt = std::find_if( queueFamilyProperties.begin(),
                                    queueFamilyProperties.end(),
                                    [&physicalDevice, &surface]( vk::QueueFamilyProperties const & qfp )
                                    {
                                    static uint32_t index = 0;
                                    return ( qfp.queueFlags & vk::QueueFlagBits::eGraphics ) && physicalDevice.getSurfaceSupportKHR( index++, surface );
                                    } );
    if ( combinedIt != queueFamilyProperties.end() )
    {
    uint32_t index = static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), combinedIt ) );
    return { index, index };  // the first index that supports graphics and present
    }
    else
    {
    // there's no single index that supports both graphics and present -> look for separate ones
    auto graphicsIt = std::find_if( queueFamilyProperties.begin(),
                                    queueFamilyProperties.end(),
                                    []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );
    if ( graphicsIt != queueFamilyProperties.end() )
    {
        uint32_t graphicsIndex = static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), graphicsIt ) );
        auto     presentIt     = std::find_if( queueFamilyProperties.begin(),
                                        queueFamilyProperties.end(),
                                        [&physicalDevice, &surface]( vk::QueueFamilyProperties const & )
                                        {
                                        static uint32_t index = 0;
                                        return physicalDevice.getSurfaceSupportKHR( index++, surface );
                                        } );
        if ( presentIt != queueFamilyProperties.end() )
        {
        uint32_t presentIndex = static_cast<uint32_t>( std::distance( queueFamilyProperties.begin(), presentIt ) );
        return { graphicsIndex, presentIndex };
        }
        else
        {
        throw std::runtime_error( "Could not find a queue family index that supports present -> terminating" );
        }
    }
    else
    {
        throw std::runtime_error( "Could not find a queue family index that supports graphics -> terminating" );
    }
    }
}

vk::DescriptorSet VulkanDevice::getDescriptorSet() const
{
    return _descriptorSet;
}


void VulkanDevice::CreateDevice()
{
    // Find graphics queue
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
    assert(queueFamilyProperties.size() < std::numeric_limits<uint32_t>::max());

    // Find graphics queue family
    auto graphicsQueueFamilyProperty = std::find_if(queueFamilyProperties.begin(),
                                                    queueFamilyProperties.end(),
                                                    [](vk::QueueFamilyProperties const& qfp)
                                                    {
                                                        return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                                                    });
    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    _graphicsQueueFamilyIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

    // Find a present queue family
    size_t presentQueueFamilyIndex = _physicalDevice.getSurfaceSupportKHR(_graphicsQueueFamilyIndex, _surface)
                                         ? _graphicsQueueFamilyIndex
                                         : queueFamilyProperties.size();
    if (presentQueueFamilyIndex == queueFamilyProperties.size())
    {
        for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
        {
            if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                _physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _surface))
            {
                _graphicsQueueFamilyIndex = static_cast<uint32_t>(i);
                presentQueueFamilyIndex = i;
                break;
            }
        }
        if (presentQueueFamilyIndex == queueFamilyProperties.size())
        {
            for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
            {
                if (_physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), _surface))
                {
                    presentQueueFamilyIndex = i;
                    break;
                }
            }
        }
    }

    if (_graphicsQueueFamilyIndex == queueFamilyProperties.size() || presentQueueFamilyIndex == queueFamilyProperties.size())
    {
        throw std::runtime_error("Could not find a queue for graphics or present -> terminating");
    }

    _graphicsandPresentQueueIndex = std::make_pair(_graphicsQueueFamilyIndex, presentQueueFamilyIndex);

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
    };

    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, _graphicsandPresentQueueIndex.first, 1, &queuePriority);

    vk::PhysicalDeviceFeatures supportedFeatures = _physicalDevice.getFeatures();

    vk::PhysicalDeviceFeatures enabledFeatures{};
    if (supportedFeatures.samplerAnisotropy)
    {
        enabledFeatures.samplerAnisotropy = VK_TRUE;
    }
    else
    {
        std::cerr << "Warning: samplerAnisotropy not supported by device." << std::endl;
    }

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    _device = _physicalDevice.createDevice(deviceCreateInfo);

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    VULKAN_HPP_DEFAULT_DISPATCHER.init(_device);
#endif

    _device.getQueue(_graphicsandPresentQueueIndex.first, 0, &_graphicsQueue);
}


void VulkanDevice::CreateCommandPool()
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphicsandPresentQueueIndex.first);
    _commandPool = _device.createCommandPool(commandPoolCreateInfo);

    _commandBuffer = _device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(_commandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
    
}

VkDescriptorPool VulkanDevice::GetDescriptorPool(VkCommandBuffer commandBuffer)
{
    if (_commandBufferDescriptorPools.find(commandBuffer) == _commandBufferDescriptorPools.end())
        AllocateNewDescriptorPool(commandBuffer);
    if (_commandBufferDescriptorPools[commandBuffer].size() == 0)
        AllocateNewDescriptorPool(commandBuffer);

    return *(_commandBufferDescriptorPools[commandBuffer].rbegin());
}

void VulkanDevice::AllocateNewDescriptorPool(VkCommandBuffer commandBuffer)
{
    auto& pools = _commandBufferDescriptorPools[commandBuffer];
    pools.resize(pools.size() + 1);

    // default pool sizes
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 256 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 256 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 256 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 256 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 256 }
    };

    VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 256;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_ERROR_CHECK(
        vkCreateDescriptorPool(this->_device, &poolInfo, NULL, &pools[pools.size() - 1]));
}

// Call this when you're done with the command buffer
void VulkanDevice::FreeDescriptorPools(VkCommandBuffer commandBuffer)
{
    auto& pools = _commandBufferDescriptorPools[commandBuffer];
    for (auto& pool : pools)
        vkDestroyDescriptorPool(this->_device, pool, nullptr);
    pools.clear();
}

uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlagBits flags)
{
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, NULL);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for(; i<queueFamilies.size(); ++i)
    {
        VkQueueFamilyProperties props = queueFamilies[i];

        if( props.queueCount > 0 && (props.queueFlags & flags) )
        {
            if( _surface != nullptr )
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &presentSupport);
                if( presentSupport )
                    break;
            }else
                break;
        }
    }

    if( i == queueFamilies.size() )
        throw std::runtime_error("Could not find a queue family that supports graphics.");

    return i;
}

bool VulkanDevice::QuerySwapChainSupport(SwapChainSupportDetails &details, VkPhysicalDevice pd)
{
    if( pd == nullptr )
        pd = _physicalDevice;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, _surface, &details.capabilities);

    // formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pd, _surface, &formatCount, details.formats.data());
    }

    // present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pd, _surface, &presentModeCount, details.presentModes.data());
    }
    return true;
}


VkSurfaceFormatKHR VulkanDevice::FindSurfaceFormat()
{
    SwapChainSupportDetails swapChainSupport;
    this->QuerySwapChainSupport(swapChainSupport);
    return this->FindSurfaceFormat(swapChainSupport.formats);
}

VkSurfaceFormatKHR VulkanDevice::FindSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
            if( availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
                return availableFormat;
    }
    return availableFormats[0];
}


uint32_t VulkanDevice::FindMemoryType(uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = _physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }

    throw std::runtime_error("Appropriate memory type not found.");
}

VkFormatProperties VulkanDevice::FindFormatProperties(VkFormat format)
{
    VkFormatProperties props = {};
    vkGetPhysicalDeviceFormatProperties(this->_physicalDevice, format, &props);
    return props;
}

VkSampleCountFlagBits VulkanDevice::GetMaxMultisampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags sampleCounts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (sampleCounts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (sampleCounts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (sampleCounts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (sampleCounts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (sampleCounts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (sampleCounts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}


VulkanDevice::SwapChainData::SwapChainData(vk::PhysicalDevice const& physicalDevice,
                             vk::Device const& device,
                             vk::SurfaceKHR const& surface,
                             vk::Extent2D const& extent,
                             vk::ImageUsageFlags usage,
                             vk::SwapchainKHR const& oldSwapChain,
                             uint32_t graphicsQueueFamilyIndex,
                             uint32_t presentQueueFamilyIndex)
{
    // 
    std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
    assert(!formats.empty());

    vk::SurfaceFormatKHR surfaceFormat = formats[0];

    if (formats.size() == 1)
    {
        if (formats[0].format == vk::Format::eUndefined)
        {
            surfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
            surfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        }
    }
    else
    {
        // request several formats, the first found will be used
        vk::Format requestedFormats[] = { vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm };
        vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        for (size_t i = 0; i < sizeof(requestedFormats) / sizeof(requestedFormats[0]); i++)
        {
            vk::Format requestedFormat = requestedFormats[i];
            auto it = std::find_if(formats.begin(),
                                   formats.end(),
                                   [requestedFormat, requestedColorSpace](vk::SurfaceFormatKHR const& f)
                                   {
                return (f.format == requestedFormat) && (f.colorSpace == requestedColorSpace);
            });
            if (it != formats.end())
            {
                surfaceFormat = *it;
                break;
            }
        }
    }
    assert(surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);

    _colorFormat = surfaceFormat.format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    vk::Extent2D swapchainExtent;
    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        // If the surface size is undefined, the size is set to the size of the images requested.
        swapchainExtent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfaceCapabilities.currentExtent;
    }
    vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
                                                       ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                                                       : surfaceCapabilities.currentTransform;
    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)    ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)        ? vk::CompositeAlphaFlagBitsKHR::eInherit
                                                                                                         : vk::CompositeAlphaFlagBitsKHR::eOpaque;
    vk::PresentModeKHR presentationMode = vk::PresentModeKHR::eFifo;
    for (const auto& presentMode : physicalDevice.getSurfacePresentModesKHR(surface))
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            presentationMode = presentMode;
            break;
        }

        if (presentMode == vk::PresentModeKHR::eImmediate)
        {
            presentationMode = presentMode;
        }
    }

    // SurfaceImageCount 
    uint32_t surfaceImageCount = 3;
    surfaceImageCount = ( std::max )( surfaceImageCount, surfaceCapabilities.minImageCount );
      if ( surfaceImageCount > 0 )
      {
        surfaceImageCount = ( std::min )( surfaceImageCount, surfaceCapabilities.maxImageCount );
      }

    // Create SwapChain
    vk::SwapchainCreateInfoKHR swapChainCreateInfo({},
                                                   surface,
                                                   surfaceImageCount,
                                                   _colorFormat,
                                                   surfaceFormat.colorSpace,
                                                   swapchainExtent,
                                                   1,
                                                   usage,
                                                   vk::SharingMode::eExclusive,
                                                   {},
                                                   preTransform,
                                                   compositeAlpha,
                                                   presentationMode,
                                                   true,
                                                   oldSwapChain);

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        uint32_t queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
        // If the graphics and present queues are from different queue families, we either have to explicitly transfer
        // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
        // vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    _swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    _images = device.getSwapchainImagesKHR(_swapChain);


    _imageViews.reserve(_images.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, _colorFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    for (auto image : _images)
    {
        imageViewCreateInfo.image = image;
        _imageViews.push_back(device.createImageView(imageViewCreateInfo));
    }
}

void VulkanDevice::SwapChainData::clear(vk::Device const& device)
{
    for (auto& imageView : _imageViews)
    {
        device.destroyImageView(imageView);
    }
    _imageViews.clear();
    _images.clear();
    device.destroySwapchainKHR(_swapChain);
}


vk::CommandBuffer VulkanDevice::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = _commandPool; 
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    commandBuffer = _device.allocateCommandBuffers(allocInfo).front();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void VulkanDevice::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    _graphicsQueue.submit(submitInfo, nullptr);  
    _graphicsQueue.waitIdle();

    _device.freeCommandBuffers(_commandPool, commandBuffer);
}

vk::PipelineCache VulkanDevice::getPipelineCache()
{
    if (!_pipelineCache)
    {
        vk::PipelineCacheCreateInfo pipelineCacheInfo{};
        _pipelineCache = _device.createPipelineCache(pipelineCacheInfo);
    }
    return _pipelineCache;
}

void VulkanDevice::addFramebufferForCleanup(vk::Framebuffer framebuffer)
{
    _framebuffers.push_back(framebuffer);
}

void VulkanDevice::cleanupFramebuffers()
{
    for (auto& framebuffer : _framebuffers)
    {
        if (framebuffer)
        {
            _device.destroyFramebuffer(framebuffer);
        }
    }
    _framebuffers.clear();
}
