#pragma once
#include <vulkan/vulkan.h>
#include "graphics/vulkan_defines.hpp"

namespace graphics
{
struct BufferedVulkanDescriptor
{
  VkDescriptorSetLayout layout{};
  std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> set{};
};

struct VulkanDescriptor
{
  VkDescriptorSetLayout layout{};
  VkDescriptorSet set{};
};

// class VulkanDescriptor
//{
// public:
//   VulkanDescriptor() = default;
//   explicit VulkanDescriptor( uint32_t descriptorCount );
//
//   ~VulkanDescriptor() = default;
//
//   auto getLayout() -> VkDescriptorSetLayout&;
//   auto getSet() -> VkDescriptorSet&;
//   auto getBufferedSet() -> std::vector<VkDescriptorSet>&;
//
// private:
//   bool m_buffered{ false };
//   // if we have more than one this is buffered
//   std::vector<VkDescriptorSet> m_descriptorSets;
//   VkDescriptorSetLayout m_descriptorLayout;
// };

} // namespace graphics