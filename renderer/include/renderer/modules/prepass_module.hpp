#pragma once
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_descriptor.hpp"
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
                          graphics::VulkanContext* ctx,
                          VkExtent2D extent,
                          ModuleDescriptorData descriptorData );

  ~PrepassModule();

  auto registerPasses() -> void override;

protected:
  auto createModuleResources( VkExtent2D extent ) -> void override;
  auto destroyModuleResources() -> void override;
  auto registerDepthPrepass() -> void;

private:
  FrameGraph* m_graph;
  graphics::VulkanContext* m_vkCtx;
  VkExtent2D m_extent;
  ModuleDescriptorData m_moduleDescriptorData;
};
} // namespace rendering