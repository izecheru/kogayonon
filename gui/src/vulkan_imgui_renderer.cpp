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
#include "utilities/config_manager/config_manager.hpp"
#include "utilities/fonts/fontawesome5.hpp"
#include "utilities/fonts/fontawesome6Pro.hpp"
#include "utilities/fonts/forkawesome.hpp"
#include "utilities/fonts/materialdesign.hpp"

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

    ImGuiDockNode* node = ImGui::DockBuilderGetNode( dockSpaceId );
    node->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;

    auto centerNodeId = dockSpaceId;
    const auto bottomNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Down, 0.35f, nullptr, &centerNodeId );
    const auto leftNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Left, 0.20f, nullptr, &centerNodeId );
    auto rightNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Right, 0.30f, nullptr, &centerNodeId );

    ImGui::DockBuilderDockWindow( ICON_FA_FILE " File explorer", bottomNodeId );
    ImGui::DockBuilderDockWindow( ICON_FK_ARROWS " Viewport", rightNodeId );
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

  float baseFontSize = 24.0f;
  float iconFontSize =
    baseFontSize * 2.0f /
    3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

  static const ImWchar fontawesome6Ranges[] = { ICON_MIN_FA6, ICON_MAX_16_FA6, 0 };
  static const ImWchar fontawesome5Ranges[] = { ICON_MIN_FA5, ICON_MAX_16_FA5, 0 };
  static const ImWchar materialdesignRanges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
  static const ImWchar forkAwesomeRanges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };

  ImFontConfig iconsConfig;
  iconsConfig.PixelSnapH = true;
  iconsConfig.GlyphMinAdvanceX = iconFontSize;
  iconsConfig.MergeMode = true;

  ImFontConfig materialdesignConfig;
  materialdesignConfig.PixelSnapH = true;
  materialdesignConfig.GlyphMinAdvanceX = baseFontSize;
  materialdesignConfig.MergeMode = true;

  m_fonts.emplace( "inter",
                   io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/Inter_18pt-Medium.ttf", baseFontSize, &cfg ) );

  io.FontDefault = m_fonts.at( "inter" );

  m_fonts.emplace(
    "forkawesome",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/forkawesome-webfont.ttf", iconFontSize, &iconsConfig, forkAwesomeRanges ) );

  m_fonts.emplace( "materialdesign",
                   io.Fonts->AddFontFromFileTTF( "engine_resources/fonts/materialdesignicons-webfont.ttf",
                                                 iconFontSize,
                                                 &materialdesignConfig,
                                                 materialdesignRanges ) );

  m_fonts.emplace(
    "fa6-brands-400",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/fontawesome/fa6-brands-400.otf", iconFontSize, &iconsConfig, fontawesome6Ranges ) );

  m_fonts.emplace(
    "fa6-regular-400",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/fontawesome/fa6-regular-400.otf", iconFontSize, &iconsConfig, fontawesome6Ranges ) );

  m_fonts.emplace(
    "fa6-solid-900",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/fontawesome/fa6-solid-900.otf", iconFontSize, &iconsConfig, fontawesome6Ranges ) );

  m_fonts.emplace(
    "fa5-regular-400",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/fontawesome/fa5-regular-400.ttf", iconFontSize, &iconsConfig, fontawesome5Ranges ) );

  m_fonts.emplace(
    "fa5-solid-900",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/fontawesome/fa5-solid-900.ttf", iconFontSize, &iconsConfig, fontawesome5Ranges ) );

  m_fonts.emplace(
    "fa5-brands-400",
    io.Fonts->AddFontFromFileTTF(
      "engine_resources/fonts/fontawesome/fa5-brands-400.ttf", iconFontSize, &iconsConfig, fontawesome5Ranges ) );

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
  style.WindowRounding = 0.0f;
  style.GrabRounding = style.FrameRounding = 2.3f;
  style.ScrollbarRounding = 5.0f;
  style.FrameBorderSize = 1.0f;
  style.ItemSpacing.y = 6.5f;
  style.TabRounding = 0.0f;
  style.TabBorderSize = 0.0f;

  auto& colorCfg = utilities::EditorConfigManager::getColorConfig();
  setColorPallete( colorCfg );

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

  m_windows.emplace( "Viewport", std::make_unique<Viewport>( m_wnd, ICON_FK_ARROWS " Viewport", viewportSpec ) );

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
        m_popups.imguiVariablesPopup = true;
      }

      ImGui::EndMenu();
    }
  }
  ImGui::EndMainMenuBar();

  if ( m_popups.colorChangerPopup )
    ImGui::OpenPopup( "Color settings" );

  if ( m_popups.imguiVariablesPopup )
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
    if ( ImGui::Button( ICON_MDI_CLOSE "Close" ) )
    {
      m_popups.colorChangerPopup = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine( 0.0f, 10.0f );
    if ( ImGui::Button( ICON_FA_SAVE "Save config" ) )
    {
      // save config
      changeColorConfig();

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
  if ( ImGui::BeginPopupModal( "Edit imgui", &m_popups.imguiVariablesPopup, ImGuiWindowFlags_NoDocking ) )
  {
    imguiChanger();

    if ( ImGui::Button( ICON_FA_SAVE " Close" ) )
    {
      m_popups.imguiVariablesPopup = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void gui::VulkanImguiRenderer::configChanger()
{
  // TODO(kogayonon) get the config here and edit it
  auto& cfg = utilities::EditorConfigManager::getConfig();

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
      utilities::EditorConfigManager::writeConfig();

      m_popups.configPopup = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void gui::VulkanImguiRenderer::setColorPallete( const utilities::ColorConfig& cfg )
{
  auto& style = ImGui::GetStyle();

  style.Colors[ImGuiCol_Text] = {
    cfg.ImGuiCol_Text.r, cfg.ImGuiCol_Text.g, cfg.ImGuiCol_Text.b, cfg.ImGuiCol_Text.alpha };

  style.Colors[ImGuiCol_ModalWindowDimBg] = { cfg.ImGuiCol_ModalWindowDimBg.r,
                                              cfg.ImGuiCol_ModalWindowDimBg.g,
                                              cfg.ImGuiCol_ModalWindowDimBg.b,
                                              cfg.ImGuiCol_ModalWindowDimBg.alpha };

  style.Colors[ImGuiCol_TextDisabled] = { cfg.ImGuiCol_TextDisabled.r,
                                          cfg.ImGuiCol_TextDisabled.g,
                                          cfg.ImGuiCol_TextDisabled.b,
                                          cfg.ImGuiCol_TextDisabled.alpha };

  style.Colors[ImGuiCol_WindowBg] = {
    cfg.ImGuiCol_WindowBg.r, cfg.ImGuiCol_WindowBg.g, cfg.ImGuiCol_WindowBg.b, cfg.ImGuiCol_WindowBg.alpha };

  style.Colors[ImGuiCol_ChildBg] = {
    cfg.ImGuiCol_ChildBg.r, cfg.ImGuiCol_ChildBg.g, cfg.ImGuiCol_ChildBg.b, cfg.ImGuiCol_ChildBg.alpha };

  style.Colors[ImGuiCol_PopupBg] = {
    cfg.ImGuiCol_PopupBg.r, cfg.ImGuiCol_PopupBg.g, cfg.ImGuiCol_PopupBg.b, cfg.ImGuiCol_PopupBg.alpha };

  style.Colors[ImGuiCol_Border] = {
    cfg.ImGuiCol_Border.r, cfg.ImGuiCol_Border.g, cfg.ImGuiCol_Border.b, cfg.ImGuiCol_Border.alpha };

  style.Colors[ImGuiCol_BorderShadow] = { cfg.ImGuiCol_BorderShadow.r,
                                          cfg.ImGuiCol_BorderShadow.g,
                                          cfg.ImGuiCol_BorderShadow.b,
                                          cfg.ImGuiCol_BorderShadow.alpha };

  style.Colors[ImGuiCol_TabHovered] = {
    cfg.ImGuiCol_TabHovered.r, cfg.ImGuiCol_TabHovered.g, cfg.ImGuiCol_TabHovered.b, cfg.ImGuiCol_TabHovered.alpha };

  style.Colors[ImGuiCol_TabActive] = {
    cfg.ImGuiCol_TabActive.r, cfg.ImGuiCol_TabActive.g, cfg.ImGuiCol_TabActive.b, cfg.ImGuiCol_TabActive.alpha };

  style.Colors[ImGuiCol_TabSelected] = { cfg.ImGuiCol_TabSelected.r,
                                         cfg.ImGuiCol_TabSelected.g,
                                         cfg.ImGuiCol_TabSelected.b,
                                         cfg.ImGuiCol_TabSelected.alpha };

  style.Colors[ImGuiCol_TabSelectedOverline] = { cfg.ImGuiCol_TabSelectedOverline.r,
                                                 cfg.ImGuiCol_TabSelectedOverline.g,
                                                 cfg.ImGuiCol_TabSelectedOverline.b,
                                                 cfg.ImGuiCol_TabSelectedOverline.alpha };

  style.Colors[ImGuiCol_TabDimmed] = {
    cfg.ImGuiCol_TabDimmed.r, cfg.ImGuiCol_TabDimmed.g, cfg.ImGuiCol_TabDimmed.b, cfg.ImGuiCol_TabDimmed.alpha };

  style.Colors[ImGuiCol_TabDimmedSelected] = { cfg.ImGuiCol_TabDimmedSelected.r,
                                               cfg.ImGuiCol_TabDimmedSelected.g,
                                               cfg.ImGuiCol_TabDimmedSelected.b,
                                               cfg.ImGuiCol_TabDimmedSelected.alpha };

  style.Colors[ImGuiCol_FrameBg] = {
    cfg.ImGuiCol_FrameBg.r, cfg.ImGuiCol_FrameBg.g, cfg.ImGuiCol_FrameBg.b, cfg.ImGuiCol_FrameBg.alpha };

  style.Colors[ImGuiCol_FrameBgHovered] = { cfg.ImGuiCol_FrameBgHovered.r,
                                            cfg.ImGuiCol_FrameBgHovered.g,
                                            cfg.ImGuiCol_FrameBgHovered.b,
                                            cfg.ImGuiCol_FrameBgHovered.alpha };

  style.Colors[ImGuiCol_FrameBgActive] = { cfg.ImGuiCol_FrameBgActive.r,
                                           cfg.ImGuiCol_FrameBgActive.g,
                                           cfg.ImGuiCol_FrameBgActive.b,
                                           cfg.ImGuiCol_FrameBgActive.alpha };

  style.Colors[ImGuiCol_TitleBg] = {
    cfg.ImGuiCol_TitleBg.r, cfg.ImGuiCol_TitleBg.g, cfg.ImGuiCol_TitleBg.b, cfg.ImGuiCol_TitleBg.alpha };

  style.Colors[ImGuiCol_TitleBgCollapsed] = { cfg.ImGuiCol_TitleBgCollapsed.r,
                                              cfg.ImGuiCol_TitleBgCollapsed.g,
                                              cfg.ImGuiCol_TitleBgCollapsed.b,
                                              cfg.ImGuiCol_TitleBgCollapsed.alpha };

  style.Colors[ImGuiCol_TitleBgActive] = { cfg.ImGuiCol_TitleBgActive.r,
                                           cfg.ImGuiCol_TitleBgActive.g,
                                           cfg.ImGuiCol_TitleBgActive.b,
                                           cfg.ImGuiCol_TitleBgActive.alpha };

  style.Colors[ImGuiCol_MenuBarBg] = {
    cfg.ImGuiCol_MenuBarBg.r, cfg.ImGuiCol_MenuBarBg.g, cfg.ImGuiCol_MenuBarBg.b, cfg.ImGuiCol_MenuBarBg.alpha };

  style.Colors[ImGuiCol_ScrollbarBg] = { cfg.ImGuiCol_ScrollbarBg.r,
                                         cfg.ImGuiCol_ScrollbarBg.g,
                                         cfg.ImGuiCol_ScrollbarBg.b,
                                         cfg.ImGuiCol_ScrollbarBg.alpha };

  style.Colors[ImGuiCol_ScrollbarGrab] = { cfg.ImGuiCol_ScrollbarGrab.r,
                                           cfg.ImGuiCol_ScrollbarGrab.g,
                                           cfg.ImGuiCol_ScrollbarGrab.b,
                                           cfg.ImGuiCol_ScrollbarGrab.alpha };

  style.Colors[ImGuiCol_ScrollbarGrabHovered] = { cfg.ImGuiCol_ScrollbarGrabHovered.r,
                                                  cfg.ImGuiCol_ScrollbarGrabHovered.g,
                                                  cfg.ImGuiCol_ScrollbarGrabHovered.b,
                                                  cfg.ImGuiCol_ScrollbarGrabHovered.alpha };

  style.Colors[ImGuiCol_ScrollbarGrabActive] = { cfg.ImGuiCol_ScrollbarGrabActive.r,
                                                 cfg.ImGuiCol_ScrollbarGrabActive.g,
                                                 cfg.ImGuiCol_ScrollbarGrabActive.b,
                                                 cfg.ImGuiCol_ScrollbarGrabActive.alpha };

  style.Colors[ImGuiCol_CheckMark] = {
    cfg.ImGuiCol_CheckMark.r, cfg.ImGuiCol_CheckMark.g, cfg.ImGuiCol_CheckMark.b, cfg.ImGuiCol_CheckMark.alpha };

  style.Colors[ImGuiCol_SliderGrab] = {
    cfg.ImGuiCol_SliderGrab.r, cfg.ImGuiCol_SliderGrab.g, cfg.ImGuiCol_SliderGrab.b, cfg.ImGuiCol_SliderGrab.alpha };

  style.Colors[ImGuiCol_SliderGrabActive] = { cfg.ImGuiCol_SliderGrabActive.r,
                                              cfg.ImGuiCol_SliderGrabActive.g,
                                              cfg.ImGuiCol_SliderGrabActive.b,
                                              cfg.ImGuiCol_SliderGrabActive.alpha };

  style.Colors[ImGuiCol_Button] = {
    cfg.ImGuiCol_Button.r, cfg.ImGuiCol_Button.g, cfg.ImGuiCol_Button.b, cfg.ImGuiCol_Button.alpha };

  style.Colors[ImGuiCol_ButtonHovered] = { cfg.ImGuiCol_ButtonHovered.r,
                                           cfg.ImGuiCol_ButtonHovered.g,
                                           cfg.ImGuiCol_ButtonHovered.b,
                                           cfg.ImGuiCol_ButtonHovered.alpha };

  style.Colors[ImGuiCol_ButtonActive] = { cfg.ImGuiCol_ButtonActive.r,
                                          cfg.ImGuiCol_ButtonActive.g,
                                          cfg.ImGuiCol_ButtonActive.b,
                                          cfg.ImGuiCol_ButtonActive.alpha };

  style.Colors[ImGuiCol_Header] = {
    cfg.ImGuiCol_Header.r, cfg.ImGuiCol_Header.g, cfg.ImGuiCol_Header.b, cfg.ImGuiCol_Header.alpha };

  style.Colors[ImGuiCol_HeaderHovered] = { cfg.ImGuiCol_HeaderHovered.r,
                                           cfg.ImGuiCol_HeaderHovered.g,
                                           cfg.ImGuiCol_HeaderHovered.b,
                                           cfg.ImGuiCol_HeaderHovered.alpha };

  style.Colors[ImGuiCol_HeaderActive] = { cfg.ImGuiCol_HeaderActive.r,
                                          cfg.ImGuiCol_HeaderActive.g,
                                          cfg.ImGuiCol_HeaderActive.b,
                                          cfg.ImGuiCol_HeaderActive.alpha };

  style.Colors[ImGuiCol_Separator] = {
    cfg.ImGuiCol_Separator.r, cfg.ImGuiCol_Separator.g, cfg.ImGuiCol_Separator.b, cfg.ImGuiCol_Separator.alpha };

  style.Colors[ImGuiCol_SeparatorHovered] = { cfg.ImGuiCol_SeparatorHovered.r,
                                              cfg.ImGuiCol_SeparatorHovered.g,
                                              cfg.ImGuiCol_SeparatorHovered.b,
                                              cfg.ImGuiCol_SeparatorHovered.alpha };

  style.Colors[ImGuiCol_SeparatorActive] = { cfg.ImGuiCol_SeparatorActive.r,
                                             cfg.ImGuiCol_SeparatorActive.g,
                                             cfg.ImGuiCol_SeparatorActive.b,
                                             cfg.ImGuiCol_SeparatorActive.alpha };

  style.Colors[ImGuiCol_ResizeGrip] = {
    cfg.ImGuiCol_ResizeGrip.r, cfg.ImGuiCol_ResizeGrip.g, cfg.ImGuiCol_ResizeGrip.b, cfg.ImGuiCol_ResizeGrip.alpha };

  style.Colors[ImGuiCol_ResizeGripHovered] = { cfg.ImGuiCol_ResizeGripHovered.r,
                                               cfg.ImGuiCol_ResizeGripHovered.g,
                                               cfg.ImGuiCol_ResizeGripHovered.b,
                                               cfg.ImGuiCol_ResizeGripHovered.alpha };

  style.Colors[ImGuiCol_ResizeGripActive] = { cfg.ImGuiCol_ResizeGripActive.r,
                                              cfg.ImGuiCol_ResizeGripActive.g,
                                              cfg.ImGuiCol_ResizeGripActive.b,
                                              cfg.ImGuiCol_ResizeGripActive.alpha };

  style.Colors[ImGuiCol_PlotLines] = {
    cfg.ImGuiCol_PlotLines.r, cfg.ImGuiCol_PlotLines.g, cfg.ImGuiCol_PlotLines.b, cfg.ImGuiCol_PlotLines.alpha };

  style.Colors[ImGuiCol_PlotLinesHovered] = { cfg.ImGuiCol_PlotLinesHovered.r,
                                              cfg.ImGuiCol_PlotLinesHovered.g,
                                              cfg.ImGuiCol_PlotLinesHovered.b,
                                              cfg.ImGuiCol_PlotLinesHovered.alpha };

  style.Colors[ImGuiCol_PlotHistogram] = { cfg.ImGuiCol_PlotHistogram.r,
                                           cfg.ImGuiCol_PlotHistogram.g,
                                           cfg.ImGuiCol_PlotHistogram.b,
                                           cfg.ImGuiCol_PlotHistogram.alpha };

  style.Colors[ImGuiCol_PlotHistogramHovered] = { cfg.ImGuiCol_PlotHistogramHovered.r,
                                                  cfg.ImGuiCol_PlotHistogramHovered.g,
                                                  cfg.ImGuiCol_PlotHistogramHovered.b,
                                                  cfg.ImGuiCol_PlotHistogramHovered.alpha };

  style.Colors[ImGuiCol_TextSelectedBg] = { cfg.ImGuiCol_TextSelectedBg.r,
                                            cfg.ImGuiCol_TextSelectedBg.g,
                                            cfg.ImGuiCol_TextSelectedBg.b,
                                            cfg.ImGuiCol_TextSelectedBg.alpha };
}

void gui::VulkanImguiRenderer::changeColorConfig()
{
  auto& cfg = utilities::EditorConfigManager::getColorConfig();
  auto& style = ImGui::GetStyle();

  cfg.ImGuiCol_Text = utilities::ColorVec4{ style.Colors[ImGuiCol_Text].x,
                                            style.Colors[ImGuiCol_Text].y,
                                            style.Colors[ImGuiCol_Text].z,
                                            style.Colors[ImGuiCol_Text].w };

  cfg.ImGuiCol_ModalWindowDimBg = utilities::ColorVec4{ style.Colors[ImGuiCol_ModalWindowDimBg].x,
                                                        style.Colors[ImGuiCol_ModalWindowDimBg].y,
                                                        style.Colors[ImGuiCol_ModalWindowDimBg].z,
                                                        style.Colors[ImGuiCol_ModalWindowDimBg].w };

  cfg.ImGuiCol_TextDisabled = utilities::ColorVec4{ style.Colors[ImGuiCol_TextDisabled].x,
                                                    style.Colors[ImGuiCol_TextDisabled].y,
                                                    style.Colors[ImGuiCol_TextDisabled].z,
                                                    style.Colors[ImGuiCol_TextDisabled].w };

  cfg.ImGuiCol_WindowBg = utilities::ColorVec4{ style.Colors[ImGuiCol_WindowBg].x,
                                                style.Colors[ImGuiCol_WindowBg].y,
                                                style.Colors[ImGuiCol_WindowBg].z,
                                                style.Colors[ImGuiCol_WindowBg].w };

  cfg.ImGuiCol_ChildBg = utilities::ColorVec4{ style.Colors[ImGuiCol_ChildBg].x,
                                               style.Colors[ImGuiCol_ChildBg].y,
                                               style.Colors[ImGuiCol_ChildBg].z,
                                               style.Colors[ImGuiCol_ChildBg].w };

  cfg.ImGuiCol_PopupBg = utilities::ColorVec4{ style.Colors[ImGuiCol_PopupBg].x,
                                               style.Colors[ImGuiCol_PopupBg].y,
                                               style.Colors[ImGuiCol_PopupBg].z,
                                               style.Colors[ImGuiCol_PopupBg].w };

  cfg.ImGuiCol_Border = utilities::ColorVec4{ style.Colors[ImGuiCol_Border].x,
                                              style.Colors[ImGuiCol_Border].y,
                                              style.Colors[ImGuiCol_Border].z,
                                              style.Colors[ImGuiCol_Border].w };

  cfg.ImGuiCol_BorderShadow = utilities::ColorVec4{ style.Colors[ImGuiCol_BorderShadow].x,
                                                    style.Colors[ImGuiCol_BorderShadow].y,
                                                    style.Colors[ImGuiCol_BorderShadow].z,
                                                    style.Colors[ImGuiCol_BorderShadow].w };

  cfg.ImGuiCol_TabHovered = utilities::ColorVec4{ style.Colors[ImGuiCol_TabHovered].x,
                                                  style.Colors[ImGuiCol_TabHovered].y,
                                                  style.Colors[ImGuiCol_TabHovered].z,
                                                  style.Colors[ImGuiCol_TabHovered].w };

  cfg.ImGuiCol_TabSelected = utilities::ColorVec4{ style.Colors[ImGuiCol_TabSelected].x,
                                                   style.Colors[ImGuiCol_TabSelected].y,
                                                   style.Colors[ImGuiCol_TabSelected].z,
                                                   style.Colors[ImGuiCol_TabSelected].w };

  cfg.ImGuiCol_TabDimmed = utilities::ColorVec4{ style.Colors[ImGuiCol_TabDimmed].x,
                                                 style.Colors[ImGuiCol_TabDimmed].y,
                                                 style.Colors[ImGuiCol_TabDimmed].z,
                                                 style.Colors[ImGuiCol_TabDimmed].w };

  cfg.ImGuiCol_FrameBg = utilities::ColorVec4{ style.Colors[ImGuiCol_FrameBg].x,
                                               style.Colors[ImGuiCol_FrameBg].y,
                                               style.Colors[ImGuiCol_FrameBg].z,
                                               style.Colors[ImGuiCol_FrameBg].w };

  cfg.ImGuiCol_FrameBgHovered = utilities::ColorVec4{ style.Colors[ImGuiCol_FrameBgHovered].x,
                                                      style.Colors[ImGuiCol_FrameBgHovered].y,
                                                      style.Colors[ImGuiCol_FrameBgHovered].z,
                                                      style.Colors[ImGuiCol_FrameBgHovered].w };

  cfg.ImGuiCol_FrameBgActive = utilities::ColorVec4{ style.Colors[ImGuiCol_FrameBgActive].x,
                                                     style.Colors[ImGuiCol_FrameBgActive].y,
                                                     style.Colors[ImGuiCol_FrameBgActive].z,
                                                     style.Colors[ImGuiCol_FrameBgActive].w };

  cfg.ImGuiCol_TitleBg = utilities::ColorVec4{ style.Colors[ImGuiCol_TitleBg].x,
                                               style.Colors[ImGuiCol_TitleBg].y,
                                               style.Colors[ImGuiCol_TitleBg].z,
                                               style.Colors[ImGuiCol_TitleBg].w };

  cfg.ImGuiCol_TitleBgActive = utilities::ColorVec4{ style.Colors[ImGuiCol_TitleBgActive].x,
                                                     style.Colors[ImGuiCol_TitleBgActive].y,
                                                     style.Colors[ImGuiCol_TitleBgActive].z,
                                                     style.Colors[ImGuiCol_TitleBgActive].w };

  cfg.ImGuiCol_MenuBarBg = utilities::ColorVec4{ style.Colors[ImGuiCol_MenuBarBg].x,
                                                 style.Colors[ImGuiCol_MenuBarBg].y,
                                                 style.Colors[ImGuiCol_MenuBarBg].z,
                                                 style.Colors[ImGuiCol_MenuBarBg].w };

  cfg.ImGuiCol_ScrollbarBg = utilities::ColorVec4{ style.Colors[ImGuiCol_ScrollbarBg].x,
                                                   style.Colors[ImGuiCol_ScrollbarBg].y,
                                                   style.Colors[ImGuiCol_ScrollbarBg].z,
                                                   style.Colors[ImGuiCol_ScrollbarBg].w };

  cfg.ImGuiCol_ScrollbarGrab = utilities::ColorVec4{ style.Colors[ImGuiCol_ScrollbarGrab].x,
                                                     style.Colors[ImGuiCol_ScrollbarGrab].y,
                                                     style.Colors[ImGuiCol_ScrollbarGrab].z,
                                                     style.Colors[ImGuiCol_ScrollbarGrab].w };

  cfg.ImGuiCol_ScrollbarGrabHovered = utilities::ColorVec4{ style.Colors[ImGuiCol_ScrollbarGrabHovered].x,
                                                            style.Colors[ImGuiCol_ScrollbarGrabHovered].y,
                                                            style.Colors[ImGuiCol_ScrollbarGrabHovered].z,
                                                            style.Colors[ImGuiCol_ScrollbarGrabHovered].w };

  cfg.ImGuiCol_ScrollbarGrabActive = utilities::ColorVec4{ style.Colors[ImGuiCol_ScrollbarGrabActive].x,
                                                           style.Colors[ImGuiCol_ScrollbarGrabActive].y,
                                                           style.Colors[ImGuiCol_ScrollbarGrabActive].z,
                                                           style.Colors[ImGuiCol_ScrollbarGrabActive].w };

  cfg.ImGuiCol_CheckMark = utilities::ColorVec4{ style.Colors[ImGuiCol_CheckMark].x,
                                                 style.Colors[ImGuiCol_CheckMark].y,
                                                 style.Colors[ImGuiCol_CheckMark].z,
                                                 style.Colors[ImGuiCol_CheckMark].w };

  cfg.ImGuiCol_SliderGrabActive = utilities::ColorVec4{ style.Colors[ImGuiCol_SliderGrabActive].x,
                                                        style.Colors[ImGuiCol_SliderGrabActive].y,
                                                        style.Colors[ImGuiCol_SliderGrabActive].z,
                                                        style.Colors[ImGuiCol_SliderGrabActive].w };

  cfg.ImGuiCol_Button = utilities::ColorVec4{ style.Colors[ImGuiCol_Button].x,
                                              style.Colors[ImGuiCol_Button].y,
                                              style.Colors[ImGuiCol_Button].z,
                                              style.Colors[ImGuiCol_Button].w };

  cfg.ImGuiCol_ButtonHovered = utilities::ColorVec4{ style.Colors[ImGuiCol_ButtonHovered].x,
                                                     style.Colors[ImGuiCol_ButtonHovered].y,
                                                     style.Colors[ImGuiCol_ButtonHovered].z,
                                                     style.Colors[ImGuiCol_ButtonHovered].w };

  cfg.ImGuiCol_ButtonActive = utilities::ColorVec4{ style.Colors[ImGuiCol_ButtonActive].x,
                                                    style.Colors[ImGuiCol_ButtonActive].y,
                                                    style.Colors[ImGuiCol_ButtonActive].z,
                                                    style.Colors[ImGuiCol_ButtonActive].w };

  cfg.ImGuiCol_Header = utilities::ColorVec4{ style.Colors[ImGuiCol_Header].x,
                                              style.Colors[ImGuiCol_Header].y,
                                              style.Colors[ImGuiCol_Header].z,
                                              style.Colors[ImGuiCol_Header].w };

  cfg.ImGuiCol_HeaderHovered = utilities::ColorVec4{ style.Colors[ImGuiCol_HeaderHovered].x,
                                                     style.Colors[ImGuiCol_HeaderHovered].y,
                                                     style.Colors[ImGuiCol_HeaderHovered].z,
                                                     style.Colors[ImGuiCol_HeaderHovered].w };

  cfg.ImGuiCol_HeaderActive = utilities::ColorVec4{ style.Colors[ImGuiCol_HeaderActive].x,
                                                    style.Colors[ImGuiCol_HeaderActive].y,
                                                    style.Colors[ImGuiCol_HeaderActive].z,
                                                    style.Colors[ImGuiCol_HeaderActive].w };

  cfg.ImGuiCol_Separator = utilities::ColorVec4{ style.Colors[ImGuiCol_Separator].x,
                                                 style.Colors[ImGuiCol_Separator].y,
                                                 style.Colors[ImGuiCol_Separator].z,
                                                 style.Colors[ImGuiCol_Separator].w };

  cfg.ImGuiCol_SeparatorHovered = utilities::ColorVec4{ style.Colors[ImGuiCol_SeparatorHovered].x,
                                                        style.Colors[ImGuiCol_SeparatorHovered].y,
                                                        style.Colors[ImGuiCol_SeparatorHovered].z,
                                                        style.Colors[ImGuiCol_SeparatorHovered].w };

  cfg.ImGuiCol_SeparatorActive = utilities::ColorVec4{ style.Colors[ImGuiCol_SeparatorActive].x,
                                                       style.Colors[ImGuiCol_SeparatorActive].y,
                                                       style.Colors[ImGuiCol_SeparatorActive].z,
                                                       style.Colors[ImGuiCol_SeparatorActive].w };

  cfg.ImGuiCol_ResizeGrip = utilities::ColorVec4{ style.Colors[ImGuiCol_ResizeGrip].x,
                                                  style.Colors[ImGuiCol_ResizeGrip].y,
                                                  style.Colors[ImGuiCol_ResizeGrip].z,
                                                  style.Colors[ImGuiCol_ResizeGrip].w };

  cfg.ImGuiCol_ResizeGripActive = utilities::ColorVec4{ style.Colors[ImGuiCol_ResizeGripActive].x,
                                                        style.Colors[ImGuiCol_ResizeGripActive].y,
                                                        style.Colors[ImGuiCol_ResizeGripActive].z,
                                                        style.Colors[ImGuiCol_ResizeGripActive].w };

  //.ImGuiCol_PlotLinesHovered = { 1.00f, 0.43f, 0.35f, 1.00f }, .ImGuiCol_PlotHistogram = { 0.90f, 0.70f, 0.00f, 1.00f
  //}; .ImGuiCol_PlotHistogramHovered = { 1.00f, 0.60f, 0.00f, 1.00f }; .ImGuiCol_TextSelectedBg = { 0.18431373f,
  // 0.39607847f, 0.79215693f, 0.90f };
  utilities::EditorConfigManager::writeColorConfig();
}
