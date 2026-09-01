#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{
struct VulkanImage
{
  VkImage vkImage{ VK_NULL_HANDLE };
  VkImageView vkImageView{ VK_NULL_HANDLE };
  VkExtent2D extent{ 0, 0 };
  VmaAllocation vmaAllocation{ VK_NULL_HANDLE };

  VkImageMemoryBarrier2 currentState{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .pNext = nullptr,
    .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
    .srcAccessMask = VK_ACCESS_2_NONE,
    .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
    .dstAccessMask = VK_ACCESS_2_NONE,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = VK_NULL_HANDLE,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_NONE, .baseMipLevel = 0, .levelCount = 0, .baseArrayLayer = 0, .layerCount = 0 } };
};
} // namespace graphics