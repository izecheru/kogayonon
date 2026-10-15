#pragma once

template <typename T>
inline auto graphics::VulkanMemoryAllocator::updateBuffer( std::vector<T>& data,
                                                           VmaAllocation vmaAllocation,
                                                           VkDeviceSize offset ) -> void
{
    vmaCopyMemoryToAllocation( m_allocator, data.data(), vmaAllocation, offset, sizeof( T ) * data.size() );
}

template <typename T>
inline auto graphics::VulkanMemoryAllocator::copyFromBuffer( T& data, VulkanBuffer& buffer ) -> void
{
    vmaCopyMemoryToAllocation( m_allocator, &data, buffer.vmaAllocation, 0, sizeof( T ) );
}

template <typename T>
inline auto graphics::VulkanMemoryAllocator::copyToBuffer( T& data, VulkanBuffer& buffer ) -> void
{
    vmaCopyMemoryToAllocation( m_allocator, &data, buffer.vmaAllocation, 0, sizeof( T ) );
}

template <typename T>
inline auto graphics::VulkanMemoryAllocator::copyToBuffer( T& data, VulkanBuffer& buffer, VkDeviceSize size ) -> void
{
    vmaCopyMemoryToAllocation( m_allocator, &data, buffer.vmaAllocation, 0, size );
}

template <typename T>
inline auto graphics::VulkanMemoryAllocator::copyToBuffer( std::vector<T>& data, VulkanBuffer& buffer ) -> void
{
    VkDeviceSize size = sizeof( T ) * data.size();
    VmaAllocationInfo allocInfo{ getAllocInfo( buffer.vmaAllocation ) };

    if ( size > allocInfo.size )
    {
        throw std::runtime_error( "size is bigger, sry" );
    }
    vmaCopyMemoryToAllocation( m_allocator, data.data(), buffer.vmaAllocation, 0, size );
}
