#pragma once
#include <vulkan/vulkan.h>

namespace graphics
{
struct VulkanContext;
}

namespace rendering
{
class FrameGraph;
}

namespace gui
{
class VulkanImguiRenderer;
}

namespace rendering
{
// This should draw directly to the swapchain image and retrieve the
// other drawn images like geometry and what not to set the viewport
struct ImGuiModuleData
{
  VkRenderingAttachmentInfo renderingAttachment{};
  VkRenderingInfo renderingInfo{};
};

namespace passId
{
inline constexpr const char* ImGui = "imguiPass";
} // namespace passId

class ImGuiModule
{
public:
  explicit ImGuiModule( graphics::VulkanContext* vkCtx, FrameGraph* graph, gui::VulkanImguiRenderer* imguiRenderer );
  ~ImGuiModule();

  void registerPasses();

private:
  void registerImGuiPass();

private:
  graphics::VulkanContext* m_vkCtx;
  gui::VulkanImguiRenderer* m_imguiRenderer;
  FrameGraph* m_graph;
};
} // namespace rendering
