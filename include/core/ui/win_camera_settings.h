#pragma once

#include "core/singleton/singleton.h"
#include "imgui_window.h"

namespace kogayonon
{
  class CameraSettingsWindow : public ImguiWindow
  {
  public:
    explicit CameraSettingsWindow(std::string name, bool can_move = false) :ImguiWindow(std::move(name))
    {
      m_props->can_move = can_move;
      m_props->m_flags |= ImGuiWindowFlags_NoMove;
    }

  private:
    bool initCameraWindow();
    void draw() override;

    // specific functions for this particular window
    void drawCoordinates(float x, float y, float z);
  };
}
