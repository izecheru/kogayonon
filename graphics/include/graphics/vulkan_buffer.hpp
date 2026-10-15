#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace graphics
{

struct VulkanBuffer
{
    VkBuffer vkBuffer{ VK_NULL_HANDLE };
    VmaAllocation vmaAllocation{ VK_NULL_HANDLE };

    /**
     * @brief Semaphore signal value that this buffer waits on, if it is signaled we know
     * the GPU is done using this buffer or the commands that reference it are submitted and processed
     */
    uint64_t semaphoreSignal{ UINT64_MAX };

    /**
     * @brief Address of the buffer on the gpu
     */
    VkDeviceAddress deviceAdress{ 0u };

    /**
     * @brief If we write more things into the buffer we must know at which offset we last wrote
     * the layout of the buffer should be tracked elsewhere
     */
    VkDeviceSize lastWrittenOffset{ 0u };
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