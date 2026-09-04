//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VKUTIL_H
#define MATERIALX_VKUTIL_H

/// @file
/// Vulkan utility helpers shared across the MaterialXRenderVk module.

#include <MaterialXRenderVk/Export.h>

#include <MaterialXRender/ShaderRenderer.h>

#include <vulkan/vulkan.hpp>

#include <functional>
#include <string>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

using VkContextPtr = std::shared_ptr<class VkContext>;

/// Convert a Vulkan result code to a human-readable string.
MX_RENDERVK_API std::string vkResultString(VkResult result);

/// Convert a vk::Result to a human-readable string (overload for the C++ binding).
MX_RENDERVK_API std::string vkResultString(vk::Result result);

/// Check a VkResult and throw ExceptionRenderError on failure.
/// The message is prefixed with `msg`; the Vulkan result string is appended.
#define VK_CHECK(result, msg)                                                  \
    do                                                                         \
    {                                                                          \
        VkResult __r = (result);                                               \
        if (__r != VK_SUCCESS)                                                 \
        {                                                                      \
            throw ExceptionRenderError(                                        \
                std::string(msg) + ": " + ::MaterialX::vkResultString(__r));   \
        }                                                                      \
    } while (0)

/// Check a vk::Result and throw ExceptionRenderError on failure.
#define VK_CHECK_HPP(result, msg)                                               \
    do                                                                         \
    {                                                                          \
        vk::Result __r = (result);                                             \
        if (__r != vk::Result::eSuccess)                                       \
        {                                                                      \
            throw ExceptionRenderError(                                        \
                std::string(msg) + ": " + ::MaterialX::vkResultString(__r));   \
        }                                                                      \
    } while (0)

/// Find a suitable memory type index for the given requirements and property flags.
/// Returns UINT32_MAX if no matching type is found.
MX_RENDERVK_API uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                                        uint32_t typeBits,
                                        VkMemoryPropertyFlags properties);

/// Transition an image from `oldLayout` to `newLayout` using a one-time command buffer
/// submitted to the queue owned by `context`. Performs the appropriate pipeline barrier.
MX_RENDERVK_API void transitionImageLayout(VkContextPtr context,
                                           VkImage image,
                                           VkFormat format,
                                           VkImageLayout oldLayout,
                                           VkImageLayout newLayout,
                                           uint32_t mipLevels = 1);

/// Record, submit, and wait on a single-use command buffer. The recorder lambda is
/// handed a begun command buffer; when it returns, the buffer is ended, submitted to
/// the context's queue, and waited on. Convenience for one-off setup operations.
MX_RENDERVK_API void submitOneTimeCommands(VkContextPtr context,
                                           const std::function<void(VkCommandBuffer)>& recorder);

/// Debug-utils messenger callback. Installed only in validation builds.
MX_RENDERVK_API VkBool32 VKAPI_PTR debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

MATERIALX_NAMESPACE_END

#endif
