#pragma once

#include <vulkan/vulkan.h>

namespace graphics
{
struct FrameInFlightVulkanDescriptor
{
  VkDescriptorSetLayout layout{};
  std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> set{};
};

struct VulkanDescriptorInfo
{
  VkDescriptorSetLayoutBinding descriptorBinding{};
  VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
};

struct VulkanDescriptor
{
  VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
  VkDescriptorSet set{ VK_NULL_HANDLE };
};

} // namespace graphics