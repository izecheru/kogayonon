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

enum VulkanAttachmentType
{
  DEPTH,
  COLOR
};

struct VulkanPipelineAttachmentData
{
  VulkanAttachmentType type;
  uint32_t count{ 0u };
  std::vector<VkRenderingAttachmentInfo> attachment;
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
  VkCompareOp depthCompareOp{ VK_COMPARE_OP_LESS };

  // For wireframe
  float lineWidth{ 1.0f };

  // Color and depth attachment data
  VulkanPipelineAttachmentData colorAttachmentData;
  VulkanPipelineAttachmentData depthAttachmentData;
};

struct VulkanPipelineSpec
{
  PipelineType type;
  VulkanPipelineOptions options;
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

  // Color and depth attachments
  std::vector<VkRenderingAttachmentInfo> colorAttachment;
  std::vector<VkRenderingAttachmentInfo> depthAttachment;
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
  VulkanContext* m_pVkContext;
};
} // namespace graphics
