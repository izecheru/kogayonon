#pragma once
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"

namespace graphics
{
struct VulkanPipelineSpec
{
  // shaders
  std::string vertexShaderPath;
  std::string fragmentShaderPath;

  VkVertexInputBindingDescription vertexBindingDescription;
  std::vector<VkVertexInputAttributeDescription> vertexAttributesDescription;
};

class VulkanPipeline
{
public:
  explicit VulkanPipeline( const VulkanPipelineSpec& spec );
  ~VulkanPipeline() = default;

  void bindPipeline();

private:
  VulkanPipelineSpec m_spec;
};
} // namespace graphics
