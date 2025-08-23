#pragma once
#include "imgui_window.h"

namespace kogayonon
{
  class SceneViewportWindow : public ImGuiWindow
  {
  public:
    SceneViewportWindow(const std::string& name) : ImGuiWindow(name) {}

    ~SceneViewportWindow() {}

    void draw() override;
  };
} // namespace kogayonon