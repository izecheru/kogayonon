#pragma once
#include <SDL.h>

#include <vector>

#include "imgui_window.h"
#include "layer/imgui_layer.h"
#include "singleton/singleton.h"

namespace kogayonon
{
class ImGuiManager
{
public:
  explicit ImGuiManager(SDL_Window* window, SDL_GLContext context);
  ~ImGuiManager();

  bool initImgui(SDL_Window* window, SDL_GLContext context);
  void push_window(std::shared_ptr<ImGuiWindow> window);
  std::vector<std::shared_ptr<ImGuiWindow>>& ImGuiManager::getWindows();
  void draw();
  void mainMenu();
  void setupDockSpace(ImGuiViewport* viewport);

  void beginImGuiFrame();
  void endImGuiFrame();

private:
  ImGuiIO* m_io = nullptr;
  std::vector<std::shared_ptr<ImGuiWindow>> m_windows{};
};
} // namespace kogayonon
