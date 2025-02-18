#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif  // !GLFW_INCLUDE_NONE

#include "imgui_window.h"
#include "core/singleton/singleton.h"

#include <GLFW/glfw3.h>
#include <vector>

namespace kogayonon
{
  using Windows = std::vector<ImguiWindow*>;
  class ImGuiInterface : public Singleton<ImGuiInterface> {
  public:
    ImGuiInterface() = default;
    ~ImGuiInterface();

    ImGuiInterface(GLFWwindow* window);
    bool initImgui(GLFWwindow* window);
    bool initWindows();
    void draw();

  private:
    Windows& getWindows();

  private:
    Windows m_windows{};
  };
}
