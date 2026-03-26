#pragma once
#include <memory>

namespace window
{
class Window;
} // namespace window

namespace gui
{
class VulkanImguiRenderer;
}

namespace graphics
{
class VulkanDevice;
class VulkanSwapchain;
} // namespace graphics

namespace editor
{
class Editor
{
public:
  Editor();
  ~Editor();
  void cleanup() const;
  void pollEvents();
  void run();

  bool init();
  bool initSDL();
  bool initVulkan();
  bool initImgui();
  bool initMainWindow();

private:
  std::shared_ptr<window::Window> m_pWindow;
  std::shared_ptr<graphics::VulkanDevice> m_pDevice;
  std::shared_ptr<graphics::VulkanSwapchain> m_pSwapchain;
  std::shared_ptr<gui::VulkanImguiRenderer> m_pImguiRenderer;

  bool m_running{ false };
};
} // namespace editor