//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkUtil.h>

#include <MaterialXRenderVk/VkContext.h>

#include <MaterialXFormat/Util.h>

MATERIALX_NAMESPACE_BEGIN

std::string vkResultString(VkResult result)
{
    switch (result)
    {
#define CASE(r) case r: return #r
        CASE(VK_SUCCESS);
        CASE(VK_NOT_READY);
        CASE(VK_TIMEOUT);
        CASE(VK_EVENT_SET);
        CASE(VK_EVENT_RESET);
        CASE(VK_INCOMPLETE);
        CASE(VK_ERROR_OUT_OF_HOST_MEMORY);
        CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        CASE(VK_ERROR_INITIALIZATION_FAILED);
        CASE(VK_ERROR_DEVICE_LOST);
        CASE(VK_ERROR_MEMORY_MAP_FAILED);
        CASE(VK_ERROR_LAYER_NOT_PRESENT);
        CASE(VK_ERROR_EXTENSION_NOT_PRESENT);
        CASE(VK_ERROR_FEATURE_NOT_PRESENT);
        CASE(VK_ERROR_INCOMPATIBLE_DRIVER);
        CASE(VK_ERROR_TOO_MANY_OBJECTS);
        CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
        CASE(VK_ERROR_FRAGMENTED_POOL);
        CASE(VK_ERROR_OUT_OF_POOL_MEMORY);
        CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE);
        CASE(VK_ERROR_SURFACE_LOST_KHR);
        CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
        CASE(VK_SUBOPTIMAL_KHR);
        CASE(VK_ERROR_OUT_OF_DATE_KHR);
        CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
        CASE(VK_ERROR_VALIDATION_FAILED_EXT);
        CASE(VK_ERROR_INVALID_SHADER_NV);
        CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
        CASE(VK_ERROR_NOT_PERMITTED_KHR);
        CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
        CASE(VK_THREAD_IDLE_KHR);
        CASE(VK_THREAD_DONE_KHR);
        CASE(VK_OPERATION_DEFERRED_KHR);
        CASE(VK_OPERATION_NOT_DEFERRED_KHR);
        CASE(VK_PIPELINE_COMPILE_REQUIRED_EXT);
#undef CASE
        default: return "VK_UNKNOWN_RESULT_" + std::to_string(static_cast<int>(result));
    }
}

std::string vkResultString(vk::Result result)
{
    return vkResultString(static_cast<VkResult>(result));
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    return UINT32_MAX;
}

void transitionImageLayout(VkContextPtr context, VkImage image, VkFormat format,
                           VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    (void)format; // depth/stencil aspect is inferred from newLayout below.
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    context->submitOneTimeCommands([&](VkCommandBuffer cb)
    {
        cmd = cb;

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        // Depth attachment case.
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }

        vkCmdPipelineBarrier(cb, sourceStage, destinationStage, 0,
                             0, nullptr, 0, nullptr, 1, &barrier);
    });
    (void)cmd;
}

void submitOneTimeCommands(VkContextPtr context, const std::function<void(VkCommandBuffer)>& recorder)
{
    context->submitOneTimeCommands(recorder);
}

VkBool32 VKAPI_PTR debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    (void)messageTypes;
    (void)pUserData;

    const char* severity = "UNKNOWN";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        severity = "ERROR";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        severity = "WARN";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        severity = "INFO";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        severity = "VERBOSE";

    // Only surface errors and warnings by default; info/verbose is too noisy.
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "Vulkan validation [" << severity << "]: " << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

MATERIALX_NAMESPACE_END
