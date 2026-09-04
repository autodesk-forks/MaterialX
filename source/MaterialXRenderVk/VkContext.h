//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKCONTEXT_H
#define MATERIALX_VKCONTEXT_H

/// @file
/// Vulkan device context class

#include <MaterialXRenderVk/Export.h>

#include <vulkan/vulkan.h>

#include <iostream>
#include <memory>
#include <optional>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a VkContext
using VkContextPtr = std::shared_ptr<class VkContext>;

/// Queue family indices used for device selection.
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    bool isComplete() const { return graphicsFamily.has_value(); }
};

/// Default: enable validation layers when building without NDEBUG (i.e. debug builds).
#ifndef MX_VK_DEFAULT_VALIDATION
#ifdef NDEBUG
#define MX_VK_DEFAULT_VALIDATION false
#else
#define MX_VK_DEFAULT_VALIDATION true
#endif
#endif

/// @class VkContext
/// Owns the Vulkan instance, physical device, logical device, queue and command
/// pool used by a renderer. Headless: no surface, no swapchain.
///
/// Per-instance and shared_ptr-owned (unlike MetalState, which is a process-wide
/// singleton). A global would make TextureBakerVk (which constructs a second
/// renderer) and the viewer fight over one device.
class MX_RENDERVK_API VkContext
{
  public:
    /// Create a context, throwing ExceptionRenderError on failure.
    static VkContextPtr create(bool enableValidation = MX_VK_DEFAULT_VALIDATION);

    /// Non-throwing probe for applications that must degrade gracefully.
    /// Returns nullptr on failure, logging diagnostics to `log` if non-null.
    static VkContextPtr tryCreate(std::ostream* log = nullptr) noexcept;

    /// Cheap cached query: does this machine expose a Vulkan device?
    /// Safe to call before create(). Used by the render tests to skip on headless CI.
    static bool isDeviceAvailable();

    /// Adopt an externally owned device, e.g. from a host application passing a
    /// RenderContextHandle to VkRenderer::initialize(). The context does not own
    /// (and will not destroy) the adopted device when `_ownsDevice` is false.
    static VkContextPtr createShared(VkInstance instance,
                                     VkPhysicalDevice physicalDevice,
                                     VkDevice device,
                                     uint32_t queueFamilyIndex,
                                     VkQueue queue);

    ~VkContext();

    VkInstance       getInstance()         const { return _instance; }
    VkPhysicalDevice getPhysicalDevice()   const { return _physicalDevice; }
    VkDevice         getDevice()           const { return _device; }
    VkQueue          getQueue()            const { return _queue; }
    uint32_t         getQueueFamilyIndex() const { return _queueFamilyIndex; }
    VkCommandPool    getCommandPool()      const { return _commandPool; }

    /// Allocate, begin, run, submit and wait on a single-use command buffer.
    void submitOneTimeCommands(const std::function<void(VkCommandBuffer)>& recorder);

    /// Wait for the queue to become idle.
    void waitIdle();

    /// Find a memory type matching `typeBits` and `properties`.
    uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

    /// Find a supported depth format (prefers D32_SFLOAT_S8_UINT, falls back to D24_UNORM_S8_UINT).
    VkFormat findSupportedDepthFormat() const;

    /// Whether validation layers are active on this context.
    bool validationEnabled() const { return _enableValidation; }

  protected:
    VkContext(bool enableValidation);

    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;
    uint32_t _queueFamilyIndex = 0;
    VkCommandPool _commandPool = VK_NULL_HANDLE;
    bool _enableValidation = false;
    bool _ownsDevice = true;
};

MATERIALX_NAMESPACE_END

#endif
