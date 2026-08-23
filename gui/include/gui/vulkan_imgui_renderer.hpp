#pragma once
#include "precompiled/pch.hpp"
#include <vulkan/vulkan.h>

struct SDL_Window;
struct ImGuiViewport;
struct ImFont;

#define IMGUI_VULKAN_MAX_DESCRIPTORS 200

namespace graphics
{
class VulkanDevice;
class VulkanSwapchain;
} // namespace graphics

namespace core
{
class KeyPressedEvent;
}

namespace utilities
{
struct ColorConfig;
}

namespace gui
{
enum class ImGuiWindowName
{
  File_Explorer,
  // If we have multiple viewports we change from map<enum,pWindow> to map<enum,vector<pWindow>> or smth
  Viewport,
  Scene_Hierarchy,
  Entity_Properties
};
} // namespace gui

namespace gui
{
class ImGuiWindow;
class IWidget;

struct Popups
{
  bool colorChangerPopup{ false };
  bool imguiVariablesPopup{ false };
  bool configPopup{ false };
  bool deviceDetailsPopup{ false };
};
} // namespace gui

namespace gui
{

class VulkanImguiRenderer
{
public:
  explicit VulkanImguiRenderer( SDL_Window* wnd, graphics::VulkanDevice* device, graphics::VulkanSwapchain* swapchain );
  ~VulkanImguiRenderer();

  auto initImgui( SDL_Window* wnd, graphics::VulkanDevice* device, graphics::VulkanSwapchain* swapchain ) -> void;
  auto render() -> void;
  auto present( VkCommandBuffer& buffer ) -> void;

  auto getImGuiWindows() -> std::unordered_map<ImGuiWindowName, std::unique_ptr<ImGuiWindow>>&;

  // Used to pass the rendered output to a texture and display it in the viewport window
  auto setViewport( VkImageView viewportView ) -> void;
  auto getViewportSize() -> VkExtent2D;

private:
  auto createIconSampler( graphics::VulkanDevice* device ) -> void;
  auto initWindows() -> void;
  auto mainMenu() -> void;

  // MODALS
  auto configChanger() -> void;
  auto configModal() -> void;

  auto colorChanger() -> void;
  auto changeColorConfig() -> void;
  auto colorModal() -> void;

  auto imguiChanger() -> void;
  auto imguiModal() -> void;

  auto showDeviceProperties() -> void;
  auto deviceModal() -> void;

  // ------------

  auto setColorPallete( const utilities::ColorConfig& cfg ) -> void;

  auto setupDockspace( ImGuiViewport* viewport ) -> void;

  auto begin() -> void;
  auto end() -> void;

private:
  // this gets passed to the viewport
  VkImageView m_viewportView;
  VkDescriptorPool m_descriptorPool;
  graphics::VulkanDevice* m_device;
  std::unordered_map<ImGuiWindowName, std::unique_ptr<ImGuiWindow>> m_windows;
  VkSampler m_iconSampler;
  Popups m_popups;

  SDL_Window* m_wnd;

  std::unordered_map<std::string, ImFont*> m_fonts;
};
} // namespace gui