#define VMA_IMPLEMENTATION
#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/utils.hpp"

graphics::VulkanMemoryAllocator::VulkanMemoryAllocator( VkDevice& device,
                                                        VkInstance& instance,
                                                        VkPhysicalDevice& physicalDevice )
    : m_vulkanFunctions{}
    , m_allocator{}
    , m_pDevice{ &device }
{
  m_vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
  m_vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
  allocatorCreateInfo.physicalDevice = physicalDevice;
  allocatorCreateInfo.device = device;
  allocatorCreateInfo.instance = instance;
  allocatorCreateInfo.pVulkanFunctions = &m_vulkanFunctions;

  VK_CALL( vmaCreateAllocator( &allocatorCreateInfo, &m_allocator ) );
}

graphics::VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
  vmaDestroyAllocator( m_allocator );
}

void graphics::VulkanMemoryAllocator::createBuffer( VulkanBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage )
{
  VK_CALL(
    vmaCreateBuffer( m_allocator, &createInfo, &usage, &vulkanBuffer.vkBuffer, &vulkanBuffer.allocation, nullptr ) );

  if ( vulkanBuffer.stagingBuff )
    return;

  m_deleteQueue.push( [&]() {
    // unmap memory before deleting
    if ( vulkanBuffer.persistent )
    {
      vmaUnmapMemory( m_allocator, vulkanBuffer.allocation );
    }
    vmaDestroyBuffer( m_allocator, vulkanBuffer.vkBuffer, vulkanBuffer.allocation );
  } );
}

void graphics::VulkanMemoryAllocator::createImage( VkImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   VmaAllocation& allocation )
{
  VK_CALL( vmaCreateImage( m_allocator, &imageCreateInfo, &usage, &image, &allocation, nullptr ) );
  m_deleteQueue.push( [&]() { vmaDestroyImage( m_allocator, image, allocation ); } );
}

auto graphics::VulkanMemoryAllocator::getAllocator() -> VmaAllocator&
{
  return m_allocator;
}

auto graphics::VulkanMemoryAllocator::createStagingBuffer( VkBufferCreateInfo& createInfo,
                                                           VmaAllocationCreateInfo& usage ) -> VulkanBuffer
{
  VulkanBuffer stageBuffer{ .persistent = false, .stagingBuff = true };
  createBuffer( stageBuffer, createInfo, usage );
  return stageBuffer;
}

void graphics::VulkanMemoryAllocator::deallocate()
{
  // wait for everything to complete
  VK_CALL( vkDeviceWaitIdle( *m_pDevice ) );

  while ( !m_deleteQueue.empty() )
  {
    const std::function<void()>& func = m_deleteQueue.front();
    func();

    m_deleteQueue.pop();
  }
}
