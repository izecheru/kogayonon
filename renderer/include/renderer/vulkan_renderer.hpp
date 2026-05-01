#pragma once
#include <vma/vk_mem_alloc.h>
#include "graphics/vulkan_pipeline.hpp"
#include "precompiled/pch.hpp"

namespace graphics
{
struct VulkanContext;
}

namespace rendering
{
enum PipelineType
{
  GEOMETRY_BASIC
};

struct VulkanViewport
{
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;

  VkImage depthImage;
  VkImageView depthView;
  VmaAllocation depthAllocation;
};

class VulkanRenderer
{
public:
  explicit VulkanRenderer(
    const std::initializer_list<std::pair<PipelineType, graphics::VulkanPipelineSpec>>& pipelineInitializer,
    graphics::VulkanContext* pCtx );

  ~VulkanRenderer();

  void render();

  auto getViewport() -> VulkanViewport&;

private: // funcs
  void createViewport();
  void createPipeline( const graphics::VulkanPipelineSpec& spec, const PipelineType& pipelineType );

private:
  std::map<PipelineType, std::unique_ptr<graphics::VulkanPipeline>> m_pipelines;
  graphics::VulkanContext* m_pVkContext{ nullptr };

  VulkanViewport m_viewport;
};
} // namespace rendering