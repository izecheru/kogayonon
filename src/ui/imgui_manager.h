#pragma once
#include <SDL.h>

#include <vector>

#include "imgui_window.h"
#include "layer/imgui_layer.h"
#include "renderer/framebuffer.h"
#include "singleton/singleton.h"

namespace kogayonon
{
class ImGuiManager
{
public:
  explicit ImGuiManager(SDL_Window* window, SDL_GLContext context, std::shared_ptr<FrameBuffer> fbo);
  ~ImGuiManager();

  bool initImgui(SDL_Window* window, SDL_GLContext context);
  void push_window(std::string&& name, std::shared_ptr<ImGuiWindow> window);
  std::unordered_map<std::string, std::shared_ptr<ImGuiWindow>>& getWindows();
  void draw();
  void mainMenu();
  void setupDockSpace(ImGuiViewport* viewport);

  void beginImGuiFrame();
  void endImGuiFrame();

private:
  ImGuiIO* m_io = nullptr;
  std::unordered_map<std::string, std::shared_ptr<ImGuiWindow>> m_windows{};
};
} // namespace kogayonon
