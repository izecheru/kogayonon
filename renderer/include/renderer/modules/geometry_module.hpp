#pragma once
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_descriptor.hpp"
#include "renderer/modules/module_rendering_info.hpp"

namespace graphics
{
struct VulkanContext;
}

namespace rendering
{

namespace passId
{
inline constexpr const char* Wireframe = "wireframePass";
inline constexpr const char* Geometry = "geometryPass";
inline constexpr const char* DepthPrepass = "depthPrepass";
} // namespace passId

struct GeometryModuleData
{
  FGResource* color{ VK_NULL_HANDLE };

  graphics::VulkanPipeline basePipeline{};
  graphics::VulkanPipeline wireframePipeline{};

  ModuleRenderingInfo renderingInfo;
};

class GeometryModule : public BaseModule
{
public:
  explicit GeometryModule( FrameGraph* graph,
                           graphics::VulkanContext* ctx,
                           VkExtent2D extent,
                           ModuleDescriptorData descriptorData,
                           glm::vec4 clearColor );

  explicit GeometryModule( FrameGraph* graph,
                           graphics::VulkanContext* ctx,
                           VkExtent2D extent,
                           ModuleDescriptorData descriptorData );

  ~GeometryModule();

  auto setClearColor( glm::vec4 clearColor ) -> void;
  auto setExtent( VkExtent2D extent ) -> void override;

  /**
   * @brief Create the pipelines from the module data and register
   * render passes
   * @return
   */
  auto registerPasses() -> void override;

  /**
   * @brief Initialize and allocate all the necessary resources for using this module to render
   * @param extent Extent of the images that this pass is going to render to
   * @return
   */
  auto createModuleResources( VkExtent2D extent ) -> void override;

  auto enableWireframe() -> void;
  auto disableWireframe() -> void;

  auto recreate( VkExtent2D extent ) -> void override;

protected:
  auto destroyModuleResources() -> void override;
  auto registerWireframePass() -> void;
  auto registerBaseGeometryPass() -> void;

private:
  graphics::VulkanContext* m_vkCtx;
  rendering::FrameGraph* m_graph;
  ModuleDescriptorData m_moduleDescriptorData;
  glm::vec4 m_clearColor;

  VkExtent2D m_extent{};

  bool m_wireframe;
  bool m_wireframeInit;
};
} // namespace rendering