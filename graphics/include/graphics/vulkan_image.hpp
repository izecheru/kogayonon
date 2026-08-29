#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{

struct ImageTransitionData
{
  VkPipelineStageFlags2 srcStage{ VK_PIPELINE_STAGE_2_NONE };
  VkPipelineStageFlags2 currentStage{ VK_PIPELINE_STAGE_2_NONE };
  VkAccessFlags2 srcAccess{ VK_ACCESS_2_NONE };
  VkAccessFlags2 currentAccess{ VK_ACCESS_2_NONE };
  VkImageLayout oldLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
  VkImageLayout currentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
  /**
   * @brief Defaults to VK_IMAGE_ASPECT_COLOR_BIT
   */
  VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
};

struct VulkanImage
{
  VkImage vkImage{ VK_NULL_HANDLE };
  VkImageView vkImageView{ VK_NULL_HANDLE };
  VkExtent2D extent{ 0, 0 };
  VmaAllocation vmaAllocation{ VK_NULL_HANDLE };
  ImageTransitionData transition;
};
} // namespace graphics