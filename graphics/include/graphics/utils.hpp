#pragma once
#include <stdexcept>
#include <vulkan/vulkan.h>

static auto beginSingleTimeCommands( VkCommandPool& pool, VkDevice device ) -> VkCommandBuffer
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers( device, &allocInfo, &commandBuffer );

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer( commandBuffer, &beginInfo );

  return commandBuffer;
}

static void endSingleTimeCommands( VkCommandBuffer& commandBuffer,
                                   VkDevice device,
                                   VkQueue& graphicsQueue,
                                   VkCommandPool& pool )
{
  vkEndCommandBuffer( commandBuffer );

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
  vkQueueWaitIdle( graphicsQueue );

  vkFreeCommandBuffers( device, pool, 1, &commandBuffer );
}

static auto findMemoryType( VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties )
  -> uint32_t
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

  for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
  {
    if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
    {
      return i;
    }
  }

  throw std::runtime_error( "failed to find suitable memory type!" );
}
