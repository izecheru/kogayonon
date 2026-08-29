#pragma once
#include "vulkan/vulkan.h"

namespace rendering
{
struct ModuleRenderingInfo
{
  VkRenderingInfo vkRenderingInfo{};
  VkRenderingAttachmentInfo colorAttachmentInfo{};
  VkRenderingAttachmentInfo depthAttachmentInfo{};
};
} // namespace rendering