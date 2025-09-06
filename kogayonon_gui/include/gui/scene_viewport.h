#pragma once
#include "imgui_window.h"

namespace kogayonon_rendering {
class FrameBuffer;
}

namespace kogayonon_gui {
class SceneViewportWindow : public ImGuiWindow
{
  public:
    explicit SceneViewportWindow(std::string name);
    ~SceneViewportWindow() = default;
    void draw() override;

  private:
    std::unique_ptr<kogayonon_rendering::FrameBuffer> m_fbo;
};
} // namespace kogayonon_gui