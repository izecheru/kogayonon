#include "graphics/vulkan_tracy_context.hpp"
#include "graphics/vulkan_buffer.hpp"
#include "utilities/utils/utils.hpp"

#include "utilities/tracy_utils/tracy_vulkan_utils.hpp"

graphics::VulkanTracyContext::~VulkanTracyContext()
{
  if ( m_tracyContext )
  {
    TracyVkDestroy( m_tracyContext );
  }
}

graphics::VulkanTracyContext::VulkanTracyContext( VkDevice device,
                                                  VkPhysicalDevice physicalDevice,
                                                  VkQueue graphicsQueue,
                                                  VkCommandPool cmdPool )
{
  VkCommandBufferAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                         .commandPool = cmdPool,
                                         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                         .commandBufferCount = 1 };

  vkAllocateCommandBuffers( device, &allocInfo, &m_buffer );

  m_tracyContext = TracyVkContext( physicalDevice, device, graphicsQueue, m_buffer );
}

auto graphics::VulkanTracyContext::getCtx() -> tracy::VkCtx*
{
  return m_tracyContext;
}

auto graphics::VulkanTracyContext::getBuffer() -> VkCommandBuffer
{
  return m_buffer;
}

auto graphics::VulkanTracyContext::collect( VkCommandBuffer buffer ) -> void
{
  TracyVkCollect( m_tracyContext, buffer );
}
