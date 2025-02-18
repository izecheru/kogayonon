#pragma once

#include "core/singleton/singleton.h"

#include "imgui_window.h"

namespace kogayonon
{
  class CameraSettingsWindow : public ImguiWindow {
  public:
    ~CameraSettingsWindow() = default;
    CameraSettingsWindow(std::string name) :ImguiWindow(std::move(name)) {}

  private:
    bool initCameraWindow();
    void draw() override;
  };
}
