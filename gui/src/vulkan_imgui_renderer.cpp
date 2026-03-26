#include "gui/vulkan_imgui_renderer.hpp"
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include "core/asset_manager/asset_manager.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "gui/imgui_windows/file_explorer.hpp"
#include "gui/imgui_windows/scene_hierarchy.hpp"
#include "gui/imgui_windows/viewport.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "resources/texture.hpp"
#include "utilities/configurator/configurator.hpp"
#include "utilities/fonts/fontawesome7.hpp"

gui::VulkanImguiRenderer::VulkanImguiRenderer( SDL_Window* wnd,
                                               graphics::VulkanDevice* device,
                                               graphics::VulkanSwapchain* swapchain )
    : m_device{ device }
    , m_wnd{ wnd }
{
  initImgui( wnd, device, swapchain );
  createIconSampler( device );
  initWindows();
}

void gui::VulkanImguiRenderer::createIconSampler( graphics::VulkanDevice* device )
{
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties( device->getPhysicalDevice(), &properties );

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_TRUE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if ( vkCreateSampler( device->getLogicalDevice(), &samplerInfo, nullptr, &m_iconSampler ) != VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create texture sampler!" );
  }
}

gui::VulkanImguiRenderer::~VulkanImguiRenderer()
{
  vkDeviceWaitIdle( m_device->getLogicalDevice() );
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool( m_device->getLogicalDevice(), m_descriptorPool, nullptr );
}

void gui::VulkanImguiRenderer::begin()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  setupDockspace( ImGui::GetMainViewport() );
}

void gui::VulkanImguiRenderer::end()
{
  ImGui::Render();
}

void gui::VulkanImguiRenderer::setupDockspace( ImGuiViewport* viewport )
{
  const auto dockSpaceId = ImGui::DockSpaceOverViewport( 0, ImGui::GetMainViewport() );

  if ( static auto firstTime = true; firstTime ) [[unlikely]]
  {
    firstTime = false;

    ImGui::DockBuilderRemoveNode( dockSpaceId );
    ImGui::DockBuilderAddNode( dockSpaceId );

    auto centerNodeId = dockSpaceId;
    const auto bottomNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Down, 0.35f, nullptr, &centerNodeId );
    const auto leftNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Left, 0.20f, nullptr, &centerNodeId );
    auto rightNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Right, 0.30f, nullptr, &centerNodeId );

    ImGui::DockBuilderDockWindow( ICON_FA_FILE " File explorer", bottomNodeId );
    ImGui::DockBuilderDockWindow( ICON_FA_IMAGE " Viewport", rightNodeId );
    ImGui::DockBuilderDockWindow( ICON_FA_LIST " Hierarchy", leftNodeId );

    ImGui::DockBuilderFinish( dockSpaceId );
  }
}

void gui::VulkanImguiRenderer::render()
{
  begin();

  mainMenu();

  colorModal();
  imguiModal();
  configModal();

  for ( auto& window : m_windows )
  {
    window.second->render();
  }

  end();
}

void gui::VulkanImguiRenderer::present( VkCommandBuffer& buffer )
{
  auto drawData = ImGui::GetDrawData();
  const bool isMinimized = ( drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f );

  if ( !isMinimized )
    ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), buffer, NULL );

  if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
  {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
}

void gui::VulkanImguiRenderer::initImgui( SDL_Window* wnd,
                                          graphics::VulkanDevice* device,
                                          graphics::VulkanSwapchain* swapchain )
{
  VkDescriptorPoolSize poolSizes[] = {
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
  };

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = 0;

  for ( VkDescriptorPoolSize& poolSize : poolSizes )
    poolInfo.maxSets += poolSize.descriptorCount;

  poolInfo.poolSizeCount = (uint32_t)IM_COUNTOF( poolSizes );
  poolInfo.pPoolSizes = poolSizes;

  if ( vkCreateDescriptorPool( device->getLogicalDevice(), &poolInfo, nullptr, &m_descriptorPool ) != VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create imgui descriptor pool!" );
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImFontConfig cfg;
  cfg.OversampleH = 3;
  cfg.OversampleV = 1;
  cfg.PixelSnapH = true;

  float baseFontSize = 18.0f;
  float iconFontSize =
    baseFontSize * 2.0f /
    3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

  // merge in icons from Font Awesome
  static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

  ImFontConfig iconsConfig;
  iconsConfig.PixelSnapH = true;
  iconsConfig.GlyphMinAdvanceX = iconFontSize;
  iconsConfig.MergeMode = true;

  m_fonts.emplace( "inter",
                   io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/Inter_18pt-Medium.ttf", 18.0f, &cfg ) );

  io.FontDefault = m_fonts.at( "inter" );

  m_fonts.emplace(
    "fa-solid-900",
    io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/fa-solid-900.ttf", iconFontSize, &iconsConfig, iconRanges ) );

  m_fonts.emplace( "inter-bold",
                   io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/Inter_18pt-Bold.ttf", 18.0f, &cfg ) );

  m_fonts.emplace( "inter-bold-italic",
                   io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/Inter_18pt-BoldItalic.ttf", 18.0f, &cfg ) );

  m_fonts.emplace( "inter-extrabold",
                   io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/Inter_18pt-ExtraBold.ttf", 18.0f, &cfg ) );

  m_fonts.emplace(
    "inter-extrabold-italic",
    io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/Inter_18pt-ExtraBoldItalic.ttf", 18.0f, &cfg ) );

  // change the style
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4* colors = style.Colors;
  style.WindowRounding = 5.3f;
  style.GrabRounding = style.FrameRounding = 2.3f;
  style.ScrollbarRounding = 5.0f;
  style.FrameBorderSize = 1.0f;
  style.ItemSpacing.y = 6.5f;

  style.Colors[ImGuiCol_Text] = { 0.73333335f, 0.73333335f, 0.73333335f, 1.00f };
  style.Colors[ImGuiCol_ModalWindowDimBg] = { 0.0f, 0.0f, 0.0f, 0.0f };
  style.Colors[ImGuiCol_TextDisabled] = { 0.34509805f, 0.34509805f, 0.34509805f, 1.00f };
  style.Colors[ImGuiCol_WindowBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
  style.Colors[ImGuiCol_ChildBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.00f };
  style.Colors[ImGuiCol_PopupBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
  style.Colors[ImGuiCol_Border] = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f };
  style.Colors[ImGuiCol_BorderShadow] = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f };
  style.Colors[ImGuiCol_FrameBg] = { 0.16862746f, 0.16862746f, 0.16862746f, 0.54f };
  style.Colors[ImGuiCol_FrameBgHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
  style.Colors[ImGuiCol_FrameBgActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
  style.Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
  style.Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
  style.Colors[ImGuiCol_TitleBgActive] = { 0.741f, 0.576f, 0.976f, 0.094f };
  style.Colors[ImGuiCol_MenuBarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.80f };
  style.Colors[ImGuiCol_ScrollbarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.60f };
  style.Colors[ImGuiCol_ScrollbarGrab] = { 0.21960786f, 0.30980393f, 0.41960788f, 0.51f };
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
  style.Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f };
  // style.Colors[ImGuiCol_ComboBg]               = {0.1f, 0.1f, 0.1f, 0.99f};
  style.Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
  style.Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
  style.Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
  style.Colors[ImGuiCol_Button] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f };
  style.Colors[ImGuiCol_ButtonHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
  style.Colors[ImGuiCol_ButtonActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f };
  style.Colors[ImGuiCol_Header] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f };
  style.Colors[ImGuiCol_HeaderHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
  style.Colors[ImGuiCol_HeaderActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
  style.Colors[ImGuiCol_Separator] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
  style.Colors[ImGuiCol_SeparatorHovered] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
  style.Colors[ImGuiCol_SeparatorActive] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
  style.Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
  style.Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
  style.Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
  style.Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
  style.Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
  style.Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f };
  style.Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
  style.Colors[ImGuiCol_TextSelectedBg] = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f };

  ImGui_ImplSDL2_InitForVulkan( wnd );

  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = device->getInstance();
  initInfo.PhysicalDevice = device->getPhysicalDevice();
  initInfo.Device = device->getLogicalDevice();
  initInfo.QueueFamily = device->getGraphicsQueue().familyIndex;
  initInfo.Queue = device->getGraphicsQueue().handle;
  initInfo.DescriptorPool = m_descriptorPool;

  // TODO(kogayonon) get this from swapchain
  initInfo.MinImageCount = 2;
  initInfo.ImageCount = 3;

  initInfo.UseDynamicRendering = true;
  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain->getSwapchainFormat();

  ImGui_ImplVulkan_Init( &initInfo );
}

void gui::VulkanImguiRenderer::initWindows()
{
  auto& assetManager = core::AssetManager::get();

  auto folderPath = std::filesystem::absolute( "." ) / "engine_resources\\textures\\folder.png";
  auto folderTexture = assetManager.addTexture( "logo.png", folderPath.string() );

  auto filePath = std::filesystem::absolute( "." ) / "engine_resources\\textures\\file.png";
  auto fileTexture = assetManager.addTexture( "logo.png", filePath.string() );

  auto folder =
    ImGui_ImplVulkan_AddTexture( m_iconSampler, folderTexture->getView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  auto file =
    ImGui_ImplVulkan_AddTexture( m_iconSampler, fileTexture->getView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  FileExplorerSpec fileExp{ .fonts = &m_fonts, .folderIcon = std::move( folder ), .fileIcon = std::move( file ) };

  m_windows.emplace( "File explorer", std::make_unique<FileExplorerWindow>( ICON_FA_FILE " File explorer", fileExp ) );

  auto playPath = std::filesystem::absolute( "." ) / "engine_resources\\textures\\play.png";
  auto playTexture = assetManager.addTexture( "play.png", playPath.string() );

  auto stopPath = std::filesystem::absolute( "." ) / "engine_resources\\textures\\stop.png";
  auto stopTexture = assetManager.addTexture( "stop.png", stopPath.string() );

  auto renderModePath = std::filesystem::absolute( "." ) / "engine_resources\\textures\\render_mode_icon.png";
  auto renderModeTexture = assetManager.addTexture( "render_mode_icon.png", renderModePath.string() );

  auto play =
    ImGui_ImplVulkan_AddTexture( m_iconSampler, playTexture->getView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  auto stop =
    ImGui_ImplVulkan_AddTexture( m_iconSampler, stopTexture->getView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  auto renderMode = ImGui_ImplVulkan_AddTexture(
    m_iconSampler, renderModeTexture->getView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  ViewportSpec viewportSpec{ .fonts = &m_fonts,
                             .renderModeIcon = std::move( renderMode ),
                             .playIcon = std::move( play ),
                             .stopIcon = std::move( stop ) };

  m_windows.emplace( "Viewport", std::make_unique<Viewport>( m_wnd, ICON_FA_IMAGE " Viewport", viewportSpec ) );

  m_windows.emplace( "Hierarchy", std::make_unique<SceneHierarchy>( ICON_FA_LIST " Hierarchy", SceneHierarchySpec{} ) );
}

void gui::VulkanImguiRenderer::mainMenu()
{
  if ( ImGui::BeginMainMenuBar() )
  {
    if ( ImGui::BeginMenu( "File" ) )
    {
      if ( ImGui::MenuItem( "Close" ) )
      {
      }
      ImGui::EndMenu();
    }
    if ( ImGui::BeginMenu( "Settings" ) )
    {
      if ( ImGui::MenuItem( "Edit config" ) )
      {
        m_popups.configPopup = true;
      }
      if ( ImGui::MenuItem( "Edit colors" ) )
      {
        ImGui::OpenPopup( "Color edit" );
        m_popups.colorChangerPopup = true;
      }
      if ( ImGui::MenuItem( "Edit imgui variables" ) )
      {
        ImGui::OpenPopup( "Edit imgui" );
        m_popups.imguiVariables = true;
      }

      ImGui::EndMenu();
    }
  }
  ImGui::EndMainMenuBar();

  if ( m_popups.colorChangerPopup )
    ImGui::OpenPopup( "Color settings" );

  if ( m_popups.imguiVariables )
    ImGui::OpenPopup( "Edit imgui" );

  if ( m_popups.configPopup )
    ImGui::OpenPopup( "Engine config" );
}

void gui::VulkanImguiRenderer::colorChanger()
{
  auto& style = ImGui::GetStyle();
  ImGui::ColorEdit4( "Window bg", (float*)&style.Colors[ImGuiCol_WindowBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Child bg", (float*)&style.Colors[ImGuiCol_WindowBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Popup bg", (float*)&style.Colors[ImGuiCol_PopupBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Menu bar bg", (float*)&style.Colors[ImGuiCol_MenuBarBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Check mark", (float*)&style.Colors[ImGuiCol_CheckMark], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Border" );
  ImGui::ColorEdit4( "Border", (float*)&style.Colors[ImGuiCol_Border], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Border shadow", (float*)&style.Colors[ImGuiCol_BorderShadow], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Frame" );
  ImGui::ColorEdit4( "Frame bg", (float*)&style.Colors[ImGuiCol_FrameBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Frame bg hovered", (float*)&style.Colors[ImGuiCol_FrameBgHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Frame bg active", (float*)&style.Colors[ImGuiCol_FrameBgActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Title" );
  ImGui::ColorEdit4( "Title bg", (float*)&style.Colors[ImGuiCol_TitleBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Title bg collapsed", (float*)&style.Colors[ImGuiCol_TitleBgCollapsed], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Title bg active", (float*)&style.Colors[ImGuiCol_TitleBgActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Scroll bar" );
  ImGui::ColorEdit4( "Scroll bar bg", (float*)&style.Colors[ImGuiCol_ScrollbarBg], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Scroll bar grab", (float*)&style.Colors[ImGuiCol_ScrollbarGrab], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Scroll bar grab hovered", (float*)&style.Colors[ImGuiCol_ScrollbarGrabHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Scroll bar grab active", (float*)&style.Colors[ImGuiCol_ScrollbarGrabActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Slider" );
  ImGui::ColorEdit4( "Slider grab", (float*)&style.Colors[ImGuiCol_SliderGrab], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Slider grab active", (float*)&style.Colors[ImGuiCol_SliderGrabActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Button" );
  ImGui::ColorEdit4( "Button", (float*)&style.Colors[ImGuiCol_Button], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Button hovered", (float*)&style.Colors[ImGuiCol_ButtonHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Button active", (float*)&style.Colors[ImGuiCol_ButtonActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Header" );
  ImGui::ColorEdit4( "Header", (float*)&style.Colors[ImGuiCol_Header], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Header hovered", (float*)&style.Colors[ImGuiCol_HeaderHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Header active", (float*)&style.Colors[ImGuiCol_HeaderActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Separator" );
  ImGui::ColorEdit4( "Separator", (float*)&style.Colors[ImGuiCol_Separator], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Separator hovered", (float*)&style.Colors[ImGuiCol_SeparatorHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Separator active", (float*)&style.Colors[ImGuiCol_SeparatorActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Resize" );
  ImGui::ColorEdit4( "Resize grip", (float*)&style.Colors[ImGuiCol_ResizeGrip], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Resize grip hovered", (float*)&style.Colors[ImGuiCol_ResizeGripHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Resize grip active", (float*)&style.Colors[ImGuiCol_ResizeGripActive], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Plots" );
  ImGui::ColorEdit4( "Plot lines", (float*)&style.Colors[ImGuiCol_PlotLines], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Plot lines hovered", (float*)&style.Colors[ImGuiCol_PlotLinesHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Plot histogram", (float*)&style.Colors[ImGuiCol_PlotHistogram], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Plot histogram hovered", (float*)&style.Colors[ImGuiCol_PlotHistogramHovered], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Text" );
  ImGui::ColorEdit4( "Text", (float*)&style.Colors[ImGuiCol_Text], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Text disabled", (float*)&style.Colors[ImGuiCol_TextDisabled], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Text selectable", (float*)&style.Colors[ImGuiCol_TextSelectedBg], ImGuiColorEditFlags_NoInputs );

  ImGui::SeparatorText( "Tabs" );
  ImGui::ColorEdit4( "Tab", (float*)&style.Colors[ImGuiCol_Tab], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Tab hovered", (float*)&style.Colors[ImGuiCol_TabHovered], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Tab active", (float*)&style.Colors[ImGuiCol_TabActive], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Tab dimmed", (float*)&style.Colors[ImGuiCol_TabDimmed], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4(
    "Tab dimmed selected", (float*)&style.Colors[ImGuiCol_TabDimmedSelected], ImGuiColorEditFlags_NoInputs );
  ImGui::ColorEdit4( "Tab dimmed selected overline",
                     (float*)&style.Colors[ImGuiCol_TabDimmedSelectedOverline],
                     ImGuiColorEditFlags_NoInputs );
}

void gui::VulkanImguiRenderer::colorModal()
{
  if ( ImGui::BeginPopupModal( "Color settings", &m_popups.colorChangerPopup, ImGuiWindowFlags_NoDocking ) )
  {
    colorChanger();
    if ( ImGui::Button( ICON_FA_SAVE " Close" ) )
    {
      m_popups.colorChangerPopup = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void gui::VulkanImguiRenderer::imguiChanger()
{
  auto& style = ImGui::GetStyle();

  ImGui::SeparatorText( "Window" );
  ImGui::Text( "x " );
  ImGui::SameLine();
  ImGui::DragFloat( "##widthWindow", &style.FramePadding.x );

  ImGui::Text( "y " );
  ImGui::SameLine();
  ImGui::DragFloat( "##heightWindow", &style.FramePadding.y );

  ImGui::SeparatorText( "Item spacing" );
  ImGui::Text( "x " );
  ImGui::SameLine();
  ImGui::DragFloat( "##widthSpacing", &style.ItemSpacing.x );

  ImGui::Text( "y " );
  ImGui::SameLine();
  ImGui::DragFloat( "##heightSpacing", &style.ItemSpacing.y );

  ImGui::SeparatorText( "Frame padding" );
  ImGui::Text( "x " );
  ImGui::SameLine();
  ImGui::DragFloat( "##framePaddingX", &style.FramePadding.x );

  ImGui::Text( "y " );
  ImGui::SameLine();
  ImGui::DragFloat( "##framePaddingY", &style.FramePadding.y );

  ImGui::SeparatorText( "Frame border size" );
  ImGui::DragFloat( "##frameBorderSize", &style.FrameBorderSize );

  ImGui::SeparatorText( "Frame" );
  ImGui::Text( "Rounding" );
  ImGui::SameLine();
  ImGui::DragFloat( "##frameRounding", &style.FrameRounding );

  ImGui::SeparatorText( "Scrollbar" );
  ImGui::Text( "Size" );
  ImGui::SameLine();
  ImGui::DragFloat( "##scrollbarSize", &style.ScrollbarSize );

  ImGui::Text( "Padding" );
  ImGui::SameLine();
  ImGui::DragFloat( "##scrollbarPadding", &style.ScrollbarPadding );

  ImGui::Text( "Rounding" );
  ImGui::SameLine();
  ImGui::DragFloat( "##scrollbarRounding", &style.ScrollbarRounding );

  ImGui::SeparatorText( "Window" );
  ImGui::Text( "Border hover padding" );
  ImGui::SameLine();
  ImGui::DragFloat( "##windowBorderHoverPadd", &style.WindowBorderHoverPadding );

  ImGui::Text( "Window border size" );
  ImGui::SameLine();
  ImGui::DragFloat( "##windowBorderSize", &style.WindowBorderSize );

  ImGui::Text( "Window border padding X" );
  ImGui::SameLine();
  ImGui::DragFloat( "##windowPaddingX", &style.WindowPadding.x );

  ImGui::Text( "Window border padding Y" );
  ImGui::SameLine();
  ImGui::DragFloat( "##windowPaddingY", &style.WindowPadding.y );

  ImGui::Text( "Rounding" );
  ImGui::SameLine();
  ImGui::DragFloat( "##windowRounding", &style.WindowRounding );
}

void gui::VulkanImguiRenderer::imguiModal()
{
  if ( ImGui::BeginPopupModal( "Edit imgui", &m_popups.imguiVariables, ImGuiWindowFlags_NoDocking ) )
  {
    imguiChanger();

    if ( ImGui::Button( ICON_FA_SAVE " Close" ) )
    {
      m_popups.imguiVariables = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void gui::VulkanImguiRenderer::configChanger()
{
  // TODO(kogayonon) get the config here and edit it
  auto& cfg = utilities::Configurator::getConfig();

  ImGui::SeparatorText( "File filters" );
  if ( ImGui::Button( "Add filter##file" ) )
  {
    cfg.fileFilters.push_back( { "" } );
  }
  for ( auto i = 0u; i < cfg.fileFilters.size(); i++ )
  {
    auto id = std::format( "##fileFilter{}", i );
    ImGui::InputText( id.c_str(), &cfg.fileFilters.at( i ) );
    ImGui::SameLine();
    auto buttonRemoveId = std::format( "-##folderRemove{}", cfg.fileFilters.at( i ) );
    // add or remove file filters
    if ( ImGui::Button( buttonRemoveId.c_str() ) )
    {
      cfg.fileFilters.erase( cfg.fileFilters.begin() + i );
    }
  }

  ImGui::SeparatorText( "Folder filters" );
  if ( ImGui::Button( "Add filter##folder" ) )
  {
    cfg.folderFilters.push_back( { "" } );
  }

  for ( auto i = 0u; i < cfg.folderFilters.size(); i++ )
  {
    auto id = std::format( "##folderFilter{}", i );
    ImGui::InputText( id.c_str(), &cfg.folderFilters.at( i ) );
    ImGui::SameLine();
    // add or remove folder filters
    auto buttonRemoveId = std::format( "-##folderRemove{}", cfg.folderFilters.at( i ) );
    if ( ImGui::Button( buttonRemoveId.c_str() ) )
    {
      cfg.folderFilters.erase( cfg.folderFilters.begin() + i );
    }
  }

  ImGui::SeparatorText( "Window settings" );
  ImGui::InputInt( "##height", &cfg.height );
  ImGui::InputInt( "##width", &cfg.width );
  ImGui::Checkbox( "Maximized", &cfg.maximized );
}

void gui::VulkanImguiRenderer::configModal()
{
  if ( ImGui::BeginPopupModal( "Engine config", &m_popups.configPopup, ImGuiWindowFlags_NoDocking ) )
  {
    configChanger();
    if ( ImGui::Button( " Close" ) )
    {
      m_popups.configPopup = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if ( ImGui::Button( ICON_FA_SAVE "Save" ) )
    {
      // save config
      utilities::Configurator::writeConfig();
      ImGui::CloseCurrentPopup();
      m_popups.configPopup = false;
    }

    ImGui::EndPopup();
  }
}
