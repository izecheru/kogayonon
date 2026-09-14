#pragma once
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_rendering_info.hpp"

namespace graphics
{
struct VulkanContext;
}

namespace passId
{
inline constexpr const char* DepthPrepass = "depthPrepass";
}

namespace rendering
{

struct PrepassModuleData
{
  FGResource* depth{ VK_NULL_HANDLE };
  graphics::VulkanPipeline depthPrepassPipeline{};
  ModuleRenderingInfo renderingInfo;
};

class PrepassModule : public BaseModule
{
public:
  explicit PrepassModule( FrameGraph* graph,
                          graphics::VulkanContext* vkCtx,
                          VkExtent2D extent,
                          graphics::FrameInFlightVulkanDescriptor* cameraDescriptor );
  ~PrepassModule();

  auto registerPasses() -> void override;
  auto recreate( VkExtent2D extent ) -> void override;
  auto setExtent( VkExtent2D extent ) -> void override;

protected:
  auto registerDepthPrepass() -> void;

  auto createModuleResources( VkExtent2D extent ) -> void override;
  auto destroyModuleResources() -> void override;

private:
  FrameGraph* m_graph;
  graphics::VulkanContext* m_vkCtx;
  VkExtent2D m_extent;
  graphics::FrameInFlightVulkanDescriptor* m_cameraDescriptor;
};
} // namespace rendering