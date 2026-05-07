#pragma once
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"
#include "vulkan_context.hpp"

namespace graphics
{

// with this we change how we provide the vertex input in the spec struct
#define VERTEX_PROVIDED

enum PipelineType
{
  GEOMETRY_BASIC
};

struct VulkanPipelineSpec
{
  PipelineType type;
  std::vector<VkDescriptorSetLayout> descriptorLayout;

  /**
   * @brief Shader paths
   */
  std::filesystem::path vertexShaderPath{ "" };
  std::filesystem::path fragmentShaderPath{ "" };

  /**
   * @brief sizeof(PushConstant)
   */
  uint32_t pushConstantSize{ 0u };

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
  void bind( VkCommandBuffer& cmd, VkPipelineBindPoint bindPoint ) const;

  auto getLayout() -> VkPipelineLayout&;

private:
  VulkanPipelineSpec m_spec;
  VkPipeline m_pipeline;
  VkPipelineLayout m_layout;

  VulkanContext* m_pVkContext{ nullptr };
};
} // namespace graphics
