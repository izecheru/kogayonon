#pragma once
#include "glm/glm.hpp"
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_image.hpp"
#include "graphics/vulkan_pipeline.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/module_base.hpp"
#include "renderer/modules/module_descriptor.hpp"
#include "renderer/modules/module_rendering_info.hpp"

namespace gui
{
class VulkanImguiRenderer;
}

namespace core
{
class MouseClickedEvent;
}

namespace rendering
{
class FrameGraph;

namespace passId
{
inline constexpr const char* Picking = "pickingPass";
inline constexpr const char* PickingReadback = "pickingReadbackPass";
inline constexpr const char* PickingEntityRead = "pickingEntityReadPass";
} // namespace passId

struct PickingModuleData
{
  FGResource* color{ VK_NULL_HANDLE };
  graphics::VulkanPipeline pickingPipeline{};
  graphics::FrameInFlightVulkanBuffer pickingBuffer;
  ModuleRenderingInfo renderingInfo;
};

class PickingModule : public BaseModule
{
public:
  explicit PickingModule( graphics::VulkanContext* vkCtx,
                          FrameGraph* graph,
                          gui::VulkanImguiRenderer* imguiRenderer,
                          VkExtent2D extent,
                          ModuleDescriptorData descriptorData );
  ~PickingModule();

  auto registerPasses() -> void override;
  auto setCoords( glm::ivec2 coords ) -> void;

private:
  auto createModuleResources( VkExtent2D extent ) -> void override;
  auto destroyModuleResources() -> void override;

  auto registerPickingPass() -> void;
  auto registerPickingReadbackPass() -> void;
  auto registerPickingEntityReadPass() -> void;

private:
  glm::ivec2 m_mouseCoords;
  bool m_pickRequested;
  bool m_readyToCopy;
  FrameGraph* m_graph;
  graphics::VulkanContext* m_vkCtx;
  ModuleDescriptorData m_moduleDescriptorData;
  VkExtent2D m_extent;
  gui::VulkanImguiRenderer* m_imguiRenderer;
  int32_t m_lastFrameIndex;
};
} // namespace rendering