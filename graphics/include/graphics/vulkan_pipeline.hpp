#pragma once
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"
#include "vulkan_context.hpp"

namespace graphics
{

/**
 * @brief Options related to various pipeline options
 */
struct VulkanPipelineOptions
{
    VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
    VkPolygonMode polyMode{ VK_POLYGON_MODE_FILL };

    VkBool32 depthTestEnable{ VK_TRUE };
    VkBool32 depthWriteEnable{ VK_TRUE };
    VkCompareOp depthCompareOp{ VK_COMPARE_OP_LESS_OR_EQUAL };

    float lineWidth{ 1.0f };
};

struct VulkanPipelineSpec
{
    VulkanPipelineOptions options;
    std::vector<VkDescriptorSetLayout> descriptorLayout;

    uint32_t colorAttachmentCount{ 0u };
    std::vector<VkFormat> colorAttachmentFormat;
    VkFormat depthAttachmentFormat{ VK_FORMAT_UNDEFINED };
    VkFormat stencilAttachmentFormat{ VK_FORMAT_UNDEFINED };

    VkBool32 blendEnable{ VK_FALSE };

    VkShaderModule vertexModule;
    VkShaderModule fragmentModule;

    uint32_t pushConstantSize{ 0u };
    VkShaderStageFlags pushConstantVisibility;

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
