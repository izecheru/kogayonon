#pragma once
#include "precompiled/pch.hpp"
#include "vulkan_context.hpp"
#include <vulkan/vulkan.h>

namespace graphics
{

enum PipelineType
{
  geometry,
  picking,
  wireframe,
  text
};

enum VulkanAttachmentType
{
  depth,
  color
};

/**
 * @brief Options related to various pipeline options
 */
struct VulkanPipelineOptions
{
  VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
  VkPolygonMode polyMode{ VK_POLYGON_MODE_FILL };

  // Depth
  VkBool32 depthTestEnable{ VK_TRUE };
  VkBool32 depthWriteEnable{ VK_TRUE };
  VkCompareOp depthCompareOp{ VK_COMPARE_OP_LESS_OR_EQUAL };

  // For wireframe
  float lineWidth{ 1.0f };
};

struct VulkanPipelineSpec
{
  PipelineType type;
  VulkanPipelineOptions options;
  std::vector<VkDescriptorSetLayout> descriptorLayout;

  std::vector<VkFormat> colorAttachmentFormat;

  VkShaderModule vertexModule;
  VkShaderModule fragmentModule;

  /**
   * @brief sizeof(PushConstant)
   */
  uint32_t pushConstantSize{ 0u };
  VkShaderStageFlags pushConstantVisibility;

  uint32_t colorAttachmentCount{ 0u };

#ifdef VERTEX_PROVIDED
  VkVertexInputBindingDescription vertexBindingDescription;
  std::vector<VkVertexInputAttributeDescription> vertexAttributesDescription;
#endif
};

class VulkanPipeline
{
public:
  explicit VulkanPipeline( const VulkanPipelineSpec& spec, VulkanContext* pContext );
  VulkanPipeline() = default;
  ~VulkanPipeline() = default;

  /**
   * @brief Bind the pipeline
   * @param cmd Current VkCommandBuffer that we register commands on
   */
  auto bind( VkCommandBuffer& cmd, VkPipelineBindPoint bindPoint ) const -> void;
  auto create( const VulkanPipelineSpec& spec, VulkanContext* pContext ) -> void;
  auto getLayout() const -> VkPipelineLayout;
  auto getPipeline() const -> VkPipeline;

private:
  VulkanPipelineSpec m_spec;
  VkPipeline m_pipeline;
  VkPipelineLayout m_layout;
  VulkanContext* m_vkCtx;
};
} // namespace graphics
