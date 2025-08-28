#pragma once

#include "imgui_window.h"
#include "renderer/camera.h"
#include "singleton/singleton.h"

namespace kogayonon
{
class CameraSettingsWindow : public ImGuiWindow
{
public:
  explicit CameraSettingsWindow(std::string&& name, bool can_move = true) : ImGuiWindow(std::move(name))
  {
    m_props->can_move = can_move;
    m_props->flags |= ImGuiWindowFlags_AlwaysAutoResize;
  }

  void draw() override;

private:
  void drawProperties(CameraProps& t_props);
};
} // namespace kogayonon
