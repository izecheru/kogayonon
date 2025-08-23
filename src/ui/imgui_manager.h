#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include <glfw3.h>

#include <vector>

#include "layer/imgui_layer.h"
#include "singleton/singleton.h"
#include "imgui_window.h"

namespace kogayonon
{
  class ImGuiManager
  {
  public:
    explicit ImGuiManager(GLFWwindow* window);
    ~ImGuiManager();

    bool initImgui(GLFWwindow* window);
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
