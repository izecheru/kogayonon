#include "core/asset_manager/threadsafe_resource_manager.hpp"
#include "graphics/vulkan_device.hpp"

core::ThreadsafeResourceManager::ThreadsafeResourceManager( uint32_t threads, graphics::VulkanDevice* vkDevice )
    : m_device{ vkDevice }
{
    allocateCommandPools( threads );
    allocateCommandBuffers( threads );
}

core::ThreadsafeResourceManager::~ThreadsafeResourceManager()
{
    for ( auto i = 0u; i < m_commandPools.size(); ++i )
    {
        m_device->destroyCommandPool( m_commandPools[i] );
    }
}

auto core::ThreadsafeResourceManager::getCommandPool( uint32_t threadNum ) -> VkCommandPool
{
    std::lock_guard lock{ m_mutex };
    return m_commandPools[threadNum];
}

auto core::ThreadsafeResourceManager::allocateCommandPools( uint32_t threadCount ) -> void
{
    m_commandPools.resize( threadCount );
    for ( auto i = 0u; i < threadCount; i++ )
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_device->getTransferQueue().familyIndex;
        m_device->createCommandPool( poolInfo, m_commandPools[i] );
    }
}

auto core::ThreadsafeResourceManager::allocateCommandBuffers( uint32_t threadCount ) -> void
{
    for ( auto i = 0u; i < threadCount; i++ )
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandPool = m_commandPools[i];
        allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
        std::vector<VkCommandBuffer> cmdBuffers =
            m_device->allocateCommandBuffers( allocInfo, m_commandPools[i], MAX_FRAMES_IN_FLIGHT );

        for ( auto i = 0u; i < cmdBuffers.size(); i++ )
        {
            m_commandBuffers.emplace_back( false, cmdBuffers[i] );
        }
    }
}

auto core::ThreadsafeResourceManager::getCommandBuffer( uint32_t threadNum ) -> VkCommandBuffer
{
    // thread num is the index in the command pool
    // each pool has 3 buffers, if they are busy enqueue it

    uint32_t buffIndex = threadNum;

    {
        std::lock_guard lock{ m_mutex };
        for ( auto i = buffIndex; i <= buffIndex + 2; i++ )
        {
            auto& [isBusy, cmdBuffer] = m_commandBuffers[i];
            if ( !isBusy )
            {
                return cmdBuffer;
            }
        }
    }

    throw std::runtime_error( "all buffers are busy rn" );
}
