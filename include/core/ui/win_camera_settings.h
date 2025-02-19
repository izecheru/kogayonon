#pragma once

#include "core/singleton/singleton.h"
#include "imgui_window.h"

namespace kogayonon
{
  class CameraSettingsWindow : public ImguiWindow {
  public:
    explicit CameraSettingsWindow(std::string name) :ImguiWindow(std::move(name)) {}

  private:
    bool initCameraWindow();
    void draw() override;

    // specific functions for this particular window
    void drawCoordinates(float x, float y, float z);
  };
}
