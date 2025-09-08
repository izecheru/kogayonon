#pragma once
#include "imgui_window.h"

namespace kogayonon_rendering
{
class FrameBuffer;
}

namespace kogayonon_core
{
class Scene;
}

namespace kogayonon_gui
{
class SceneViewportWindow : public ImGuiWindow
{
  public:
    explicit SceneViewportWindow(std::string name);
    ~SceneViewportWindow() = default;

    void draw() override;
    kogayonon_rendering::FrameBuffer* getFrameBuffer();

  private:
    std::unique_ptr<kogayonon_rendering::FrameBuffer> m_pFrameBuffer;
    std::weak_ptr<kogayonon_core::Scene> m_pCurrentScene;
};
} // namespace kogayonon_gui