#pragma once
#include "vulkan/vulkan_core.h"
#include "imgui.h"

#define IMGUI_VULKAN_MAX_DESCRIPTORS 200

struct SDL_Window;

namespace core
{
class SwitchSceneEvent;
class ConfigChangedEvent;
} // namespace core

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
    bool deviceDetailsPopup{ false };
};
} // namespace gui

namespace gui
{

class VulkanImGuiRenderer
{
  public:
    explicit VulkanImGuiRenderer( SDL_Window* wnd,
                                  graphics::VulkanDevice* device,
                                  graphics::VulkanSwapchain* swapchain );
    ~VulkanImGuiRenderer();

    auto initImgui( SDL_Window* wnd, graphics::VulkanDevice* device, graphics::VulkanSwapchain* swapchain ) -> void;
    auto render() -> void;
    auto renderDrawData( VkCommandBuffer& buffer ) -> void;

    auto getImGuiWindows() -> std::unordered_map<ImGuiWindowName, std::unique_ptr<ImGuiWindow>>&;

    // Used to pass the rendered output to a texture and display it in the viewport window
    auto setViewport( VkImageView viewportView ) -> void;
    auto getViewportExtent() -> VkExtent2D;

  private:
    auto onConfigChange( const core::ConfigChangedEvent& e ) -> void;
    auto onSwitchScene( const core::SwitchSceneEvent& e ) -> void;

    template <class T>
    auto getWindow( ImGuiWindowName name ) -> T*
    {
        return dynamic_cast<T*>( m_windows[name].get() );
    }

    auto createIconSampler( graphics::VulkanDevice* device ) -> void;
    auto initWindows() -> void;
    auto mainMenu() -> void;

    auto customTitleBar() -> void;

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