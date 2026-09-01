#pragma once
#include "graphics/vulkan_descriptor.hpp"
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_descriptor.hpp"
#include "renderer/modules/module_rendering_info.hpp"

namespace rendering
{
namespace passId
{
constexpr const char* ShadowmapPass = "shadowmapPass";
}

struct ShadowmapModuleData
{
  FGResource* depth{ VK_NULL_HANDLE };
  graphics::VulkanPipeline shadowmapPipeline;
  ModuleRenderingInfo renderingInfo;

  /**
   * @brief used to pass the depth texture from the light's point of view to other modules that might need to sample
   * it and render the shadow
   */
  graphics::VulkanDescriptor directionalLightDescriptor;
};

class ShadowmapModule : public BaseModule
{
public:
  explicit ShadowmapModule( FrameGraph* graph,
                            graphics::VulkanContext* vkCtx,
                            ModuleDescriptorData descriptorData,
                            VkExtent2D extent );
  ~ShadowmapModule();

  auto registerPasses() -> void override;

protected:
  auto registerShadowmapPass() -> void;

  auto setExtent( VkExtent2D extent ) -> void override;
  auto recreate( VkExtent2D extent ) -> void override;
  auto createModuleResources( VkExtent2D extent ) -> void override;
  auto destroyModuleResources() -> void override;

private:
  FrameGraph* m_graph;
  graphics::VulkanContext* m_vkCtx;
  ModuleDescriptorData m_moduleDescriptorData;
  VkExtent2D m_extent;
};

} // namespace rendering