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

  auto getImGuiWindows() -> std::unordered_map<ImGuiWindowName, std::unique_ptr<ImGuiWindow>>&;

  // Used to pass the rendered output to a texture and display it in the viewport window
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
  std::unordered_map<ImGuiWindowName, std::unique_ptr<ImGuiWindow>> m_windows;
  VkSampler m_iconSampler;
  Popups m_popups;

  SDL_Window* m_wnd;

  std::unordered_map<std::string, ImFont*> m_fonts;
};
} // namespace gui