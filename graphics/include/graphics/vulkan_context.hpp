#pragma once
#include <vulkan/vulkan.h>
#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "precompiled/pch.hpp"

namespace graphics
{
struct CreateImageInfo
{
  uint32_t width;
  uint32_t height;
  VkFormat format;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags properties;
  VkImage* image;
  VkDeviceMemory* imageMemory;
};

struct VulkanContext
{
  std::shared_ptr<VulkanDevice> device;
  std::shared_ptr<VulkanSwapchain> swapchain;
  std::shared_ptr<VulkanMemoryAllocator> memoryAllocator;
};

inline auto createImageView( VulkanContext* ctx, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags )
  -> VkImageView
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

  VkImageView imageView;
  VK_CALL( vkCreateImageView( ctx->device->getLogicalDevice(), &viewInfo, nullptr, &imageView ) );
  return imageView;
}

inline void transitionImageLayout(
  VulkanContext* ctx, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout )
{
  VkCommandBuffer commandBuffer =
    beginSingleTimeCommands( ctx->swapchain->getCommandPool(), ctx->device->getLogicalDevice() );

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else
  {
    throw std::invalid_argument( "unsupported layout transition!" );
  }

  vkCmdPipelineBarrier( commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

  endSingleTimeCommands( commandBuffer,
                         ctx->device->getLogicalDevice(),
                         ctx->device->getGraphicsQueue().handle,
                         ctx->swapchain->getCommandPool() );
}

inline void copyBufferToImage( VulkanContext* ctx, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height )
{
  VkCommandBuffer commandBuffer =
    beginSingleTimeCommands( ctx->swapchain->getCommandPool(), ctx->device->getLogicalDevice() );

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { width, height, 1 };

  vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

  endSingleTimeCommands( commandBuffer,
                         ctx->device->getLogicalDevice(),
                         ctx->device->getGraphicsQueue().handle,
                         ctx->swapchain->getCommandPool() );
}

inline void createTextureSampler( VulkanContext* ctx, VkSampler& textureSampler )
{
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties( ctx->device->getPhysicalDevice(), &properties );

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  VK_CALL( vkCreateSampler( ctx->device->getLogicalDevice(), &samplerInfo, nullptr, &textureSampler ) );
}
} // namespace graphics
