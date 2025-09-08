#pragma once
#include <SDL.h>
#include "imgui_window.h"

namespace kogayonon_gui {
class ImGuiManager
{
    using ImGuiWindows_Map = std::unordered_map<std::string, std::unique_ptr<ImGuiWindow>>;

  public:
    explicit ImGuiManager( SDL_Window* window, SDL_GLContext context );
    ~ImGuiManager();

    bool initImgui( SDL_Window* window, SDL_GLContext context );
    void pushWindow( std::string name, std::unique_ptr<ImGuiWindow> window );
    ImGuiWindows_Map& getWindows();
    void setupDockSpace( ImGuiViewport* viewport );

    void draw();
    void mainMenu();
    void beginImGuiFrame();
    void endImGuiFrame();

  private:
    ImGuiIO* m_io = nullptr;
    ImGuiWindows_Map m_windows{};
};
} // namespace kogayonon_gui
