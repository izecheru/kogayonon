#pragma once
#include "graphics/vulkan_defines.hpp"
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
   * @brief Prevents a second deallocation in the lambda queue in VulkanMemoryAllocator
   */
  bool deallocated{ false };

  /**
   * @brief If the buffer is persistent, we need to store the pData here
   */
  void* mappedData{ nullptr };

  /**
   * @brief Is this buffer a staging buffer? If yes we manually vmaDestroyBuffer at the end of current scope
   */
  bool stagingBuffer{ false };
};

/**
 * @brief This is used for buffers like camera, ssbos and what not that need to be written either by cpu or gpu at the
 * same time and to avoid overwriting information, we just index into the currentFrameIndex variable from the swapchain
 * to avoid that so we got ourselves a simple sync mechanism
 */
struct FrameInFlightVulkanBuffer
{
  std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT> buffers;
};

} // namespace graphics