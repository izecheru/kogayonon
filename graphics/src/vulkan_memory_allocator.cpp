#define VMA_IMPLEMENTATION
#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/utils.hpp"
#include "utilities/utils/utils.hpp"

graphics::VulkanMemoryAllocator::VulkanMemoryAllocator( VkDevice device,
                                                        VkInstance instance,
                                                        VkPhysicalDevice physicalDevice )
    : m_vulkanFunctions{}
    , m_allocator{}
    , m_device{ device }
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
  deallocate();
#ifdef PRINT_LEAKS
  printLeaks();
#endif
  vmaDestroyAllocator( m_allocator );
}

void graphics::VulkanMemoryAllocator::createBuffer( VulkanBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage,
                                                    std::string_view bufferName )
{
  VK_CALL(
    vmaCreateBuffer( m_allocator, &createInfo, &usage, &vulkanBuffer.vkBuffer, &vulkanBuffer.allocation, nullptr ) );

  if ( !bufferName.empty() )
  {
    K_INFO( "[BUFF_ALLOC] {} {}", bufferName, formatSize( static_cast<double>( createInfo.size ) ) );
    setName( bufferName, vulkanBuffer.allocation );
  }
  else
  {

    K_INFO( "[BUFF_ALLOC] {}", formatSize( static_cast<double>( createInfo.size ) ) );
  }
}

void graphics::VulkanMemoryAllocator::createBuffers( FrameInFlightVulkanBuffer& vulkanBuffer,
                                                     VkBufferCreateInfo& createInfo,
                                                     VmaAllocationCreateInfo& usage )
{
  for ( auto& buffer : vulkanBuffer.buffers )
  {
    createBuffer( buffer, createInfo, usage );
  }
}

void graphics::VulkanMemoryAllocator::createImage( VkImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   VmaAllocation& allocation,
                                                   std::string_view imageName )
{
  VK_CALL( vmaCreateImage( m_allocator, &imageCreateInfo, &usage, &image, &allocation, nullptr ) );

  if ( !imageName.empty() )
  {
    K_INFO( "[IMG_ALLOC] {} size {}", imageName, formatSize( static_cast<double>( allocation->GetSize() ) ) );
    setName( imageName, allocation );
  }
  else
  {
    K_INFO( "[IMG_ALLOC] size {}", formatSize( static_cast<double>( allocation->GetSize() ) ) );
  }
}

auto graphics::VulkanMemoryAllocator::getAllocator() -> VmaAllocator&
{
  return m_allocator;
}

auto graphics::VulkanMemoryAllocator::createStagingBuffer( VkBufferCreateInfo& createInfo,
                                                           VmaAllocationCreateInfo& usage ) -> VulkanBuffer
{
  VulkanBuffer stageBuffer{};
  createBuffer( stageBuffer, createInfo, usage );
  return stageBuffer;
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

void graphics::VulkanMemoryAllocator::mapBuffer( VulkanBuffer& vulkanBuffer ) const
{
  auto info = getAllocInfo( vulkanBuffer.allocation );
  if ( info.pName )
  {
    K_INFO( "Buffer {} mapped", info.pName );
  }
  vmaMapMemory( m_allocator, vulkanBuffer.allocation, &vulkanBuffer.mapped );
  vulkanBuffer.flags |= Persistent | Mapped;
}

void graphics::VulkanMemoryAllocator::mapBuffer( FrameInFlightVulkanBuffer& vulkanBuffer ) const
{
  for ( auto& buff : vulkanBuffer.buffers )
  {
    vmaMapMemory( m_allocator, buff.allocation, &buff.mapped );
  }
}

void graphics::VulkanMemoryAllocator::unmapBuffer( VulkanBuffer& vulkanBuffer ) const
{
  auto info = getAllocInfo( vulkanBuffer.allocation );
  if ( info.pName )
  {
    K_INFO( "Unmapping buffer {}", info.pName );
  }
  vmaUnmapMemory( m_allocator, vulkanBuffer.allocation );
}

void graphics::VulkanMemoryAllocator::unmapBuffer( FrameInFlightVulkanBuffer& vulkanBuffer ) const
{
  for ( auto& buff : vulkanBuffer.buffers )
  {
    vmaUnmapMemory( m_allocator, buff.allocation );
  }
}

auto graphics::VulkanMemoryAllocator::destroyImage( VkImage& image, VmaAllocation& allocation ) -> void
{
  if ( image == VK_NULL_HANDLE )
    return;

  auto info = getAllocInfo( allocation );
  if ( info.pName )
  {
    K_INFO( "[IMG_DEALLOC] {}, size {}", info.pName, formatSize( info.size ) );
  }

  vmaDestroyImage( m_allocator, image, allocation );
}

auto graphics::VulkanMemoryAllocator::destroyImages(
  const std::initializer_list<std::tuple<VkImage&, VmaAllocation&>>& images ) -> void
{
  for ( auto& [image, alloc] : images )
  {
    destroyImage( image, alloc );
  }
}

auto graphics::VulkanMemoryAllocator::printLeaks() const -> void
{
  char* statsString = nullptr;
  VmaAllocatorCreateInfo info{};
  vmaBuildStatsString( m_allocator, &statsString, VK_TRUE );
  printf_s( "%s", statsString );
  // K_INFO( statsString );
  vmaFreeStatsString( m_allocator, statsString );
}

auto graphics::VulkanMemoryAllocator::setName( std::string_view name, VmaAllocation& allocation ) const -> void
{
#ifndef _DEBUG
  return;
#endif

  auto allocName = std::string{ name };
  vmaSetAllocationName( m_allocator, allocation, allocName.c_str() );
}

auto graphics::VulkanMemoryAllocator::destroyBuffer( VulkanBuffer& buff ) -> void
{
  auto info = getAllocInfo( buff.allocation );

  if ( info.pName )
  {
    K_INFO( "[BUFF_DEALLOC] {}, size {}", info.pName, formatSize( info.size ) );
  }

  vmaDestroyBuffer( m_allocator, buff.vkBuffer, buff.allocation );
}

auto graphics::VulkanMemoryAllocator::destroyBuffer( FrameInFlightVulkanBuffer& buff ) -> void
{
  for ( auto& buff : buff.buffers )
  {
    destroyBuffer( buff );
  }
}

auto graphics::VulkanMemoryAllocator::getAllocInfo( VmaAllocation allocation ) const -> VmaAllocationInfo
{
  VmaAllocationInfo info{};
  vmaGetAllocationInfo( m_allocator, allocation, &info );
  return info;
}
