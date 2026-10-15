#define VMA_IMPLEMENTATION

#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/utils.hpp"
#include "utilities/utils/utils.hpp"

graphics::VulkanMemoryAllocator::VulkanMemoryAllocator( VkDevice device,
                                                        VkPhysicalDevice physicalDevice,
                                                        VkInstance instance )
    : m_vulkanFunctions{}
    , m_device{ device }
    , m_instance{ instance }
    , m_physicalDevice{ physicalDevice }
    , m_totalImageAllocationSize{ 0u }
    , m_totalBufferAllocationSize{ 0u }
{
    m_vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    m_vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};

    allocatorCreateInfo.flags =
        VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &m_vulkanFunctions;

    if ( VK_CALL( vmaCreateAllocator( &allocatorCreateInfo, &m_allocator ) ) != VK_SUCCESS )
    {
        throw std::runtime_error( "VmaAllocator could not be created" );
    }

    createGlobalBuffers();
}

graphics::VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
    destroyBuffer( m_globalIndexBuffer );
    destroyBuffer( m_globalVertexBuffer );

    writeLeaksLog();
    vmaDestroyAllocator( m_allocator );
}

auto graphics::VulkanMemoryAllocator::createBuffer( VulkanBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage,
                                                    std::string_view bufferName ) -> VkResult
{
    VkResult result = VK_CALL( vmaCreateBuffer(
        m_allocator, &createInfo, &usage, &vulkanBuffer.vkBuffer, &vulkanBuffer.vmaAllocation, nullptr ) );

    if ( result != VK_SUCCESS )
    {
        return result;
    }

    if ( createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT )
    {
        vulkanBuffer.deviceAdress = getBufferDeviceAddressInfo( vulkanBuffer );
    }

    if ( !bufferName.empty() )
    {
#ifdef VMA_ALLOCATION_LOGS
        KINFO( "[Buffer] {} {}", bufferName, formatSize( static_cast<double>( createInfo.size ) ) );
#endif
        setAllocationName( bufferName, vulkanBuffer.vmaAllocation );
    }
#ifdef VMA_ALLOCATION_LOGS
    else
    {
        KINFO( "[Buffer] {}", formatSize( static_cast<double>( createInfo.size ) ) );
    }
#endif

    m_totalBufferAllocationSize += createInfo.size;

    return VK_SUCCESS;
}

auto graphics::VulkanMemoryAllocator::createBuffer( FrameInFlightVulkanBuffer& vulkanBuffer,
                                                    VkBufferCreateInfo& createInfo,
                                                    VmaAllocationCreateInfo& usage,
                                                    std::string_view name ) -> VkResult
{
    uint32_t index{ 0u };
    VkResult firstError{ VK_SUCCESS };

    for ( auto& buffer : vulkanBuffer.buffers )
    {
        std::string buffName = std::string{ name } + std::to_string( index++ );
        VkResult result = createBuffer( buffer, createInfo, usage, buffName );
        if ( result != VK_SUCCESS && firstError == VK_SUCCESS )
        {
            firstError = result;
        }
    }

    if ( firstError != VK_SUCCESS )
    {
        KERROR( "Could not initialize the buffer" );
        destroyBuffer( vulkanBuffer );
        return firstError;
    }

    return VK_SUCCESS;
}

auto graphics::VulkanMemoryAllocator::createImage( VkImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   VmaAllocation& allocation,
                                                   std::string_view imageName ) -> VkResult
{
    VkResult result = VK_CALL( vmaCreateImage( m_allocator, &imageCreateInfo, &usage, &image, &allocation, nullptr ) );

    if ( result != VK_SUCCESS )
    {
        destroyImage( image, allocation );
        return result;
    }

    VkDeviceSize size_ = static_cast<double>( allocation->GetSize() );

    m_totalImageAllocationSize += size_;

    std::string size = formatSize( size_ );
    if ( !imageName.empty() )
    {
#ifdef VMA_ALLOCATION_LOGS
        KINFO( "[Image] {} size {}", imageName, size );
#endif
        setAllocationName( imageName, allocation );
    }
#ifdef VMA_ALLOCATION_LOGS
    else
    {
        KINFO( "[Image] size {}", size );
    }
#endif

    return VK_SUCCESS;
}

auto graphics::VulkanMemoryAllocator::createImage( VulkanImage& image,
                                                   VkImageCreateInfo& imageCreateInfo,
                                                   VmaAllocationCreateInfo& usage,
                                                   std::string_view imageName ) -> VkResult
{
    return createImage( image.vkImage, imageCreateInfo, usage, image.vmaAllocation, imageName );
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
    {
        oss << ( size / GB ) << " GB";
    }
    else if ( size >= MB )
    {
        oss << ( size / MB ) << " MB";
    }
    else if ( size >= KB )
    {
        oss << ( size / KB ) << " KB";
    }
    else
    {
        oss << size << " B";
    }

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
    if ( !image || !allocation )
        return;

    VmaAllocationInfo info = getAllocInfo( allocation );

#ifdef VMA_ALLOCATION_LOGS
    if ( info.pName )
    {
        KINFO( "[~Image] {} size {}", info.pName, formatSize( info.size ) );
    }
    else
    {
        KINFO( "[~Image]  size {}", formatSize( info.size ) );
    }
#endif;

    m_totalImageAllocationSize -= info.size;
    vmaDestroyImage( m_allocator, image, allocation );
}

auto graphics::VulkanMemoryAllocator::destroyImages( const std::vector<std::tuple<VkImage&, VmaAllocation&>>& images )
    -> void
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

    p = p / "log.txt";

    if ( fs::exists( p ) )
    {
        fs::remove( p );
    }

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
    if ( !buff.vkBuffer || !buff.vmaAllocation )
        return;

    VmaAllocationInfo info{ getAllocInfo( buff.vmaAllocation ) };

#ifdef VMA_ALLOCATION_LOGS
    if ( info.pName )
    {
        KINFO( "[~Buffer] {} size {}", info.pName, formatSize( info.size ) );
    }
    else
    {
        KINFO( "[~Buffer] size {}", formatSize( info.size ) );
    }
#endif

    m_totalBufferAllocationSize -= info.size;

    vmaDestroyBuffer( m_allocator, buff.vkBuffer, buff.vmaAllocation );
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
                                                       std::string_view imageViewName ) const -> VkResult
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

auto graphics::VulkanMemoryAllocator::createImageView( VulkanImage& image,
                                                       VkFormat format,
                                                       VkImageAspectFlags aspectFlags,
                                                       std::string_view imageViewName ) const -> VkResult
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return VK_CALL( vkCreateImageView( m_device, &viewInfo, nullptr, &image.vkImageView ) );
}

auto graphics::VulkanMemoryAllocator::createGlobalBuffers() -> void
{
    // VkBufferCreateInfo stageBufferCreateInfo{};
    // stageBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // stageBufferCreateInfo.size = 500 * 1024 * 1024; // 500mb
    // stageBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    // stageBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // VmaAllocationCreateInfo stageAllocInfo{};
    // stageAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    // createBuffer( m_globalStagingBuffer, stageBufferCreateInfo, stageAllocInfo, "global_staging_buffer" );

    // 250mb each
    VkBufferCreateInfo vertexBufferCreateInfo{};
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.size = 250 * 1024 * 1024;
    vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaVertexAllocInfo{};
    vmaVertexAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaVertexAllocInfo.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBufferCreateInfo indexBufferCreateInfo{};
    indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferCreateInfo.size = 250 * 1024 * 1024;
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaIndexAllocInfo{};
    vmaIndexAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaIndexAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    createBuffer( m_globalVertexBuffer, vertexBufferCreateInfo, vmaVertexAllocInfo, "global_vertex_buffer" );
    createBuffer( m_globalIndexBuffer, indexBufferCreateInfo, vmaIndexAllocInfo, "global_index_buffer" );
}

auto graphics::VulkanMemoryAllocator::getGlobalVertexBuffer() -> graphics::VulkanBuffer&
{
    return m_globalVertexBuffer;
}

auto graphics::VulkanMemoryAllocator::getGlobalStaginBuffer() -> graphics::VulkanBuffer&
{
    return m_globalStagingBuffer;
}

auto graphics::VulkanMemoryAllocator::getGlobalIndexBuffer() -> graphics::VulkanBuffer&
{
    return m_globalIndexBuffer;
}

auto graphics::VulkanMemoryAllocator::getBufferDeviceAddressInfo( graphics::VulkanBuffer& buffer ) const
    -> VkDeviceAddress
{
    VkBufferDeviceAddressInfo info{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffer.vkBuffer };
    return vkGetBufferDeviceAddress( m_device, &info );
}

auto graphics::VulkanMemoryAllocator::getBuffMutex() -> std::mutex&
{
    return m_buffMutex;
}

auto graphics::VulkanMemoryAllocator::unmapBuffer( graphics::VulkanBuffer& buff ) const -> void
{
    vmaUnmapMemory( m_allocator, buff.vmaAllocation );
}

auto graphics::VulkanMemoryAllocator::mapBuffer( graphics::VulkanBuffer& buff, void** ppData ) const -> VkResult
{
    return VK_CALL( vmaMapMemory( m_allocator, buff.vmaAllocation, ppData ) );
}

auto graphics::VulkanMemoryAllocator::getTotalImageAllocationSize() const -> VkDeviceSize
{
    return m_totalImageAllocationSize;
}

auto graphics::VulkanMemoryAllocator::getTotalBufferAllocationSize() const -> VkDeviceSize
{
    return m_totalBufferAllocationSize;
}
