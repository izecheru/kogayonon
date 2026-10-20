#pragma once
#include "vulkan/vulkan_core.h"

namespace rendering
{
struct ModuleRenderingInfo
{
    VkRenderingInfo vkRenderingInfo{};
    VkRenderingAttachmentInfo colorAttachmentInfo{};
    VkRenderingAttachmentInfo depthAttachmentInfo{};
};
} // namespace rendering