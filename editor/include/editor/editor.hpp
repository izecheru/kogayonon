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
  bool initMainRegistry();
  bool initSDL();

  /**
   * @brief This just sends the device and swapchain to the asset manager
   * @return
   */
  bool initVulkan();

  bool initImgui();
  bool initMainWindow();

private:
  std::shared_ptr<window::Window> m_pWindow;
  std::shared_ptr<gui::VulkanImguiRenderer> m_pImguiRenderer;

  bool m_running{ false };
};
} // namespace editor