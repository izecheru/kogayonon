#pragma once
#include "imgui_window.h"

namespace kogayonon
{
class SceneViewportWindow : public ImGuiWindow
{
public:
  SceneViewportWindow(std::string&& name) : ImGuiWindow(std ::move(name)) {}

  ~SceneViewportWindow() {}

  void draw() override;
};
} // namespace kogayonon