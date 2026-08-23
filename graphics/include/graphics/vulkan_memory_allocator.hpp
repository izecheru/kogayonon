#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1004000

#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_image.hpp"
#include "precompiled/pch.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{

class VulkanMemoryAllocator
{
public:
  explicit VulkanMemoryAllocator( VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice );
  ~VulkanMemoryAllocator();

  auto printLeaks() const -> void;
  void createBuffer( VulkanBuffer& vulkanBuffer,
                     VkBufferCreateInfo& createInfo,
                     VmaAllocationCreateInfo& usage,
                     std::string_view bufferName = "" );

  void createBuffers( FrameInFlightVulkanBuffer& vulkanBuffer,
                      VkBufferCreateInfo& createInfo,
                      VmaAllocationCreateInfo& usage );

  auto createStagingBuffer( VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& usage ) -> VulkanBuffer;

  void mapBuffer( VulkanBuffer& vulkanBuffer ) const;
  void mapBuffer( FrameInFlightVulkanBuffer& vulkanBuffer ) const;
  void unmapBuffer( VulkanBuffer& vulkanBuffer ) const;
  void unmapBuffer( FrameInFlightVulkanBuffer& vulkanBuffer ) const;

  void createImage( VkImage& image,
                    VkImageCreateInfo& imageCreateInfo,
                    VmaAllocationCreateInfo& usage,
                    VmaAllocation& allocation,
                    std::string_view imageName = "" );

  auto destroyImage( VkImage& image, VmaAllocation& allocation ) -> void;
  auto destroyImages( const std::initializer_list<std::tuple<VkImage&, VmaAllocation&>>& images ) -> void;
  auto destroyBuffer( VulkanBuffer& buff ) -> void;
  auto destroyBuffer( FrameInFlightVulkanBuffer& buff ) -> void;
  auto getAllocator() -> VmaAllocator&;

  auto setName( std::string_view name, VmaAllocation& allocation ) const -> void;

private:
  auto formatSize( VkDeviceSize size ) -> std::string;
  auto getAllocInfo( VmaAllocation allocation ) const -> VmaAllocationInfo;

private:
  VmaVulkanFunctions m_vulkanFunctions;
  VmaAllocator m_allocator;
  VkDevice m_device;
};
} // namespace graphics