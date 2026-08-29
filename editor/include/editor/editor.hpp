#pragma once
#include "precompiled/pch.hpp"
#include <vulkan/vulkan.h>

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

namespace graphics
{
struct VulkanContext;
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

  auto onUpdate() -> void;

  bool initMainRegistry();
  bool initMainWindow();
  bool initRenderer();
  bool initVulkan();

  bool initSDL();
  bool init();

private:
  void createDescriptorPool();

private:
  void onWindowClose( const core::WindowCloseEvent& e );

private:
  VkDescriptorPool m_globalDescriptorPool;
  std::shared_ptr<window::Window> m_pWindow;
  std::shared_ptr<rendering::VulkanRenderer> m_pRenderer;

  bool m_running{ false };
};
} // namespace editor