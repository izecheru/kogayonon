#pragma once
#include <SDL2/SDL.h>
#include "imgui_window.hpp"

namespace kogayonon_gui
{

struct Popups
{
  bool configPopup{ false };
};

class ImGuiManager
{
  using ImGuiWindows_Map = std::unordered_map<std::string, std::unique_ptr<ImGuiWindow>>;

public:
  explicit ImGuiManager( SDL_Window* window, SDL_GLContext context );
  ~ImGuiManager();

  bool initImgui( SDL_Window* window, SDL_GLContext context );
  auto getWindows() -> ImGuiWindows_Map&;

  void pushWindow( std::string name, std::unique_ptr<ImGuiWindow> window );
  void setupDockSpace( ImGuiViewport* viewport );
  void payloadTooltip();
  void mainMenu();
  void draw();

  void configPopup();

  void begin();
  void end();

private:
  ImGuiIO* m_io = nullptr;
  ImGuiWindows_Map m_windows{};
  Popups m_popups;
};
} // namespace kogayonon_gui