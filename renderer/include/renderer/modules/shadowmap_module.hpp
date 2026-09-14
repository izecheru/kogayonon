#pragma once
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_rendering_info.hpp"

namespace rendering
{
namespace passId
{
constexpr const char* ShadowmapPass = "shadowmapPass";
}

struct DirectionalLightUBO
{
  glm::mat4 projection;
  glm::mat4 view;
};

struct ShadowmapModuleData
{
  FGResource* depth{ VK_NULL_HANDLE };
  graphics::VulkanPipeline shadowmapPipeline;
  ModuleRenderingInfo renderingInfo;

  graphics::VulkanDescriptor directionalLightDescriptor;      // texture descriptor
  graphics::VulkanDescriptor directionalLightPovDescriptor;   // light mvp matrix descriptor
  graphics::FrameInFlightVulkanBuffer directionalLightBuffer; // ubo
};

class ShadowmapModule : public BaseModule
{
public:
  explicit ShadowmapModule( FrameGraph* graph, graphics::VulkanContext* vkCtx, VkExtent2D extent );
  ~ShadowmapModule();

  auto registerPasses() -> void override;
  auto getShadowmapDescriptor() -> graphics::VulkanDescriptor&;

protected:
  auto registerShadowmapPass() -> void;

  auto setExtent( VkExtent2D extent ) -> void override;
  auto recreate( VkExtent2D extent ) -> void override;
  auto createModuleResources( VkExtent2D extent ) -> void override;
  auto destroyModuleResources() -> void override;

private:
  FrameGraph* m_graph;
  graphics::VulkanContext* m_vkCtx;
  VkExtent2D m_extent;
};

} // namespace rendering