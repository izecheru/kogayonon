#pragma once

#include "renderer/camera.h"
#include "singleton/singleton.h"
#include "imgui_window.h"

namespace kogayonon
{
  class CameraSettingsWindow : public ImGuiWindow
  {
  public:
    explicit CameraSettingsWindow(const std::string& name, bool can_move = true) : ImGuiWindow(name)
    {
      m_props->can_move = can_move;
      m_props->m_flags |= ImGuiWindowFlags_AlwaysAutoResize;
    }

    void draw() override;

  private:
    void drawProperties(CameraProps& t_props);
  };
} // namespace kogayonon
