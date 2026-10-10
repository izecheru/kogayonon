#define VMA_IMPLEMENTATION

#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "utilities/utils/utils.hpp"

graphics::VulkanMemoryAllocator::VulkanMemoryAllocator( VkDevice device,
                                                        VkPhysicalDevice physicalDevice,
                                                        VkInstance instance )
    : m_vulkanFunctions{}
    , m_allocator{}
    , m_device{ device }
    , m_instance{ instance }
    , m_physicalDevice{ physicalDevice }
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
    writeLeaksLog();
    vmaDestroyAllocator( m_allocator );
}

void graphics::VulkanMemoryAllocator::createBuffer( VulkanBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage,
                                                    std::string_view bufferName )
{
    VK_CALL( vmaCreateBuffer(
        m_allocator, &createInfo, &usage, &vulkanBuffer.vkBuffer, &vulkanBuffer.vmaAllocation, nullptr ) );

    if ( !bufferName.empty() )
    {
        KINFO( "[Buffer] {} {}", bufferName, formatSize( static_cast<double>( createInfo.size ) ) );
        setAllocationName( bufferName, vulkanBuffer.vmaAllocation );
    }
    else
    {

        KINFO( "[Buffer] {}", formatSize( static_cast<double>( createInfo.size ) ) );
    }
}

void graphics::VulkanMemoryAllocator::createBuffer( FrameInFlightVulkanBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage,
                                                    std::string_view name )
{
    uint32_t index{ 0u };
    for ( auto& buffer : vulkanBuffer.buffers )
    {
        std::string buffName = std::string{ name } + std::to_string( index++ );
        createBuffer( buffer, createInfo, usage, buffName );
    }
}

void graphics::VulkanMemoryAllocator::createImage( VkImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   VmaAllocation& allocation,
                                                   std::string_view imageName )
{
    VK_CALL( vmaCreateImage( m_allocator, &imageCreateInfo, &usage, &image, &allocation, nullptr ) );

    std::string size = formatSize( static_cast<double>( allocation->GetSize() ) );
    if ( !imageName.empty() )
    {
        KINFO( "[Image] {} size {}", imageName, size );
        setAllocationName( imageName, allocation );
    }
    else
    {
        KINFO( "[Image] size {}", size );
    }
}

void graphics::VulkanMemoryAllocator::createImage( VulkanImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   std::string_view imageName )
{
    createImage( image.vkImage, imageCreateInfo, usage, image.vmaAllocation, imageName );
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

auto graphics::VulkanMemoryAllocator::invalidateAllocation( VulkanBuffer& vulkanBuffer,
                                                            VkDeviceSize offset,
                                                            VkDeviceSize size ) const -> void
{
    vmaInvalidateAllocation( m_allocator, vulkanBuffer.vmaAllocation, offset, size );
}

auto graphics::VulkanMemoryAllocator::destroyImage( VkImage& image, VmaAllocation& allocation ) -> void
{
    if ( !image )
        return;

    if ( !allocation )
        return;

    auto info = getAllocInfo( allocation );
    if ( info.pName )
    {
        KINFO( "[~Image] {} size {}", info.pName, formatSize( info.size ) );
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

auto graphics::VulkanMemoryAllocator::writeLeaksLog() const -> void
{
    namespace fs = std::filesystem;
    fs::path p{ fs::current_path() / "logs" };
    if ( !fs::exists( p ) )
    {
        fs::create_directory( p );
    }

    if ( fs::exists( p / "log.txt" ) )
    {
        fs::remove( p / "log.txt" );
    }

    p = p / "log.txt";
    std::ofstream out{ p, std::ios::out };

    char* statsString = nullptr;
    VmaAllocatorCreateInfo info{};
    vmaBuildStatsString( m_allocator, &statsString, VK_TRUE );
    out.write( statsString, strlen( statsString ) );

    if ( out.is_open() )
    {
        out.close();
    }

    vmaFreeStatsString( m_allocator, statsString );
}

auto graphics::VulkanMemoryAllocator::setAllocationName( std::string_view name, VmaAllocation& allocation ) const
    -> void
{
#ifndef _DEBUG
    return;
#endif

    auto allocName = std::string{ name };
    vmaSetAllocationName( m_allocator, allocation, allocName.c_str() );
}

auto graphics::VulkanMemoryAllocator::destroyBuffer( VulkanBuffer& buff ) -> void
{
    if ( !buff.vmaAllocation )
        return;

    auto info = getAllocInfo( buff.vmaAllocation );

    if ( info.pName )
    {
        KINFO( "[~Buffer] {} size {}", info.pName, formatSize( info.size ) );
    }
    else
    {
        KINFO( "[~Buffer] size {}", formatSize( info.size ) );
    }

    vmaDestroyBuffer( m_allocator, buff.vkBuffer, buff.vmaAllocation );

    buff.vkBuffer = VK_NULL_HANDLE;
    buff.vmaAllocation = VK_NULL_HANDLE;
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

auto graphics::VulkanMemoryAllocator::copyMemoryToAllocation( const void* pSrcHostPointer,
                                                              VmaAllocation dstAllocation,
                                                              VkDeviceSize dstAllocationLocalOffset,
                                                              VkDeviceSize size ) const -> VkResult
{
    return VK_CALL(
        vmaCopyMemoryToAllocation( m_allocator, pSrcHostPointer, dstAllocation, dstAllocationLocalOffset, size ) );
}

auto graphics::VulkanMemoryAllocator::createImageView( VkImageView& imageView,
                                                       VkImage& image,
                                                       VkFormat format,
                                                       VkImageAspectFlags aspectFlags,
                                                       std::string_view imageViewName ) -> VkResult
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return VK_CALL( vkCreateImageView( m_device, &viewInfo, nullptr, &imageView ) );
}
