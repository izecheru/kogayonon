#include "gui/imgui_manager.hpp"
#include <ImGuizmo.h>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "gui/debug_window.hpp"
#include "utilities/asset_manager/asset_manager.hpp"

namespace kogayonon_gui
{
ImGuiManager::~ImGuiManager()
{
  m_windows.clear();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

ImGuiManager::ImGuiManager( SDL_Window* window, SDL_GLContext context )
{
  if ( initImgui( window, context ) )
  {
    spdlog::info( "Imgui initialised" );
  }
}

void ImGuiManager::pushWindow( std::string name, std::unique_ptr<ImGuiWindow> window )
{
  m_windows.try_emplace( std::move( name ), std::move( window ) );
}

bool ImGuiManager::initImgui( SDL_Window* window, SDL_GLContext context )
{
  IMGUI_CHECKVERSION();
  if ( !ImGui::CreateContext() )
  {
    spdlog::error( "could not create imgui context" );
    return false;
  }

  m_io = &ImGui::GetIO();
  m_io->ConfigFlags |=
    ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
  m_io->IniFilename = "imgui_config.ini";

  m_io->ConfigWindowsMoveFromTitleBarOnly = true;

  std::string font = "resources/fonts/SGr-IosevkaTermSS18-Medium.ttc";
  ImFontConfig cfg;
  cfg.OversampleH = 3;   // horizontal oversampling
  cfg.OversampleV = 1;   // vertical oversampling
  cfg.PixelSnapH = true; // snaps glyphs to pixels (sharper)

  // make iosevka the deafult font
  ImFont* pFont = m_io->Fonts->AddFontFromFileTTF( font.c_str(), 18.0f, &cfg );
  m_io->FontDefault = pFont;

  // change the style
  ImGuiStyle& style = ImGui::GetStyle();

  // remove that slight border at the top of the tab
  style.TabBarBorderSize = 0.0f;
  style.TabBarOverlineSize = 0.0f;
  style.TabRounding = 0.0f;
  style.WindowPadding = ImVec2{ 0.0f, 0.0f };

  // colors
  ImVec4* colors = style.Colors;

  // took this color scheme from The Cherno
  colors[ImGuiCol_WindowBg] = { 0.1f, 0.105f, 0.11f, 1.0f };

  // Headers
  colors[ImGuiCol_Header] = { 0.20f, 0.205f, 0.21f, 1.0f };
  colors[ImGuiCol_HeaderHovered] = { 0.35f, 0.40f, 0.45f, 1.0f };
  colors[ImGuiCol_HeaderActive] = { 0.35f, 0.40f, 0.45f, 1.0f };

  // Buttons
  colors[ImGuiCol_Button] = { 0.20f, 0.205f, 0.21f, 1.0f };
  colors[ImGuiCol_ButtonHovered] = { 0.35f, 0.40f, 0.45f, 1.0f };
  colors[ImGuiCol_ButtonActive] = { 0.45f, 0.50f, 0.55f, 1.0f };

  // Frames
  colors[ImGuiCol_FrameBg] = { 0.20f, 0.205f, 0.21f, 1.0f };
  colors[ImGuiCol_FrameBgHovered] = { 0.35f, 0.40f, 0.45f, 1.0f };
  colors[ImGuiCol_FrameBgActive] = { 0.45f, 0.50f, 0.55f, 1.0f };

  // Tabs
  colors[ImGuiCol_Tab] = { 0.15f, 0.1505f, 0.151f, 1.0f };
  colors[ImGuiCol_TabHovered] = { 0.45f, 0.50f, 0.55f, 1.0f };
  colors[ImGuiCol_TabActive] = { 0.40f, 0.45f, 0.50f, 1.0f };
  colors[ImGuiCol_TabUnfocused] = { 0.15f, 0.1505f, 0.151f, 1.0f };
  colors[ImGuiCol_TabUnfocusedActive] = { 0.35f, 0.40f, 0.45f, 1.0f };

  // Title
  colors[ImGuiCol_TitleBg] = { 0.15f, 0.1505f, 0.151f, 1.0f };
  colors[ImGuiCol_TitleBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };
  colors[ImGuiCol_TitleBgCollapsed] = { 0.15f, 0.1505f, 0.151f, 1.0f };

  if ( !ImGui_ImplSDL2_InitForOpenGL( window, context ) || !ImGui_ImplOpenGL3_Init( "#version 460" ) )
  {
    spdlog::error( "could not init imgui" );
    return false;
  }
  return true;
}

void ImGuiManager::setupDockSpace( ImGuiViewport* viewport )
{
  static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_AutoHideTabBar;
  if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable )
  {
    const auto dockspaceId = ImGui::GetID( "MyDockspace" );
    ImGui::DockSpace( dockspaceId, ImVec2( 0, 0 ), dockspaceFlags );
    if ( !std::filesystem::exists( m_io->IniFilename ) )
    {
      // Clear previous layout
      ImGui::DockBuilderRemoveNode( dockspaceId );
      ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
      ImGui::DockBuilderSetNodeSize( dockspaceId, viewport->Size );

      auto centerNodeId = dockspaceId;
      auto leftNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Left, 0.2f, nullptr, &centerNodeId );
      auto rightNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Right, 0.2f, nullptr, &centerNodeId );
      auto bottomCenterNodeId =
        ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Down, 0.3f, nullptr, &centerNodeId );

      auto upplerLeftNodeId = ImGui::DockBuilderSplitNode( leftNodeId, ImGuiDir_Up, 0.4f, nullptr, &leftNodeId );
      auto lowerLeftNodeId = ImGui::DockBuilderSplitNode( leftNodeId, ImGuiDir_Up, 0.3f, nullptr, &leftNodeId );

      // Dock windows
      ImGui::DockBuilderDockWindow( "Debug console", bottomCenterNodeId );
      ImGui::DockBuilderDockWindow( "Assets", bottomCenterNodeId );
      ImGui::DockBuilderDockWindow( "Scene hierarchy", upplerLeftNodeId );
      ImGui::DockBuilderDockWindow( "Object properties", rightNodeId );
      ImGui::DockBuilderDockWindow( "Performance", rightNodeId );
      ImGui::DockBuilderDockWindow( "Viewport", centerNodeId );
      ImGui::DockBuilderDockWindow( "Project", centerNodeId );

      ImGui::DockBuilderFinish( dockspaceId );
    }
  }
}

void ImGuiManager::begin()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  static ImGuiViewport* viewport = ImGui::GetMainViewport();
  static float menu_bar_height = ImGui::GetFrameHeight();

  ImGui::SetNextWindowPos( ImVec2( viewport->Pos.x, viewport->Pos.y + menu_bar_height ) );
  ImGui::SetNextWindowSize( ImVec2( viewport->Size.x, viewport->Size.y - menu_bar_height ) );
  ImGui::SetNextWindowViewport( viewport->ID );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
  ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );

  ImGui::Begin( "Main", nullptr, window_flags );
  setupDockSpace( viewport );
  ImGui::PopStyleVar( 2 );
  ImGui::End();
}

void ImGuiManager::end()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

  if ( m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
  {
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    SDL_GLContext context = SDL_GL_GetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent( window, context );
  }
}

void ImGuiManager::draw()
{
  begin();
  mainMenu();
  for ( auto& win : m_windows )
  {
    win.second->draw();
  }
  payloadTooltip();
  end();
}

void ImGuiManager::payloadTooltip()
{
  // TODO make this a bit more readable since i might use many icons ore none at all and just
  // draw something representative
  static uint32_t gltfIcon = 0;
  if ( gltfIcon == 0 )
  {
    auto& assetManager = kogayonon_utilities::AssetManager::getInstance();
    if ( auto tex = assetManager.getTexture( "gltf_icon.png" ).lock() )
    {
      gltfIcon = tex->getTextureId();
    }
    else
    {
      return;
    }
  }

  if ( const ImGuiPayload* payload = ImGui::GetDragDropPayload() )
  {
    if ( payload->IsDataType( "ASSET_DROP" ) )
    {
      // TODO stylise this a bit with icons and whatnot
      ImGui::SetNextWindowSize( ImVec2{ 40.0f, 40.0f } );
      ImGui::PushStyleColor( ImGuiCol_PopupBg, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } );
      ImGui::BeginTooltip();
      auto data = static_cast<const char*>( payload->Data );
      std::string dropResult( data, payload->DataSize );
      std::filesystem::path p{ dropResult };
      if ( p.extension().string().find( ".gltf" ) != std::string::npos )
      {
        // ImGui::SetCursorPos( ImVec2{ 5.0f, 5.0f } );
        ImGui::Image( (ImTextureID)gltfIcon, ImVec2{ 40.0f, 40.0f } );
      }
      ImGui::EndTooltip();
      ImGui::PopStyleColor();
    }
  }
}

void ImGuiManager::mainMenu()
{
  const auto& pEventDispatcher = kogayonon_core::MainRegistry::getInstance().getEventDispatcher();
  if ( ImGui::BeginMainMenuBar() )
  {
    if ( ImGui::BeginMenu( "File" ) )
    {
      if ( ImGui::MenuItem( "Close" ) )
      {
        pEventDispatcher->emitEvent( kogayonon_core::WindowCloseEvent{} );
      }
      if ( ImGui::MenuItem( "Save scene" ) )
      {
      }
      ImGui::EndMenu();
    }

    if ( ImGui::BeginMenu( "Windows" ) )
    {
      if ( ImGui::MenuItem( "Hide all except viewport" ) )
      {
        for ( auto& window : m_windows )
        {
          if ( window.first != "Viewport" && window.first != "Project" )
            window.second->hide();
        }
      }
      if ( ImGui::MenuItem( "Show all" ) )
      {
        for ( auto& window : m_windows )
        {
          if ( window.first != "Viewport" && window.first != "Project" )
            window.second->show();
        }
      }
      ImGui::EndMenu();
    }
  }

  ImGui::EndMainMenuBar();
}

ImGuiManager::ImGuiWindows_Map& ImGuiManager::getWindows()
{
  return m_windows;
}
} // namespace kogayonon_gui