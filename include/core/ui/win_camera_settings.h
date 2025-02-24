#pragma once

#include "core/singleton/singleton.h"
#include "core/renderer/camera.h"
#include "imgui_window.h"

namespace kogayonon
{
  class CameraSettingsWindow : public ImguiWindow
  {
  public:
    explicit CameraSettingsWindow(std::string name, bool can_move = false) :ImguiWindow(std::move(name))
    {
      m_props->can_move = can_move;
      m_props->m_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize;
    }

  private:
    bool initCameraWindow();
    void draw() override;

    // specific functions for this particular window
    void drawProperties(CameraProps& t_props);
  };
}
