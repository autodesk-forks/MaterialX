//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkContext.h>
#include <MaterialXRenderVk/VkUtil.h>

#include <MaterialXFormat/Util.h>

#include <set>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Validation layers requested in debug builds.
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

// Required device extensions. Headless: no surface, no swapchain.
const std::vector<const char*> DEVICE_EXTENSIONS = {
    // Intentionally empty. We request only VK_EXT_debug_utils at instance level,
    // and only in validation builds. No swapchain, no surface.
};

bool checkValidationLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : VALIDATION_LAYERS)
    {
        bool found = false;
        for (const auto& layer : availableLayers)
        {
            if (strcmp(layerName, layer.layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    return debugUtilsMessengerCallback(messageSeverity, messageTypes, pCallbackData, pUserData);
}

VkResult createDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* pMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        return func(instance, &createInfo, nullptr, pMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func)
    {
        func(instance, messenger, nullptr);
    }
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            indices.graphicsFamily = i;
        }
        if (indices.isComplete())
            break;
        i++;
    }
    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!indices.isComplete())
        return false;

    // Check that all required device extensions are present.
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

} // anonymous namespace

VkContextPtr VkContext::create(bool enableValidation)
{
    VkContextPtr context(new VkContext(enableValidation));
    context->pickPhysicalDevice();
    context->createLogicalDevice();
    context->createCommandPool();
    return context;
}

VkContextPtr VkContext::tryCreate(std::ostream* log) noexcept
{
    try
    {
        return create(MX_VK_DEFAULT_VALIDATION);
    }
    catch (const std::exception& e)
    {
        if (log)
            *log << "VkContext::tryCreate failed: " << e.what() << std::endl;
        return nullptr;
    }
    catch (...)
    {
        if (log)
            *log << "VkContext::tryCreate failed: unknown error" << std::endl;
        return nullptr;
    }
}

bool VkContext::isDeviceAvailable()
{
    static bool cached = false;
    static bool checked = false;
    if (checked)
        return cached;
    checked = true;

    try
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            // Retry without the portability flag (older loaders don't support it).
            createInfo.flags = 0;
            result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS)
                return false;
        }

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (instance)
            vkDestroyInstance(instance, nullptr);
        cached = (deviceCount > 0);
    }
    catch (...)
    {
        cached = false;
    }
    return cached;
}

VkContextPtr VkContext::createShared(VkInstance instance, VkPhysicalDevice physicalDevice,
                                    VkDevice device, uint32_t queueFamilyIndex, VkQueue queue)
{
    VkContextPtr context(new VkContext(false));
    context->_instance = instance;
    context->_physicalDevice = physicalDevice;
    context->_device = device;
    context->_queueFamilyIndex = queueFamilyIndex;
    context->_queue = queue;
    context->_ownsDevice = false;
    // Create our own command pool on the adopted device.
    context->createCommandPool();
    return context;
}

VkContext::VkContext(bool enableValidation) :
    _enableValidation(enableValidation)
{
    // Create the instance.
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MaterialXRenderVk";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 39, 6);
    appInfo.pEngineName = "MaterialX";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 39, 6);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions;
    // Try to enable the portability enumeration flag (needed on some drivers).
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    if (_enableValidation)
    {
        if (checkValidationLayerSupport())
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            _enableValidation = false;
        }
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
    if (result != VK_SUCCESS)
    {
        // Retry without the portability flag.
        createInfo.flags = 0;
        result = vkCreateInstance(&createInfo, nullptr, &_instance);
        if (result != VK_SUCCESS)
        {
            throw ExceptionRenderError("Failed to create Vulkan instance: " + vkResultString(result));
        }
    }

    if (_enableValidation)
    {
        createDebugUtilsMessenger(_instance, &_debugMessenger);
    }
}

VkContext::~VkContext()
{
    if (_device != VK_NULL_HANDLE && _commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(_device, _commandPool, nullptr);
    }
    if (_ownsDevice)
    {
        if (_device != VK_NULL_HANDLE)
            vkDestroyDevice(_device, nullptr);
        if (_debugMessenger != VK_NULL_HANDLE)
            destroyDebugUtilsMessenger(_instance, _debugMessenger);
        if (_instance != VK_NULL_HANDLE)
            vkDestroyInstance(_instance, nullptr);
    }
}

void VkContext::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw ExceptionRenderError("Failed to find a Vulkan-capable physical device.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            _physicalDevice = device;
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE)
    {
        throw ExceptionRenderError("Failed to find a suitable Vulkan physical device.");
    }
}

void VkContext::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.imageCubeArray = VK_TRUE;
    deviceFeatures.geometryShader = VK_FALSE;
    deviceFeatures.tessellationShader = VK_FALSE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (_enableValidation)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
    if (result != VK_SUCCESS)
    {
        // Retry without the optional features (some drivers lack samplerAnisotropy).
        deviceFeatures.samplerAnisotropy = VK_FALSE;
        deviceFeatures.imageCubeArray = VK_FALSE;
        result = vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device);
        if (result != VK_SUCCESS)
        {
            throw ExceptionRenderError("Failed to create Vulkan logical device: " + vkResultString(result));
        }
    }

    _queueFamilyIndex = indices.graphicsFamily.value();
    vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &_queue);
}

void VkContext::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _queueFamilyIndex;

    VkResult result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
    if (result != VK_SUCCESS)
    {
        throw ExceptionRenderError("Failed to create Vulkan command pool: " + vkResultString(result));
    }
}

void VkContext::submitOneTimeCommands(const std::function<void(VkCommandBuffer)>& recorder)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    recorder(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void VkContext::waitIdle()
{
    if (_device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(_device);
}

uint32_t VkContext::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
    return MaterialX::findMemoryType(_physicalDevice, typeBits, properties);
}

VkFormat VkContext::findSupportedDepthFormat() const
{
    const VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
    };
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            return format;
        }
    }
    return VK_FORMAT_D32_SFLOAT;
}

MATERIALX_NAMESPACE_END
