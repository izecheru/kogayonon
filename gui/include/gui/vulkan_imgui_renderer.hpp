#pragma once
#include <vulkan/vulkan.h>
#include "precompiled/pch.hpp"

struct SDL_Window;
struct ImGuiViewport;
struct ImFont;

namespace graphics
{
class VulkanDevice;
class VulkanSwapchain;
} // namespace graphics

namespace utilities
{
struct ColorConfig;
}

namespace gui
{
class ImGuiWindow;

struct Popups
{
  bool colorChangerPopup{ false };
  bool imguiVariablesPopup{ false };
  bool configPopup{ false };
};
} // namespace gui

namespace gui
{
class VulkanImguiRenderer
{
public:
  explicit VulkanImguiRenderer( SDL_Window* wnd, graphics::VulkanDevice* device, graphics::VulkanSwapchain* swapchain );
  ~VulkanImguiRenderer();

  void render();
  void present( VkCommandBuffer& buffer );

  void setViewport( VkImageView& viewportView );

private:
  void initImgui( SDL_Window* wnd, graphics::VulkanDevice* device, graphics::VulkanSwapchain* swapchain );
  void createIconSampler( graphics::VulkanDevice* device );
  void initWindows();
  void mainMenu();

  // MODALS
  void configChanger();
  void configModal();

  void colorChanger();
  void changeColorConfig();
  void colorModal();

  void imguiChanger();
  void imguiModal();
  // ------------

  void setColorPallete( const utilities::ColorConfig& cfg );

  void setupDockspace( ImGuiViewport* viewport );

  void begin();
  void end();

private:
  // this gets passed to the viewport
  VkImageView m_viewportView;

  VkDescriptorPool m_descriptorPool;
  graphics::VulkanDevice* m_device;
  std::unordered_map<std::string, std::unique_ptr<ImGuiWindow>> m_windows;
  VkSampler m_iconSampler;
  Popups m_popups;

  SDL_Window* m_wnd;

  std::unordered_map<std::string, ImFont*> m_fonts;
};
} // namespace gui