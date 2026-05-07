#pragma once
#include <vulkan/vulkan.h>
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

namespace rendering
{
class VulkanRenderer;
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
  bool initRenderer();
  bool initVulkan();

  bool initSDL();
  bool init();

private: // funcs
  void createDescriptorPool();

private:
  // EVENTS
  void onWindowClose( const core::WindowCloseEvent& e );

private:
  VkDescriptorPool m_globalDescriptorPool;
  std::shared_ptr<window::Window> m_pWindow;
  std::shared_ptr<rendering::VulkanRenderer> m_pRenderer;

  bool m_running{ false };
};
} // namespace editor