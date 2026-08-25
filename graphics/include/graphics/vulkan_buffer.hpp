#pragma once
#include "graphics/vulkan_defines.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{

struct VulkanBuffer
{
  VkBuffer vkBuffer{ VK_NULL_HANDLE };
  VmaAllocation vmaAllocation{ nullptr };
  void* mappedData;
};

/**
 * @brief This is used for buffers like camera, ssbos and what not that need to be written either by cpu or gpu at the
 * same time and to avoid overwriting information, we just index into the currentFrameIndex variable from the swapchain
 * to avoid that so we got ourselves a simple sync mechanism
 */
struct FrameInFlightVulkanBuffer
{
  std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT> buffers;

  // this is for conveniently calling the setDebugName function when we want the buffer to also
  // hold a name for easier debugging experience
  auto each( std::function<void( VulkanBuffer&, uint32_t index )>&& func ) -> void
  {
    for ( auto i = 0; i < buffers.size(); ++i )
    {
      func( buffers.at( i ), i );
    }
  }

  auto each( std::function<void( VulkanBuffer& )>&& func ) -> void
  {
    for ( auto i = 0; i < buffers.size(); ++i )
    {
      func( buffers.at( i ) );
    }
  }
};

} // namespace graphics