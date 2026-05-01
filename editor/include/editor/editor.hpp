#pragma once
#include "precompiled/pch.hpp"

namespace window
{
class Window;
} // namespace window

namespace gui
{
class VulkanImguiRenderer;
}

namespace core
{
class WindowCloseEvent;
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

  bool initMainRegistry();
  bool initMainWindow();
  bool initVulkan();
  bool initImgui();
  bool initSDL();
  bool init();

private:
  // EVENTS
  void onWindowClose( const core::WindowCloseEvent& e );

private:
  std::shared_ptr<window::Window> m_pWindow;
  std::shared_ptr<gui::VulkanImguiRenderer> m_pImguiRenderer;

  bool m_running{ false };
};
} // namespace editor