#pragma once
#include <vulkan/vulkan.h>
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "utilities/utils/utils.hpp"

#ifdef TRACY_ENABLE
#include "graphics/vulkan_tracy_context.hpp"
#endif

namespace graphics
{

struct VulkanContext
{
    std::unique_ptr<VulkanDevice> device;
    std::unique_ptr<VulkanSwapchain> swapchain;

#ifdef TRACY_ENABLE
    std::unique_ptr<VulkanTracyContext> tracyContext;
#endif

    VkDescriptorPool globalDescriptorPool;
};
} // namespace graphics
