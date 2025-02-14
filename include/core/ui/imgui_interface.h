#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif  // !GLFW_INCLUDE_NONE

#include "imgui_window.h"
#include <GLFW/glfw3.h>
#include <vector>

namespace kogayonon
{
  class MyImguiInterface
  {
  public:
    MyImguiInterface() = default;
    ~MyImguiInterface() = default;

    MyImguiInterface(GLFWwindow* window);

    bool initImgui(GLFWwindow* window);
    void draw();

    bool createWindow(std::string window_name, double x_pos, double y_pos, bool docked = false, bool visible = true);

  private:
    std::vector<ImguiWindow> m_imgui_windows;
  };
}

