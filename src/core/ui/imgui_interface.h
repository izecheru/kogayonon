#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include <glfw3.h>

#include <vector>

#include "core/singleton/singleton.h"
#include "imgui_window.h"

namespace kogayonon
{
  class ImGuiInterface
  {
  public:
    ImGuiInterface() = default;
    ~ImGuiInterface();

    explicit ImGuiInterface(GLFWwindow* window);
    bool initImgui(GLFWwindow* window);
    bool initWindows();
    void draw();

  private:
    std::vector<std::shared_ptr<ImguiWindow>>& getWindows();

  private:
    std::vector<std::shared_ptr<ImguiWindow>> m_windows{};
  };
} // namespace kogayonon
