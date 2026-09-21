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
class ProjectLoadEvent;
class ProjectCreateEvent;
} // namespace core

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
    auto cleanup() const -> void;
    auto pollEvents() -> void;
    auto run() -> void;

    auto onUpdate() -> void;

    auto initMainRegistry() -> bool;
    auto initMainWindow() -> bool;
    auto initRenderer() -> bool;

    auto initSDL() -> bool;
    auto init() -> bool;

  private:
    auto createDescriptorPool() -> void;

  private:
    auto onWindowClose( const core::WindowCloseEvent& e ) -> void;

  private:
    VkDescriptorPool m_globalDescriptorPool;
    std::unique_ptr<window::Window> m_window;
    std::unique_ptr<rendering::VulkanRenderer> m_vulkanRenderer;

    bool m_running{ false };
};
} // namespace editor