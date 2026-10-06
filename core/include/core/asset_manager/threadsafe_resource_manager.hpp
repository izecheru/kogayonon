#pragma once
#include <vulkan/vulkan_core.h>

namespace graphics
{
class VulkanDevice;
struct VulkanBuffer;
} // namespace graphics

namespace core
{
class ThreadsafeResourceManager
{
  public:
    explicit ThreadsafeResourceManager( uint32_t threads, graphics::VulkanDevice* vkDevice );
    ~ThreadsafeResourceManager();

    auto getCommandPool( uint32_t threadNum ) -> VkCommandPool;
    auto getCommandBuffer( uint32_t threadNum ) -> VkCommandBuffer;

  private:
    auto allocateCommandPools( uint32_t threadCount ) -> void;
    auto allocateCommandBuffers( uint32_t threadCount ) -> void;

  private:
    std::mutex m_mutex;
    std::vector<VkCommandPool> m_commandPools;
    std::vector<std::pair<bool, VkCommandBuffer>> m_commandBuffers;
    graphics::VulkanDevice* m_device;
};
} // namespace core