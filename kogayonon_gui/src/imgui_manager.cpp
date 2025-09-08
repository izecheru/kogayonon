#include "gui/imgui_manager.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <imgui_internal.h>
#include "gui/debug_window.h"
#include "logger/logger.h"

using namespace kogayonon_logger;

namespace kogayonon_gui {
ImGuiManager::~ImGuiManager()
{
    Logger::info("imgui manager dtor");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    m_windows.clear();
}

ImGuiManager::ImGuiManager(SDL_Window* window, SDL_GLContext context)
{
    if (initImgui(window, context))
    {
        Logger::log(LogType::INFO, "Imgui initialised");

        // add the callback for the debug console window
        Logger::addCallback([](const std::string& msg) { DebugConsoleWindow::log(msg); });
    }
    else
    {
        Logger::log(LogType::ERROR, "Imgui could not be initialised");
    }
}

void ImGuiManager::pushWindow(std::string name, std::unique_ptr<ImGuiWindow> window)
{
    m_windows.emplace(std::move(name), std::move(window));
}

bool ImGuiManager::initImgui(SDL_Window* window, SDL_GLContext context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    m_io = &ImGui::GetIO();
    m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    m_io->IniFilename = "imgui_config.ini";

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForOpenGL(window, context) || !ImGui_ImplOpenGL3_Init("#version 460"))
    {
        Logger::log(LogType::ERROR, "Error init imgui");
        return false;
    }
    return true;
}

void ImGuiManager::setupDockSpace(ImGuiViewport* viewport)
{
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
        static bool first_frame = true;
        if (first_frame)
        {
            first_frame = false;

            ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

            // Clear previous layout
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // Split main dockspace
            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_id_right, dock_id_left, dock_id_bottom, dock_id_top;

            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.2f, &dock_id_top, &dock_main_id);
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, &dock_id_bottom, &dock_main_id);
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, &dock_id_right, &dock_main_id);
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, &dock_id_left, &dock_main_id);

            // Dock windows
            ImGui::DockBuilderDockWindow("Debug console", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Scene", dock_main_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
}

void ImGuiManager::beginImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                           ImGuiWindowFlags_NoNavFocus;

    static ImGuiViewport* viewport = ImGui::GetMainViewport();
    static float menu_bar_height = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menu_bar_height));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menu_bar_height));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Main window", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    setupDockSpace(viewport);
    ImGui::End();
}

void ImGuiManager::endImGuiFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* window = SDL_GL_GetCurrentWindow();
        SDL_GLContext context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(window, context);
    }
}

void ImGuiManager::draw()
{
    beginImGuiFrame();
    mainMenu();
    for (auto& win : m_windows)
    {
        win.second->draw();
    }
    endImGuiFrame();
}

void ImGuiManager::mainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close", "Ctrl+X"))
            {
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