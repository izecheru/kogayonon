#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{
struct VulkanBuffer
{
  VkBuffer vkBuffer{ VK_NULL_HANDLE };
  VmaAllocation allocation{ nullptr };

  /**
   * @brief Is the buffer remaining mapped to memory untill the verry end?
   */
  bool persistent{ false };

  /**
   * @brief If the buffer is persistent, we need to store the pData here
   */
  void* mappedData{ nullptr };

  /**
   * @brief Is this buffer a staging buffer? If yes we manually vmaDestroyBuffer at the end of current scope
   */
  bool stagingBuff{ false };
};
} // namespace graphics