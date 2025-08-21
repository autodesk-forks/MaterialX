#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <MaterialXRenderVk/VkCommon.h>
#include <MaterialXRenderVk/VkBuffer.h>
#include <memory>
#include <vector>
#include <functional>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>


class VulkanSwapchain;
class VulkanRenderList;
class VulkanRenderPass;
class VulkanMaterialX;
class VulkanDeviceBuffer;
class VulkanHostBuffer;
class VulkanRenderTarget;
class VulkanTexture;

/// Vulkan Device shared pointer
using VulkanDevicePtr = std::shared_ptr<class VulkanDevice>;

class VulkanDevice : public std::enable_shared_from_this<VulkanDevice>
{
    public:
    /// Create a new context
    static VulkanDevicePtr create()
    {
        std::vector<const char*> vkExtensions = {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        };
        return VulkanDevicePtr(new VulkanDevice(vkExtensions));
    }

    
    virtual ~VulkanDevice();

    //REQUIRED Methods:
    /// InitializeDevice creates a swapchain for now, todo move it out 
    void InitializeDevice(vk::SurfaceKHR windowSurface, uint32_t width, uint32_t height);

    vk::Semaphore createSemaphore();

    vk::Fence createFence(bool signaled);
    
     struct BufferData
    {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::DeviceSize size;

        void destroy(vk::Device device)
        {
            if (buffer)
                device.destroyBuffer(buffer);
            if (memory)
                device.freeMemory(memory);
        }
    };
    
    /// createGraphicsPipeline helper functions
    BufferData createBuffer(vk::DeviceSize size,
                             vk::BufferUsageFlags usage,
                            vk::MemoryPropertyFlags properties);
    vk::DescriptorSetLayout VulkanDevice::createDescriptorSetLayout(
        std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
        vk::DescriptorSetLayoutCreateFlags flags = {});

    vk::DescriptorPool createDescriptorPool(std::vector<vk::DescriptorPoolSize> const& poolSizes);


    void updateDescriptorSets( vk::DescriptorSet const& descriptorSet, std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize, vk::BufferView const&>> const& bufferData, uint32_t bindingOffset);

    vk::Pipeline createGraphicsPipeline(vk::PipelineCache const& pipelineCache,
                                    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
                                    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData,
                                    uint32_t vertexStride,
                                    std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset,
                                    vk::FrontFace frontFace,
                                    bool depthBuffered,
                                    vk::PipelineLayout const& pipelineLayout,
                                        vk::RenderPass const& renderPass);
    glm::mat4x4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);
    glm::mat4x4 perspective(float fovY, float aspect, float nearP, float farP);
    glm::mat4x4 createModelViewProjectionClipMatrix(vk::Extent2D const& extent);

    void copyToDevice(vk::DeviceMemory memory, const void* src, size_t size)
    {
        void* mappedMemory = _device.mapMemory(memory, 0, size);
        std::memcpy(mappedMemory, src, size);
        _device.unmapMemory(memory);
    }

    vk::ShaderModule createShaderModule(vk::ShaderStageFlagBits shaderStage, std::string const &shaderText);
    vk::RenderPass createRenderPass(vk::Format colorFormat, vk::Format depthFormat,  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
                                     vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR );
    vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats);
    std::vector<vk::Framebuffer> createFramebuffers(vk::RenderPass renderPass, const std::vector<vk::ImageView>& colorImageViews, vk::ImageView depthImageView, vk::Extent2D extent);


    std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const& surface);

    vk::Queue _graphicsQueue;
    vk::Queue _presentQueue;
    vk::DescriptorSet _descriptorSet;
    uint32_t _graphicsQueueFamilyIndex;

    // Getters

    vk::DescriptorSet getDescriptorSet() const;

    vk::Instance GetInstance() { return _instance; }
    vk::Device GetDevice() { return _device; }
    vk::PhysicalDevice GetPhysicalDevice() { return _physicalDevice; }
    void CreateCommandPool();
    uint32_t FindMemoryType(uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties);
    vk::SurfaceKHR GetSurface() { return _surface; }
    vk::PhysicalDeviceMemoryProperties GetMemoryProperties()
    {
        return _physicalDevice.getMemoryProperties();
    }
   

    struct SwapChainData
    {
        SwapChainData(vk::PhysicalDevice const& physicalDevice,
                      vk::Device const& device,
                      vk::SurfaceKHR const& surface,
                      vk::Extent2D const& extent,
                      vk::ImageUsageFlags usage,
                      vk::SwapchainKHR const& oldSwapChain,
                      uint32_t graphicsFamilyIndex,
                      uint32_t presentFamilyIndex);
        SwapChainData(){};

        void clear(vk::Device const& device);
        vk::Format _colorFormat;
        vk::SwapchainKHR _swapChain;
        std::vector<vk::Image> _images;
        std::vector<vk::ImageView> _imageViews;
        std::vector<vk::ImageLayout> _imageLayouts;

        const std::vector<vk::ImageView>& getImageViews() const { return _imageViews; }
    };


    VkDescriptorPool GetDescriptorPool(VkCommandBuffer commandBuffer);
    void AllocateNewDescriptorPool(VkCommandBuffer commandBuffer);
    void FreeDescriptorPools(VkCommandBuffer commandBuffer);


    VkFormatProperties FindFormatProperties(VkFormat format);

    VkSampleCountFlagBits GetMaxMultisampleCount();

    vk::CommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    bool QuerySwapChainSupport(SwapChainSupportDetails &details, VkPhysicalDevice pd = nullptr);
    VkSurfaceFormatKHR FindSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkSurfaceFormatKHR FindSurfaceFormat();

    void AddTextureToCache(std::string textureName, std::shared_ptr<VulkanTexture> texture) { textureCache[textureName] = texture; }
    std::shared_ptr<VulkanTexture> RetrieveTextureFromCache(std::string textureName) { auto it = textureCache.find(textureName); return (it == textureCache.end() ? nullptr : it->second); }
    const SwapChainData& getSwapChainData() const { return _swapChainData; }
    const std::vector<vk::ImageView>& VulkanDevice::getImageViews() const
    {
        return _swapChainData.getImageViews(); // Forward call to SwapChainData's getter
    }

    std::vector<vk::ImageLayout> _swapImageLayouts;

    vk::CommandBuffer getCommandBuffer() const
    {
        return _commandBuffer;
    }
    //vk::CommandPool 
    std::pair<uint32_t, uint32_t> getGraphicsAndPresentQueueIndex() const {
        return _graphicsandPresentQueueIndex;
    }
    // HELPER TEST
    vk::Semaphore _imageAvailableSemaphore;
    vk::Semaphore _renderingFinishedSemaphore;
    void clearScreen(vk::ImageView depthView);
    void CreateDevice();

    /// Resource Code
    VkBuffer createVertexBuffer(const void* data, size_t size); 
    VkBuffer createUniformBuffer(size_t size, const void* data);
    vk::DescriptorSet createDescriptorSet(vk::Buffer buffer);

    /// Pipeline Cache Management
    vk::PipelineCache getPipelineCache();
    void addFramebufferForCleanup(vk::Framebuffer framebuffer);
    void cleanupFramebuffers();

     vk::DescriptorSetLayout getDescriptorSetLayout() 
     {
         return _descriptorSetLayout;
     }
    const vk::PhysicalDevice& getPhysicalDevice() const { return _physicalDevice; }


    void FindPhysicalDevice();
    protected:
    VulkanDevice(std::vector<const char*> requestedExtensions);
    
    // REQUIRED METHODS
    void AppendValidationLayerExtension();
    void CreateInstance();
    void CreateDebugReportCallback();

  
  
    
    uint32_t GetQueueFamilyIndex(VkQueueFlagBits flags);
    
    vk::Queue getGraphicsQueue() const
    {
        return _graphicsQueue;
    }


    vk::Instance _instance;
    vk::PhysicalDevice _physicalDevice;
    vk::Device _device;
    vk::Queue _queue;
    vk::CommandPool _commandPool;
    vk::CommandBuffer _commandBuffer;
    std::pair<uint32_t, uint32_t> _graphicsandPresentQueueIndex;
    vk::DescriptorSetLayout _descriptorSetLayout;

    // set by renderer on InitializeDevice
    vk::SurfaceKHR _surface;
    SwapChainData _swapChainData;
    vk::Extent2D _surfaceExtent;

    std::map<VkCommandBuffer, std::vector<VkDescriptorPool>> _commandBufferDescriptorPools;
    
    VkExtent2D windowExtent;

    std::vector<const char *> layers;
    std::vector<const char *> extensions;

    bool enableValidationLayers;
    VkDebugReportCallbackEXT debugReportCallback;


    std::map<std::string, std::shared_ptr<VulkanTexture>> textureCache;

    // Resource cleanup tracking
    vk::PipelineCache _pipelineCache;
    std::vector<vk::Framebuffer> _framebuffers;
    
};