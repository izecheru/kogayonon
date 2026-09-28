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
class VulkanImGuiRenderer;
}

namespace rendering
{
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
    explicit ImGuiModule( FrameGraph* graph, graphics::VulkanContext* vkCtx, gui::VulkanImGuiRenderer* imguiRenderer );
    ~ImGuiModule();

    auto registerPasses() -> void;
    auto setViewport() -> void;

  private:
    auto registerImGuiPass() -> void;

  private:
    graphics::VulkanContext* m_vkCtx;
    gui::VulkanImGuiRenderer* m_imguiRenderer;
    FrameGraph* m_graph;
};
} // namespace rendering
