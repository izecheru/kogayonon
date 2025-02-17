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
  class ImGuiInterface : public Singleton<ImGuiInterface> {
  public:
    ImGuiInterface() = default;
    ~ImGuiInterface() = default;

    ImGuiInterface(GLFWwindow* window);

    bool initImgui(GLFWwindow* window);
    void draw();

    bool createWindow(std::string window_name, double x_pos, double y_pos);

    std::vector<ImguiWindow>& getWindows();

  private:
    std::vector<ImguiWindow> m_windows{};
  };
}
