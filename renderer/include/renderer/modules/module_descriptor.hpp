#pragma once
#include "vulkan/vulkan.h"

namespace rendering
{

struct ModuleDescriptorData
{
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
  std::vector<VkDescriptorSet> descriptorSets{};
};
} // namespace rendering