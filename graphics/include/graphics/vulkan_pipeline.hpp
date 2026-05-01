#pragma once
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"
#include "vulkan_context.hpp"

namespace graphics
{

// with this we change how we provide the vertex input in the spec struct
#define VERTEX_PROVIDED

struct VulkanPipelineSpec
{
  /**
   * @brief Layout of the descriptor sets, if we have a descriptor at index 0, in shader we access that set with layout
   * ( set = 0, binding = bindingNum )
   */
  std::vector<VkDescriptorSetLayout> descriptorsLayout;

  // shader paths
  std::string vertexShaderPath{ "" };
  std::string fragmentShaderPath{ "" };

  uint32_t pushSize{ 0u };

#ifdef VERTEX_PROVIDED
  VkVertexInputBindingDescription vertexBindingDescription;
  std::vector<VkVertexInputAttributeDescription> vertexAttributesDescription;
#endif
};

class VulkanPipeline
{
public:
  explicit VulkanPipeline( const VulkanPipelineSpec& spec, VulkanContext* pContext );
  ~VulkanPipeline() = default;

  /**
   * @brief Bind the pipeline
   * @param cmd Current VkCommandBuffer that we register commands on
   */
  void bind( VkCommandBuffer& cmd ) const;

private:
  VulkanPipelineSpec m_spec;
  VkPipeline m_pipeline;
  VkPipelineLayout m_layout;

  // vulkan context for device and swapchain access
  VulkanContext* m_pVkContext{ nullptr };
};
} // namespace graphics
