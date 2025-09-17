#include "gui/imgui_manager.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <imgui_internal.h>
#include "gui/debug_window.h"
#include "logger/logger.h"
#include "utilities/fonts/icons_fontawesome5.h"

using namespace kogayonon_logger;

namespace kogayonon_gui
{
ImGuiManager::~ImGuiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    m_windows.clear();
}

ImGuiManager::ImGuiManager( SDL_Window* window, SDL_GLContext context )
{
    if ( initImgui( window, context ) )
    {
        Logger::log( LogType::INFO, "Imgui initialised" );

        // add the callback for the debug console window
        Logger::addCallback( []( const std::string& msg ) { DebugConsoleWindow::log( msg ); } );
    }
    else
    {
        Logger::log( LogType::ERROR, "Imgui could not be initialised" );
    }
}

void ImGuiManager::pushWindow( std::string name, std::unique_ptr<ImGuiWindow> window )
{
    m_windows.emplace( std::move( name ), std::move( window ) );
}

bool ImGuiManager::initImgui( SDL_Window* window, SDL_GLContext context )
{
    IMGUI_CHECKVERSION();
    if ( !ImGui::CreateContext() )
    {
        Logger::error( "could not create imgui context" );
        return false;
    }

    m_io = &ImGui::GetIO();
    m_io->ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

    m_io->ConfigWindowsMoveFromTitleBarOnly = true;
    m_io->IniFilename = "imgui_config.ini";
    m_io->Fonts->AddFontDefault();

    float baseFontSize = 13.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    std::string fontPath = "resources/fonts/";
    fontPath += FONT_ICON_FILE_NAME_FAS;

    assert( std::filesystem::exists( fontPath ) && "font does not exits" );

    m_io->Fonts->AddFontFromFileTTF( fontPath.c_str(), iconFontSize, &icons_config, icons_ranges );

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::StyleColorsDark();

    if ( !ImGui_ImplSDL2_InitForOpenGL( window, context ) || !ImGui_ImplOpenGL3_Init( "#version 460" ) )
    {
        Logger::error( "could not init imgui" );
        return false;
    }
    return true;
}

void ImGuiManager::setupDockSpace( ImGuiViewport* viewport )
{
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
    if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable )
    {
        const auto dockspaceId = ImGui::GetID( "MyDockspace" );
        ImGui::DockSpace( dockspaceId, ImVec2( 0, 0 ), dockspaceFlags );
        if ( static bool firstFrame = true; firstFrame )
        {
            firstFrame = false;

            // Clear previous layout
            ImGui::DockBuilderRemoveNode( dockspaceId );
            ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
            ImGui::DockBuilderSetNodeSize( dockspaceId, viewport->Size );

            auto centerNodeId = dockspaceId;
            auto leftNodeId = ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Left, 0.3f, nullptr, &centerNodeId );
            auto rightNodeId =
                ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Right, 0.3f, nullptr, &centerNodeId );
            auto bottomCenterNodeId =
                ImGui::DockBuilderSplitNode( centerNodeId, ImGuiDir_Down, 0.3f, nullptr, &centerNodeId );

            // Dock windows
            ImGui::DockBuilderDockWindow( "Scene hierarchy", leftNodeId );
            ImGui::DockBuilderDockWindow( ICON_FA_IMAGE " Scene", centerNodeId );
            ImGui::DockBuilderDockWindow( "Debug console", bottomCenterNodeId );
            ImGui::DockBuilderDockWindow( "Assets", rightNodeId );

            ImGui::DockBuilderFinish( dockspaceId );
        }
    }
}

void ImGuiManager::begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                           ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    static ImGuiViewport* viewport = ImGui::GetMainViewport();
    static float menu_bar_height = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos( ImVec2( viewport->Pos.x, viewport->Pos.y + menu_bar_height ) );
    ImGui::SetNextWindowSize( ImVec2( viewport->Size.x, viewport->Size.y - menu_bar_height ) );
    ImGui::SetNextWindowViewport( viewport->ID );

    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
    ImGui::Begin( "Main window", nullptr, window_flags );
    ImGui::PopStyleVar( 2 );

    setupDockSpace( viewport );
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
    end();
}

void ImGuiManager::mainMenu()
{
    if ( ImGui::BeginMainMenuBar() )
    {
        if ( ImGui::BeginMenu( "File" ) )
        {
            if ( ImGui::MenuItem( "Close", "Ctrl+X" ) )
            {
                Logger::info( "Close pressed" );
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

ImGuiManager::ImGuiWindows_Map& ImGuiManager::getWindows()
{
    return m_windows;
}
} // namespace kogayonon_gui