#pragma once
#include <vulkan/vulkan.h>

#include "graphics/utils.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "utilities/utils/utils.hpp"

#ifdef TRACY_ENABLE
#include "graphics/vulkan_tracy_context.hpp"
#endif

namespace graphics
{

struct VulkanContext
{
  std::unique_ptr<VulkanDevice> device;
  std::unique_ptr<VulkanSwapchain> swapchain;

#ifdef TRACY_ENABLE
  std::unique_ptr<VulkanTracyContext> tracyContext;
#endif

  VkDescriptorPool globalDescriptorPool;
};


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
