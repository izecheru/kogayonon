#define VMA_IMPLEMENTATION
#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/utils.hpp"
#include "utilities/utils/utils.hpp"

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

void graphics::VulkanMemoryAllocator::createBuffer( GpuBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage,
                                                    bool mapBuffer )
{
  VK_CALL(
    vmaCreateBuffer( m_allocator, &createInfo, &usage, &vulkanBuffer.vkBuffer, &vulkanBuffer.allocation, nullptr ) );

  if ( vulkanBuffer.stagingBuffer )
  {
    KOGAYONON_INFO( "Allocating staging buffer: {}", formatSize( static_cast<double>( createInfo.size ) ) );
    return;
  }

  if ( mapBuffer )
  {
    vmaMapMemory( m_allocator, vulkanBuffer.allocation, &vulkanBuffer.mappedData );
    vulkanBuffer.persistent = true;
  }

  KOGAYONON_INFO( "Allocating buffer: {}", formatSize( static_cast<double>( createInfo.size ) ) );
  m_deleteQueue.emplace( [&]() {
    if ( vulkanBuffer.deallocated )
      return;

    // unmap memory before deleting
    if ( vulkanBuffer.persistent )
    {
      vmaUnmapMemory( m_allocator, vulkanBuffer.allocation );
    }
    vmaDestroyBuffer( m_allocator, vulkanBuffer.vkBuffer, vulkanBuffer.allocation );
    KOGAYONON_INFO( "Deallocating buffer: {}",
                    formatSize( static_cast<double>( vulkanBuffer.allocation->GetSize() ) ) );
  } );
}

void graphics::VulkanMemoryAllocator::createBuffers( FrameInFlightBuffer& vulkanBuffer,
                                                     VkBufferCreateInfo& createInfo,
                                                     VmaAllocationCreateInfo& usage,
                                                     bool mapBuffer )
{
  for ( auto& buffer : vulkanBuffer.buffers )
  {
    createBuffer( buffer, createInfo, usage, mapBuffer );
  }
}

void graphics::VulkanMemoryAllocator::createImage( VkImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   VmaAllocation& allocation )
{
  VK_CALL( vmaCreateImage( m_allocator, &imageCreateInfo, &usage, &image, &allocation, nullptr ) );
  KOGAYONON_INFO( "Allocating image: {}", formatSize( static_cast<double>( allocation->GetSize() ) ) );
  m_deleteQueue.emplace( [&]() {
    KOGAYONON_INFO( "Dellocating image: {}", formatSize( static_cast<double>( allocation->GetSize() ) ) );
    vmaDestroyImage( m_allocator, image, allocation );
  } );
}

auto graphics::VulkanMemoryAllocator::getAllocator() -> VmaAllocator&
{
  return m_allocator;
}

auto graphics::VulkanMemoryAllocator::createStagingBuffer( VkBufferCreateInfo& createInfo,
                                                           VmaAllocationCreateInfo& usage ) -> GpuBuffer
{
  GpuBuffer stageBuffer{ .persistent = false, .stagingBuffer = true };
  createBuffer( stageBuffer, createInfo, usage, false );
  return stageBuffer;
}

void graphics::VulkanMemoryAllocator::deallocate()
{
  // wait for everything to complete
  VK_CALL( vkDeviceWaitIdle( *m_pDevice ) );

  while ( !m_deleteQueue.empty() )
  {
    m_deleteQueue.front()();
    m_deleteQueue.pop();
  }
}

auto graphics::VulkanMemoryAllocator::formatSize( VkDeviceSize size ) -> std::string
{
  const double KB = 1024.0;
  const double MB = KB * 1024.0;
  const double GB = MB * 1024.0;

  std::ostringstream oss;
  oss << std::fixed << std::setprecision( 2 );

  if ( size >= GB )
    oss << ( size / GB ) << " GB";
  else if ( size >= MB )
    oss << ( size / MB ) << " MB";
  else if ( size >= KB )
    oss << ( size / KB ) << " KB";
  else
    oss << size << " B";

  return oss.str();
}
