#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1004000
#include "graphics/vulkan_buffer.hpp"
#include "precompiled/pch.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{

class VulkanMemoryAllocator
{
public:
  explicit VulkanMemoryAllocator( VkDevice& device, VkInstance& instance, VkPhysicalDevice& physicalDevice );
  ~VulkanMemoryAllocator();

  void createBuffer( GpuBuffer& vulkanBuffer,
                     VkBufferCreateInfo& createInfo,
                     VmaAllocationCreateInfo& usage,
                     bool mapBuffer );

  void createBuffers( FrameInFlightBuffer& vulkanBuffer,
                      VkBufferCreateInfo& createInfo,
                      VmaAllocationCreateInfo& usage,
                      bool mapBuffer );

  auto createStagingBuffer( VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& usage ) -> GpuBuffer;

  void createImage( VkImage& image,
                    VkImageCreateInfo& imageCreateInfo,
                    VmaAllocationCreateInfo& usage,
                    VmaAllocation& allocation );

  auto getAllocator() -> VmaAllocator&;

  void deallocate();

private: // Funcs
  auto formatSize( VkDeviceSize size ) -> std::string;

private:
  VmaVulkanFunctions m_vulkanFunctions;
  VmaAllocator m_allocator;
  // TODO(kogayonon) this should probably hold a queue of things that should be destroyed by the allocator
  // and this would just store lambdas for deleting resources allocated using VMA
  std::queue<std::function<void()>> m_deleteQueue;
  VkDevice* m_pDevice;
};
} // namespace graphics